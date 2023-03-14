#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "qemu/timer.h"
#include "trace.h"
#include <signal.h>
#include <ucontext.h>
#include "latxs-fastcs-cfg.h"
#include "latx-np-sys.h"

/*
 * option_fastcs
 * = 0 : disabled
 * = 1 : enable TB ctx only
 * = 2 : use jmp-glue method (TB ctx enabled)
 * = 3 : use load-exception method (TB ctx enabled)
 * = 4 : use no-link mode (TB ctx enabled)
 */

int latxs_fastcs_enabled(void)
{
    return option_fastcs >= FASTCS_ENABLED; /* >= 2 */
}

int latxs_fastcs_enable_tbctx(void)
{
    return option_fastcs >= FASTCS_TBCTX; /* >= 1 */
}

int latxs_fastcs_is_no_link(void)
{
    return option_fastcs == FASTCS_NO_LINK;
}

int latxs_fastcs_is_jmp_glue(void)
{
    return option_fastcs == FASTCS_JMP_GLUE ||
           option_fastcs == FASTCS_DIRECT_JMP_GLUE;
}

int latxs_fastcs_is_jmp_glue_direct(void)
{
    return option_fastcs == FASTCS_DIRECT_JMP_GLUE;
}

int latxs_fastcs_is_ld_excp(void)
{
    return option_fastcs == FASTCS_LD_EXCP;
}

int latxs_fastcs_is_ld_branch(void)
{
    return option_fastcs == FASTCS_LD_BRANCH ||
           option_fastcs == FASTCS_LD_BRANCH_INLINE;
}

int latxs_fastcs_is_ld_branch_inline(void)
{
    return option_fastcs == FASTCS_LD_BRANCH_INLINE;
}

void latxs_fastcs_env_init(CPUX86State *env)
{
    env->fastcs_ctx = FASTCS_CTX_NON;

    if (env->fastcs_ptr != NULL) {
        return;
    }

    assert(lsenv->cpu_state == env);

    /*
     *    00 01 10 11
     * 00 -- -F S- SF
     * 01 -- -- S- S-
     * 10 -- -F -- -F
     * 11 -- -- -- --
     */
    lsenv->fastcs_data.indir_table[0][0] = NULL;
    lsenv->fastcs_data.indir_table[0][1] = (void *)latxs_sc_fcs_check_load_F;
    lsenv->fastcs_data.indir_table[0][2] = (void *)latxs_sc_fcs_check_load_S;
    lsenv->fastcs_data.indir_table[0][3] = (void *)latxs_sc_fcs_check_load_FS;
    lsenv->fastcs_data.indir_table[1][0] = NULL;
    lsenv->fastcs_data.indir_table[1][1] = NULL;
    lsenv->fastcs_data.indir_table[1][2] = (void *)latxs_sc_fcs_check_load_S;
    lsenv->fastcs_data.indir_table[1][3] = (void *)latxs_sc_fcs_check_load_S;
    lsenv->fastcs_data.indir_table[2][0] = NULL;
    lsenv->fastcs_data.indir_table[2][1] = (void *)latxs_sc_fcs_check_load_F;
    lsenv->fastcs_data.indir_table[2][2] = NULL;
    lsenv->fastcs_data.indir_table[2][3] = (void *)latxs_sc_fcs_check_load_F;
    lsenv->fastcs_data.indir_table[3][0] = NULL;
    lsenv->fastcs_data.indir_table[3][1] = NULL;
    lsenv->fastcs_data.indir_table[3][2] = NULL;
    lsenv->fastcs_data.indir_table[3][3] = NULL;

    env->fastcs_ptr = &lsenv->fastcs_data;
    lsenv->fastcs_data.env = env;
}

#ifdef LATXS_NP_ENABLE
void latxs_native_printer_cs(lsenv_np_data_t *npd,
        int type, int r1, int r2, int r3, int r4, int r5)
{
    CPUX86State *env = npd->env;
    lsenv_fastcs_t *fcs = env->fastcs_ptr;

    uint64_t cs_type = fcs->cs_type;
    ssize_t r = 0;
    switch(cs_type) {
    case 1:
        r = write(2, "1", 1);
        break;
    case 2:
        r = write(2, "2", 1);
        break;
    case 3:
        r = write(2, "3", 1);
        break;
    case 4:
        r = write(2, "4", 1);
        break;
    case 5:
        r = write(2, "5", 1);
        break;
    case 6:
        r = write(2, "6", 1);
        break;
    default:
        break;
    }
    lsassertm(r == 1 || r == 2, "write %d", (int)r);
    (void)r;
}
#endif

void latxs_reset_tb_fastcs_ctx(TranslationBlock *tb)
{
    tb->fastcs_ctx = FASTCS_CTX_NON;
}

void latxs_disasm_tb_fastcs_ctx(TranslationBlock *tb, IR1_INST *pir1)
{
    if (!latxs_fastcs_enable_tbctx()) {
        return;
    }

    int i = 0;
    int grp_nr = latxs_ir1_grp_nr(pir1);
    uint8_t *grps = latxs_ir1_get_grps(pir1);

    switch (ir1_opcode(pir1)) {
    case X86_INS_FXSAVE:
    case X86_INS_FXRSTOR:
    case X86_INS_XSAVE:
    case X86_INS_XRSTOR:
    case X86_INS_XSAVEOPT:
        tb->fastcs_ctx |= FASTCS_CTX_FPU;
        tb->fastcs_ctx |= FASTCS_CTX_SIMD;
        break;
    default:
        break;
    }

    for (i = 0; i < grp_nr; ++i) {
        switch (grps[i]) {
            case X86_GRP_FPU:
            case X86_GRP_MMX:
            case X86_GRP_3DNOW: /* MMX */
                tb->fastcs_ctx |= FASTCS_CTX_FPU;
#if defined(FASTCS_FPU_AND_SIMD)
                tb->fastcs_ctx |= FASTCS_CTX_SIMD;
#endif
                break;
            case X86_GRP_AES:
            case X86_GRP_ADX:
            case X86_GRP_AVX:
            case X86_GRP_AVX2:
            case X86_GRP_AVX512:
            case X86_GRP_F16C:
            case X86_GRP_FMA:
            case X86_GRP_FMA4:
            case X86_GRP_SHA:
            case X86_GRP_SSE1:
            case X86_GRP_SSE2:
            case X86_GRP_SSE3:
            case X86_GRP_SSE41:
            case X86_GRP_SSE42:
            case X86_GRP_SSE4A:
            case X86_GRP_PCLMUL:
            case X86_GRP_XOP:
            case X86_GRP_CDI: /* AVX512CD  */
            case X86_GRP_ERI: /* AVX512ER  */
            case X86_GRP_DQI: /* AVX512DQ  */
            case X86_GRP_BWI: /* AVX512BW  */
            case X86_GRP_PFI: /* AVX512PF  */
            case X86_GRP_VLX: /* AVX512VL? */
#if defined(FASTCS_FPU_AND_SIMD)
                tb->fastcs_ctx |= FASTCS_CTX_FPU;
#endif
                tb->fastcs_ctx |= FASTCS_CTX_SIMD;
                break;
            case X86_GRP_SSSE3:
                tb->fastcs_ctx |= FASTCS_CTX_FPU;
                tb->fastcs_ctx |= FASTCS_CTX_SIMD;
                break;
            default:
                break;
        }
    }
}

static void __latxs_fastcs_load_registers(
        uint32_t gpr_to_load,
        uint8_t fpr_to_load, int load_top, int is_scs,
        uint32_t xmm_to_load, uint8_t vreg_to_load,
        IR2_OPND *ctx, IR2_OPND *tmp)
{
    /* LOAD REGISTERS: vreg */
    latxs_tr_load_vreg_from_env(vreg_to_load);

    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND label_no_fpu  = latxs_ir2_opnd_new_label();
    IR2_OPND label_no_simd = latxs_ir2_opnd_new_label();

    /* load ENV fastcs */
    latxs_append_ir2_opnd2i(LISA_LD_BU, ctx, env,
            offsetof(CPUX86State, fastcs_ctx));
    
    latxs_append_ir2_opnd2i(LISA_ANDI, tmp, ctx, FASTCS_CTX_FPU);
    latxs_append_ir2_opnd3(LISA_BEQ, tmp, zero, &label_no_fpu);
    /* LOAD REGISTERS: FPU */
    if (is_scs) {
        lsassert(load_top == 1);
        latxs_tr_load_fprs_from_env(fpr_to_load, 0);
        if (option_lsfpu && !option_soft_fpu) {
            latxs_tr_fpu_enable_top_mode();
            latxs_tr_load_lstop_from_env(tmp);
        }
    } else {
        latxs_tr_load_fprs_from_env(fpr_to_load, load_top);
    }
    latxs_append_ir2_opnd1(LISA_LABEL, &label_no_fpu);

    latxs_append_ir2_opnd2i(LISA_ANDI, tmp, ctx, FASTCS_CTX_SIMD);
    latxs_append_ir2_opnd3(LISA_BEQ, tmp, zero, &label_no_simd);
    /* LOAD REGISTERS: SIMD */
    latxs_tr_load_xmms_from_env(xmm_to_load);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_no_simd);

    /* LOAD REGISTERS: GPR */
    latxs_tr_load_gprs_from_env(gpr_to_load);
}

static void __latxs_fastcs_save_registers(
        uint32_t gpr_to_save,
        uint8_t fpr_to_save, int save_top, int is_scs,
        uint32_t xmm_to_save, uint8_t vreg_to_save,
        IR2_OPND *ctx, IR2_OPND *tmp)
{
    /* SAVE REGISTERS: GPR */
    latxs_tr_save_gprs_to_env(gpr_to_save);

    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND label_no_fpu  = latxs_ir2_opnd_new_label();
    IR2_OPND label_no_simd = latxs_ir2_opnd_new_label();

    /* load ENV fastcs */
    latxs_append_ir2_opnd2i(LISA_LD_BU, ctx, env,
            offsetof(CPUX86State, fastcs_ctx));

    latxs_append_ir2_opnd2i(LISA_ANDI, tmp, ctx, FASTCS_CTX_FPU);
    latxs_append_ir2_opnd3(LISA_BEQ, tmp, zero, &label_no_fpu);
    /* SAVE REGISTERS: FPU */
    if (is_scs) {
        lsassert(save_top == 1);
        if (option_lsfpu && !option_soft_fpu) {
            latxs_tr_save_lstop_to_env(tmp);
            latxs_tr_fpu_disable_top_mode();
        }
        latxs_tr_save_fprs_to_env(fpr_to_save, 0);
    } else {
        latxs_tr_save_fprs_to_env(fpr_to_save, save_top);
    }
    latxs_append_ir2_opnd1(LISA_LABEL, &label_no_fpu);

    latxs_append_ir2_opnd2i(LISA_ANDI, tmp, ctx, FASTCS_CTX_SIMD);
    latxs_append_ir2_opnd3(LISA_BEQ, tmp, zero, &label_no_simd);
    /* SAVE REGISTERS: SIMD */
    latxs_tr_save_xmms_to_env(xmm_to_save);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_no_simd);

    /* SAVE REGISTERS: vreg */
    latxs_tr_save_vreg_to_env(vreg_to_save);
}

void latxs_fastcs_load_registers(
        uint32_t gpr, uint8_t fpr, int load_top,
        uint32_t xmm, uint8_t vreg)
{
    lsassert(latxs_fastcs_enabled());

    IR2_OPND ctx = latxs_ra_alloc_itemp();
    IR2_OPND tmp = latxs_ra_alloc_itemp();

    __latxs_fastcs_load_registers(gpr, fpr, load_top, 0,
                                  xmm, vreg, &ctx, &tmp);

    latxs_ra_free_temp(&ctx);
    latxs_ra_free_temp(&tmp);
}

void latxs_fastcs_save_registers(
        uint32_t gpr, uint8_t fpr, int save_top,
        uint32_t xmm, uint8_t vreg)
{
    lsassert(latxs_fastcs_enabled());

    IR2_OPND ctx = latxs_ra_alloc_itemp();
    IR2_OPND tmp = latxs_ra_alloc_itemp();

    __latxs_fastcs_save_registers(gpr, fpr, save_top, 0,
                                  xmm, vreg, &ctx, &tmp);

    latxs_ra_free_temp(&ctx);
    latxs_ra_free_temp(&tmp);
}

void latxs_fastcs_static_load_registers(
        uint32_t gpr, uint8_t fpr,
        uint32_t xmm, uint8_t vreg)
{
    lsassert(latxs_fastcs_enabled());

    IR2_OPND *ctx = &latxs_stmp1_ir2_opnd;
    IR2_OPND *tmp = &latxs_stmp2_ir2_opnd;

    __latxs_fastcs_load_registers(gpr, fpr, 1, 1,
                                  xmm, vreg, ctx, tmp);
}

void latxs_fastcs_static_save_registers(
        uint32_t gpr, uint8_t fpr,
        uint32_t xmm, uint8_t vreg)
{
    lsassert(latxs_fastcs_enabled());

    IR2_OPND *ctx = &latxs_stmp1_ir2_opnd;
    IR2_OPND *tmp = &latxs_stmp2_ir2_opnd;

    __latxs_fastcs_save_registers(gpr, fpr, 1, 1,
                                  xmm, vreg, ctx, tmp);
}

/* FastCS TB link */
int latxs_fastcs_set_jmp_target(void *_tb,
        int n, void *_nextb)
{
    TranslationBlock *tb    = _tb;
    TranslationBlock *nextb = _nextb;

    if (latxs_fastcs_is_no_link()) {
        if (tb->fastcs_ctx != nextb->fastcs_ctx) {
            /* do not link */
            return 1;
        } else {
            /* Fall through to normal TB Link */
            return 0;
        }
    }

    if (latxs_fastcs_is_ld_excp() ||
        latxs_fastcs_is_ld_branch()) {
        /* Fall through to normal TB Link */
        return 0;
    }

    lsassert(latxs_fastcs_is_jmp_glue());

    uint8_t ctx1 =    (tb->fastcs_ctx) & 0x3;
    uint8_t ctx2 = (nextb->fastcs_ctx) & 0x3;

    lsassert(ctx1 ==    tb->fastcs_ctx);
    lsassert(ctx2 == nextb->fastcs_ctx);
    
    /*
     *      00  01  10  11
     *    +----------------+ 
     * 00 | --  -F  S-  SF |
     * 01 | --  --  S-  S- |
     * 10 | --  -F  --  -F |
     * 11 | --  --  --  -- |
     *    +----------------+ 
     */

    static uint8_t fastcs_tb_link[4][4] = {
        {0, 1, 2, 3},
        {0, 0, 2, 2},
        {0, 1, 0, 1},
        {0, 0, 0, 0}
    };

    uint8_t link_mode = fastcs_tb_link[ctx1][ctx2];

    if (link_mode == 0) {
        /* Fall through to normal TB Link */
        return 0;
    }

    lsassert(tb->fastcs_jmp_glue_checker);
    tb->next_tb[n] = nextb;

    ADDR fastcs_jmp_glue = 0;
    switch ((link_mode << 4) | n) {
    case 0x10:
        fastcs_jmp_glue = latxs_sc_fcs_F_0;
        break;
    case 0x11:
        fastcs_jmp_glue = latxs_sc_fcs_F_1;
        break;
    case 0x20:
        fastcs_jmp_glue = latxs_sc_fcs_S_0;
        break;
    case 0x21:
        fastcs_jmp_glue = latxs_sc_fcs_S_1;
        break;
    case 0x30:
        fastcs_jmp_glue = latxs_sc_fcs_FS_0;
        break;
    case 0x31:
        fastcs_jmp_glue = latxs_sc_fcs_FS_1;
        break;
    default:
        break;
    }

    lsassertm(fastcs_jmp_glue, "link mode %d n %d JG %p\n",
            link_mode, n, (void *)fastcs_jmp_glue);

    int ret = tb_set_jmp_target_fastcs(tb, n, nextb,
            fastcs_jmp_glue,
            latxs_fastcs_is_jmp_glue_direct());
    lsassertm(ret, "tb set jmp target fastcs jmp glue\n");

    return 2;
}

void tb_reset_fastcs_jmp_glue(TranslationBlock *tb, int n)
{
    if (!latxs_fastcs_is_jmp_glue_direct()) return;

    uintptr_t offset = tb->jmp_target_arg[n];
    uintptr_t tc_ptr = (uintptr_t)tb->tc.ptr;
    uintptr_t fastcs_jmp_rx = tc_ptr + offset - 4;
    uintptr_t fastcs_jmp_rw = fastcs_jmp_rx - tcg_splitwx_diff;
    tb_target_reset_fastcs_jmp_glue(tc_ptr,
            fastcs_jmp_rx, fastcs_jmp_rw);
}

int gen_latxs_sc_fcs_jmp_glue_return(void *code_ptr, int ctx, int n)
{
    lsassert(latxs_fastcs_is_jmp_glue_direct());

    lsassertm(option_lsfpu,
            "TODO: FastCS does not support non-LSFPU yet.\n");
    lsassertm(!option_soft_fpu,
            "TODO: FastCS does not support SoftFPU yet.\n");

    lsassert(ctx == (ctx & 0x3));

    latxs_tr_init(NULL);

    IR2_OPND *env   = &latxs_env_ir2_opnd;
    IR2_OPND *zero  = &latxs_zero_ir2_opnd;
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd;

    /* stmp1 = ENV.fastcs_ctx */
    latxs_append_ir2_opnd2i(LISA_LD_BU, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));

    if (ctx & 0x1) {
        /* check and Load FPU */
        IR2_OPND no_fpu = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_ANDI, stmp2, stmp1, 0x1);
        latxs_append_ir2_opnd3(LISA_BNE, stmp2, zero, &no_fpu);

        /* load FPU using LSFPU */
        latxs_tr_load_fprs_from_env(0xff, 0);
        latxs_tr_load_lstop_from_env(stmp2);
        latxs_tr_fpu_enable_top_mode();

        latxs_append_ir2_opnd1(LISA_LABEL, &no_fpu);
    }

    if (ctx & 0x2) {
        /* check and Load SIMD */
        IR2_OPND no_simd = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_ANDI, stmp2, stmp1, 0x2);
        latxs_append_ir2_opnd3(LISA_BNE, stmp2, zero, &no_simd);

        /* load simd */
        latxs_tr_load_xmms_from_env(0xffffffff);

        latxs_append_ir2_opnd1(LISA_LABEL, &no_simd);
    }

    /* update context */
    latxs_append_ir2_opnd2i(LISA_LD_BU, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));
    latxs_append_ir2_opnd2i(LISA_ORI, stmp1, stmp1, ctx);
    latxs_append_ir2_opnd2i(LISA_ST_B, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));

    /* return */
    latxs_append_ir2_opnd0_(lisa_return);

    int code_nr  = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}



int gen_latxs_sc_fcs_jmp_glue(void *code_ptr, int ctx, int n)
{
    lsassert(latxs_fastcs_is_jmp_glue());

    lsassertm(option_lsfpu,
            "TODO: FastCS does not support non-LSFPU yet.\n");
    lsassertm(!option_soft_fpu,
            "TODO: FastCS does not support SoftFPU yet.\n");

    lsassert(ctx == (ctx & 0x3));

    latxs_tr_init(NULL);

    /* TB must be set before fastcs jmp glue */
    IR2_OPND tb = latxs_ra_alloc_dbt_arg1();

    IR2_OPND *env   = &latxs_env_ir2_opnd;
    IR2_OPND *zero  = &latxs_zero_ir2_opnd;
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd;

    /* stmp1 = ENV.fastcs_ctx */
    latxs_append_ir2_opnd2i(LISA_LD_BU, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));

    if (ctx & 0x1) {
        /* check and Load FPU */
        IR2_OPND no_fpu = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_ANDI, stmp2, stmp1, 0x1);
        latxs_append_ir2_opnd3(LISA_BNE, stmp2, zero, &no_fpu);

        /* load FPU using LSFPU */
        latxs_tr_load_fprs_from_env(0xff, 0);
        latxs_tr_load_lstop_from_env(stmp2);
        latxs_tr_fpu_enable_top_mode();

        latxs_append_ir2_opnd1(LISA_LABEL, &no_fpu);
    }

    if (ctx & 0x2) {
        /* check and Load SIMD */
        IR2_OPND no_simd = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_ANDI, stmp2, stmp1, 0x2);
        latxs_append_ir2_opnd3(LISA_BNE, stmp2, zero, &no_simd);

        /* load simd */
        latxs_tr_load_xmms_from_env(0xffffffff);

        latxs_append_ir2_opnd1(LISA_LABEL, &no_simd);
    }

    /* update context */
    latxs_append_ir2_opnd2i(LISA_LD_BU, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));
    latxs_append_ir2_opnd2i(LISA_ORI, stmp1, stmp1, ctx);
    latxs_append_ir2_opnd2i(LISA_ST_B, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));

    /* jmp to execute next TB */
    latxs_append_ir2_opnd2i(LISA_LD_D, stmp1, &tb,
            offsetof(TranslationBlock, next_tb) +
            n * sizeof(void *));
    latxs_append_ir2_opnd2i(LISA_LD_D, stmp2, stmp1,
            offsetof(TranslationBlock, tc) +
            offsetof(struct tb_tc, ptr));
    latxs_append_ir2_opnd2i(LISA_JIRL, zero, stmp2, 0);

    int code_nr  = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}

int gen_latxs_sc_fcs_check_load(void *code_ptr, int ctx)
{
    lsassert(latxs_fastcs_is_jmp_glue() ||
             latxs_fastcs_is_ld_branch());

    lsassertm(option_lsfpu,
            "TODO: FastCS does not support non-LSFPU yet.\n");
    lsassertm(!option_soft_fpu,
            "TODO: FastCS does not support SoftFPU yet.\n");

    lsassert(ctx == (ctx & 0x3));

    latxs_tr_init(NULL);

    IR2_OPND *env   = &latxs_env_ir2_opnd;
    IR2_OPND *zero  = &latxs_zero_ir2_opnd;
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd;

    /* stmp1 = ENV.fastcs_ctx */
    latxs_append_ir2_opnd2i(LISA_LD_BU, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));

    if (ctx & 0x1) {
        /* check and Load FPU */
        IR2_OPND no_fpu = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_ANDI, stmp2, stmp1, 0x1);
        latxs_append_ir2_opnd3(LISA_BNE, stmp2, zero, &no_fpu);

        /* load FPU using LSFPU */
        latxs_tr_load_fprs_from_env(0xff, 0);
        latxs_tr_load_lstop_from_env(stmp2);
        latxs_tr_fpu_enable_top_mode();

        latxs_append_ir2_opnd1(LISA_LABEL, &no_fpu);
    }

    if (ctx & 0x2) {
        /* check and Load SIMD */
        IR2_OPND no_simd = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_ANDI, stmp2, stmp1, 0x2);
        latxs_append_ir2_opnd3(LISA_BNE, stmp2, zero, &no_simd);

        /* load simd */
        latxs_tr_load_xmms_from_env(0xffffffff);

        latxs_append_ir2_opnd1(LISA_LABEL, &no_simd);
    }

    /* update context */
    latxs_append_ir2_opnd2i(LISA_LD_BU, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));
    latxs_append_ir2_opnd2i(LISA_ORI, stmp1, stmp1, ctx);
    latxs_append_ir2_opnd2i(LISA_ST_B, stmp1, env,
            offsetof(CPUX86State, fastcs_ctx));

    /* return */
    latxs_append_ir2_opnd0_(lisa_return);

    int code_nr  = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}

int gen_latxs_sc_fcs_load(void *code_ptr, int ctx)
{
    lsassert(latxs_fastcs_is_jmp_glue() ||
             latxs_fastcs_is_ld_branch());

    lsassertm(option_lsfpu,
            "TODO: FastCS does not support non-LSFPU yet.\n");
    lsassertm(!option_soft_fpu,
            "TODO: FastCS does not support SoftFPU yet.\n");

    lsassert(ctx == (ctx & 0x3));

    latxs_tr_init(NULL);

    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;

    if (ctx & 0x1) {
        /* load FPU using LSFPU */
        latxs_tr_load_fprs_from_env(0xff, 0);
        latxs_tr_fpu_enable_top_mode();
        latxs_tr_load_lstop_from_env(stmp1);
    }

    if (ctx & 0x2) {
        latxs_tr_load_xmms_from_env(0xffffffff);
    }

    /* return */
    latxs_append_ir2_opnd0_(lisa_return);

    int code_nr  = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}

void latxs_fastcs_tb_start(TranslationBlock *tb)
{
    if (!latxs_fastcs_is_ld_branch()) {
        return;
    }

    ADDR code_buf = (ADDR)tb->tc.ptr;
    int offset = lsenv->tr_data->ir2_asm_nr << 2;
    int64_t ins_offset = 0;

    if (tb->fastcs_ctx == FASTCS_CTX_NON) {
        return;
    }

    if (latxs_fastcs_is_ld_branch_inline()) {
        IR2_OPND *env   = &latxs_env_ir2_opnd;
        IR2_OPND *zero  = &latxs_zero_ir2_opnd;
        IR2_OPND tmp1 = latxs_ra_alloc_itemp();
        IR2_OPND tmp2 = latxs_ra_alloc_itemp();

        IR2_OPND label = latxs_ir2_opnd_new_label();
        IR2_OPND label1, label2;

        /* tmp1 = ENV.fastcs_ctx */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp1, env,
                offsetof(CPUX86State, fastcs_ctx));

        switch (tb->fastcs_ctx) {
        case 0x1:
            /* check FPU : branch if 1 */
            latxs_append_ir2_opnd2i(LISA_ANDI, &tmp2, &tmp1, 0x1);
            latxs_append_ir2_opnd3(LISA_BNE, &tmp2, zero, &label);
            /* load FPU */
            offset = lsenv->tr_data->ir2_asm_nr << 2;
            ins_offset = (latxs_sc_fcs_load_F - code_buf - offset) >> 2;
            latxs_append_ir2_jmp_far(ins_offset, 1);
            /* update context */
            latxs_append_ir2_opnd2i(LISA_ORI, &tmp1, &tmp1, 0x1);
            latxs_append_ir2_opnd2i(LISA_ST_B, &tmp1, env,
                    offsetof(CPUX86State, fastcs_ctx));
            break;
        case 0x2:
            /* check SIMD : branch if 2 */
            latxs_append_ir2_opnd2i(LISA_ANDI, &tmp2, &tmp1, 0x2);
            latxs_append_ir2_opnd3(LISA_BNE, &tmp2, zero, &label);
            /* load SIMD */
            offset = lsenv->tr_data->ir2_asm_nr << 2;
            ins_offset = (latxs_sc_fcs_load_S - code_buf - offset) >> 2;
            latxs_append_ir2_jmp_far(ins_offset, 1);
            /* update context */
            latxs_append_ir2_opnd2i(LISA_ORI, &tmp1, &tmp1, 0x2);
            latxs_append_ir2_opnd2i(LISA_ST_B, &tmp1, env,
                    offsetof(CPUX86State, fastcs_ctx));
            break;
        case 0x3:
            /* check FPU/SIMD : branch if 3  */
            latxs_append_ir2_opnd2i(LISA_ORI, &tmp2, zero, 0x3);
            latxs_append_ir2_opnd3(LISA_BEQ, &tmp1, &tmp2, &label);
            /* check FPU : branch if not 1 */
            label1 = latxs_ir2_opnd_new_label();
            latxs_append_ir2_opnd2i(LISA_ANDI, &tmp2, &tmp1, 0x1);
            latxs_append_ir2_opnd3(LISA_BNE, &tmp2, zero, &label1);
            offset = lsenv->tr_data->ir2_asm_nr << 2;
            ins_offset = (latxs_sc_fcs_load_F - code_buf - offset) >> 2;
            latxs_append_ir2_jmp_far(ins_offset, 1);
            latxs_append_ir2_opnd1(LISA_LABEL, &label1);
            /* check SIMD : branch if not 1 */
            label2 = latxs_ir2_opnd_new_label();
            latxs_append_ir2_opnd2i(LISA_ANDI, &tmp2, &tmp1, 0x2);
            latxs_append_ir2_opnd3(LISA_BNE, &tmp2, zero, &label2);
            offset = lsenv->tr_data->ir2_asm_nr << 2;
            ins_offset = (latxs_sc_fcs_load_S - code_buf - offset) >> 2;
            latxs_append_ir2_opnd1(LISA_LABEL, &label2);
            latxs_append_ir2_jmp_far(ins_offset, 1);
            /* update context */
            latxs_append_ir2_opnd2i(LISA_ORI, &tmp1, zero, 0x3);
            latxs_append_ir2_opnd2i(LISA_ST_B, &tmp1, env,
                    offsetof(CPUX86State, fastcs_ctx));
            break;
        default:
            return;
        }

        latxs_append_ir2_opnd1(LISA_LABEL, &label);

        latxs_ra_free_temp(&tmp1);
        latxs_ra_free_temp(&tmp2);
    } else {
        switch (tb->fastcs_ctx) {
        case 0x1:
            ins_offset = (latxs_sc_fcs_check_load_F -
                    code_buf - offset) >> 2;
            break;
        case 0x2:
            ins_offset = (latxs_sc_fcs_check_load_S -
                    code_buf - offset) >> 2;
            break;
        case 0x3:
            ins_offset = (latxs_sc_fcs_check_load_FS -
                    code_buf - offset) >> 2;
            break;
        default:
            return;
        }

        latxs_append_ir2_jmp_far(ins_offset, 1);
    }
}
