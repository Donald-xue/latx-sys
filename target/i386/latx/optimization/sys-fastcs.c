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

/*
 * option_fastcs
 * = 0 : disabled
 * = 1 : enable TB ctx only
 * = 2 : use jmp-glue method (TB ctx enabled)
 * = 3 : use load-exception method (TB ctx enabled)
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
    return option_fastcs == FASTCS_JMP_GLUE;
}

int latxs_fastcs_is_ld_excp(void)
{
    return option_fastcs == FASTCS_LD_EXCP;
}

void latxs_fastcs_env_init(CPUX86State *env)
{
    env->fastcs_ctx = FASTCS_CTX_NON;

    if (env->fastcs_ptr != NULL) {
        return;
    }

    assert(lsenv->cpu_state == env);

    env->fastcs_ptr = &lsenv->fastcs_data;
    lsenv->fastcs_data.env = env;
}

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
        uint32_t gpr_to_load, uint8_t fpr_to_load, int load_top,
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
    latxs_tr_load_fprs_from_env(fpr_to_load, load_top);
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
        uint32_t gpr_to_save, uint8_t fpr_to_save, int save_top,
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
    latxs_tr_save_fprs_to_env(fpr_to_save, save_top);
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

    __latxs_fastcs_load_registers(gpr, fpr, load_top,
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

    __latxs_fastcs_save_registers(gpr, fpr, save_top,
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

    __latxs_fastcs_load_registers(gpr, fpr, 0,
                                  xmm, vreg, ctx, tmp);

    if (option_lsfpu && !option_soft_fpu) {
        latxs_tr_load_lstop_from_env(&latxs_stmp1_ir2_opnd);
        latxs_tr_fpu_enable_top_mode();
    }
}

void latxs_fastcs_static_save_registers(
        uint32_t gpr, uint8_t fpr,
        uint32_t xmm, uint8_t vreg)
{
    lsassert(latxs_fastcs_enabled());

    IR2_OPND *ctx = &latxs_stmp1_ir2_opnd;
    IR2_OPND *tmp = &latxs_stmp2_ir2_opnd;

    if (option_lsfpu && !option_soft_fpu) {
        latxs_tr_save_lstop_to_env(&latxs_stmp1_ir2_opnd);
        latxs_tr_fpu_disable_top_mode();
    }

    __latxs_fastcs_save_registers(gpr, fpr, 0,
                                  xmm, vreg, ctx, tmp);
}
