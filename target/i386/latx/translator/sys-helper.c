#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "fpu/softfloat.h"
#include <string.h>

helper_cfg_t all_helper_cfg = {
    .sv_allgpr = 1,
    .sv_eflags = 1,
    .cvt_fp80  = 1
};
helper_cfg_t zero_helper_cfg = {
    .sv_allgpr = 0,
    .sv_eflags = 0,
    .cvt_fp80  = 0
};
helper_cfg_t default_helper_cfg = {
    .sv_allgpr = 1,
    .sv_eflags = 1,
    .cvt_fp80  = 0
};

/*
 * return 1: cfg1 == cfg2
 * return 0: cfg1 != cfg2
 */
int cmp_helper_cfg(helper_cfg_t cfg1, helper_cfg_t cfg2)
{
    return cfg1.sv_allgpr == cfg2.sv_allgpr &&
           cfg1.sv_eflags == cfg2.sv_eflags &&
           cfg1.cvt_fp80  == cfg2.cvt_fp80;
}

IR2_INST *latxs_tr_gen_call_to_helper(ADDR func_addr)
{
    IR2_OPND func = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&func, (ADDR)func_addr);
    latxs_append_ir2_opnd1_(lisa_call, &func);
    latxs_ra_free_temp(&func);
    return NULL;
}

static void latxs_convert_fpregs_64_to_x80(void)
{
    int i;
    CPUX86State *env = (CPUX86State *)lsenv->cpu_state;
    float_status s = env->fp_status;
    for (i = 0; i < 8; i++) {
        FPReg *p = &(env->fpregs[i]);
        p->d = float64_to_floatx80((float64)p->d.low, &s);
    }
}

static void latxs_convert_fpregs_x80_to_64(void)
{
    int i;
    CPUX86State *env = (CPUX86State *)lsenv->cpu_state;
    float_status s = env->fp_status;
    for (i = 0; i < 8; i++) {
        FPReg *p = &(env->fpregs[i]);
        p->d.low = (uint64_t)floatx80_to_float64(p->d, &s);
        p->d.high = 0;
    }
}

void latxs_tr_cvt_fp64_to_80(void)
{
    /* 1. call convert_fpregs_x64_to_80 */
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&func_addr_opnd,
            (ADDR)latxs_convert_fpregs_64_to_x80);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);

    /* 2. call update_fp_status */
    latxs_load_addr_to_ir2(&func_addr_opnd, (ADDR)update_fp_status);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);
}

void latxs_tr_cvt_fp80_to_64(void)
{
    /* 1. save return value */
    latxs_append_ir2_opnd2i(LISA_ST_D,
            &latxs_ret0_ir2_opnd, &latxs_env_ir2_opnd,
            lsenv_offset_of_mips_regs(lsenv, 0x2));

    /* 2. call convert_fpregs_x80_to_64 */
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&func_addr_opnd,
            (ADDR)latxs_convert_fpregs_x80_to_64);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    /* 3. restore return value */
    latxs_append_ir2_opnd2i(LISA_LD_D,
            &latxs_ret0_ir2_opnd, &latxs_env_ir2_opnd,
            lsenv_offset_of_mips_regs(lsenv, 0x2));
}

/*
 * Prologue and Epilogue for helper function in system-mode
 *
 * If the helper requires complete ENV context, set @all_gpr to save
 * all the mapping registers.
 *
 * In the following situations, complete ENV is required:
 *  > helper that reads/writes all the GPRs...
 *      > helper_rsm
 *  > helper that might not return
 *      > helper_raise_exception: always no return
 *      > helper_raise_interrupt: always no return
 *      > helper that might triger interrupt/exception
 *          > many helper for system instruction
 *            - softmmu's helper : load/store
 *            - helper_ltr   : ltr
 *            - helper_lldt  : lldt
 *            - helper_pause : nop
 */
void latxs_tr_gen_call_to_helper_prologue_cfg(helper_cfg_t cfg)
{
    /* Use static helper prologue for default */
    if (scs_enabled() && cmp_helper_cfg(cfg, default_helper_cfg)) {
        TRANSLATION_DATA *td = lsenv->tr_data;
        TranslationBlock *tb = td->curr_tb;
        void *code_buf = tb->tc.ptr;
        int offset = td->real_ir2_inst_num << 2;
        latxs_append_ir2_opnda(LISA_BL,
                (latxs_sc_scs_prologue - (ADDR)code_buf - offset) >> 2);
        if (!option_lsfpu) {
            latxs_tr_gen_save_curr_top();
        }
        return;
    }


    if (likely(cfg.sv_allgpr)) {
        latxs_tr_save_registers_to_env(
                0xff, 0xff, 1, 0xff, 0xff, 0x7);
    } else {
        latxs_tr_save_registers_to_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE, 1,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x7);
    }

    if (likely(cfg.sv_eflags)) {
        latxs_tr_save_eflags();
    }

    if (unlikely(cfg.cvt_fp80)) {
        latxs_tr_save_temp_register();
        latxs_tr_cvt_fp64_to_80();
        latxs_tr_restore_temp_register();
    }
}

void latxs_tr_gen_call_to_helper_epilogue_cfg(helper_cfg_t cfg)
{
    /*
     * X86 GPRs will be store and read again for helper:
     *   > After st_w and ld_w, GPRs' EM need to be fixed
     *   > For softmmu helper, it will fix the EM through
     *      another way in tr_softmmu.c
     */
    TRANSLATION_DATA *td = lsenv->tr_data;
    int fix_em = option_by_hand && !(td->in_gen_slow_path);

    /* Use static helper epilogue for default */
    if (scs_enabled() && cmp_helper_cfg(cfg, default_helper_cfg)) {
        TranslationBlock *tb = td->curr_tb;
        void *code_buf = tb->tc.ptr;
        int offset = td->real_ir2_inst_num << 2;
        latxs_append_ir2_opnda(LISA_BL,
                (latxs_sc_scs_epilogue - (ADDR)code_buf - offset) >> 2);
        if (fix_em) {
            latxs_td_set_reg_extmb_after_cs(0xFF);
        }
        return;
    }


    if (unlikely(cfg.cvt_fp80)) {
        latxs_tr_cvt_fp80_to_64();
    }

    if (likely(cfg.sv_eflags)) {
        latxs_tr_load_eflags(0);
    }

    if (likely(cfg.sv_allgpr)) {
        latxs_tr_load_registers_from_env(0xff, 0xff, 1, 0xff, 0xff, 0x7);
        if (fix_em) {
            latxs_td_set_reg_extmb_after_cs(0xFF);
        }
    } else {
        latxs_tr_load_registers_from_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE, 1,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x7);
        if (fix_em) {
            lsassertm(0, "not ready.\n");
            latxs_td_set_reg_extmb_after_cs(GPR_USEDEF_TO_SAVE);
        }
    }
}

/* helper with zero arg */
void latxs_tr_gen_call_to_helper0_cfg(ADDR func, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();

    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    latxs_load_addr_to_ir2(&func_addr_opnd, (ADDR)func);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 1 default arg(CPUArchState*) */
void latxs_tr_gen_call_to_helper1_cfg(ADDR func, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();

    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    latxs_load_addr_to_ir2(&func_addr_opnd, (ADDR)func);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 2 arg(CPUArchState*, int) */
void latxs_tr_gen_call_to_helper2_cfg(ADDR func,
        int arg2, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();

    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    latxs_load_addr_to_ir2(&func_addr_opnd, func);
    latxs_load_imm32_to_ir2(&latxs_arg1_ir2_opnd, arg2, EXMode_S);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 3 arg(CPUArchState*, int, int) */
void latxs_tr_gen_call_to_helper3_cfg(ADDR func,
        int arg2, int arg3, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();

    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    latxs_load_addr_to_ir2(&func_addr_opnd, func);
    latxs_load_imm32_to_ir2(&latxs_arg1_ir2_opnd, arg2, EXMode_S);
    latxs_load_imm32_to_ir2(&latxs_arg2_ir2_opnd, arg3, EXMode_S);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

/* helper with 3 arg(CPUArchState*, uint64, uint64) */
void latxs_tr_gen_call_to_helper3_u64_cfg(ADDR func,
        uint64_t arg2, uint64_t arg3, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();

    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    latxs_load_addr_to_ir2(&func_addr_opnd, func);
    latxs_load_imm64_to_ir2(&latxs_arg1_ir2_opnd, arg2);
    latxs_load_imm64_to_ir2(&latxs_arg2_ir2_opnd, arg3);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

void latxs_tr_gen_call_to_helper4_u64_cfg(ADDR func,
        uint64_t arg2, uint64_t arg3, uint64_t arg4, helper_cfg_t cfg)
{
    IR2_OPND func_addr_opnd = latxs_ra_alloc_itemp();

    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    latxs_load_addr_to_ir2(&func_addr_opnd, func);
    latxs_load_imm64_to_ir2(&latxs_arg1_ir2_opnd, arg2);
    latxs_load_imm64_to_ir2(&latxs_arg2_ir2_opnd, arg3);
    latxs_load_imm64_to_ir2(&latxs_arg3_ir2_opnd, arg4);

    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    latxs_append_ir2_opnd1_(lisa_call, &func_addr_opnd);
    latxs_ra_free_temp(&func_addr_opnd);

    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);
}

void latxs_tr_gen_infinite_loop(void)
{
    IR2_OPND label_opnd = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_LABEL, &label_opnd);
    latxs_append_ir2_opnd1(LISA_B, &label_opnd);
    latxs_append_ir2_opnd0_(lisa_nop);
}

void latxs_tr_gen_save_curr_eip(void)
{
    lsassert(lsenv->tr_data->curr_ir1_inst);

    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
    ADDRX curr_eip = ir1_addr(pir1);
    IR2_OPND eip_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&eip_opnd, curr_eip);

    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_eip(lsenv));

    latxs_ra_free_temp(&eip_opnd);
}

void latxs_tr_gen_save_next_eip(void)
{
    lsassert(lsenv->tr_data->curr_ir1_inst);

    IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
    ADDRX next_eip = ir1_addr_next(pir1);
    IR2_OPND eip_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&eip_opnd, next_eip);

    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_eip(lsenv));

    latxs_ra_free_temp(&eip_opnd);
}

void latxs_tr_save_temp_register_mask(int mask)
{
    const TEMP_REG_STATUS *p = latxs_itemp_status_default;
    int i;
    for (i = 0; i < latxs_itemp_status_num; ++i) {
        if ((mask >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = latxs_ir2_opnd_new(IR2_OPND_GPR, ireg);
            latxs_append_ir2_opnd2i(LISA_ST_D,
                    &gpr, &latxs_env_ir2_opnd,
                    lsenv_offset_of_mips_regs(lsenv, ireg));
        }
    }
}

void latxs_tr_restore_temp_register_mask(int mask)
{
    const TEMP_REG_STATUS *p = latxs_itemp_status_default;
    int i;
    for (i = 0; i < latxs_itemp_status_num; ++i) {
        if ((mask >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = latxs_ir2_opnd_new(IR2_OPND_GPR, ireg);
            latxs_append_ir2_opnd2i(LISA_LD_D,
                    &gpr, &latxs_env_ir2_opnd,
                    lsenv_offset_of_mips_regs(lsenv, ireg));
        }
    }
}


/* Only need to save current used temp registers */
void latxs_tr_save_temp_register(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    const TEMP_REG_STATUS *p = latxs_itemp_status_default;

    int mask = td->itemp_mask;
    td->itemp_mask_bk = mask;

    lsassertm(!(td->itemp_saved),
            "Can not save temp again after save temp.\n");

    int i;
    for (i = 0; i < latxs_itemp_status_num; ++i) {
        /* if ((mask >> i) & 0x1) { */
            int ireg = p[i].physical_id;
            IR2_OPND gpr = latxs_ir2_opnd_new(IR2_OPND_GPR, ireg);
            latxs_append_ir2_opnd2i(LISA_ST_D,
                    &gpr, &latxs_env_ir2_opnd,
                    lsenv_offset_of_mips_regs(lsenv, ireg));
            /* pb[i].virtual_id = p[i].virtual_id; */
        /* } */
    }

    td->itemp_saved = 1;
}

/* Only need to restore saved temp registers */
void latxs_tr_restore_temp_register(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    const TEMP_REG_STATUS *p = latxs_itemp_status_default;

    int mask_bk = td->itemp_mask_bk;
    td->itemp_mask = mask_bk;

    lsassertm(td->itemp_saved,
            "Can not resoter temp again if not save temp.\n");

    int i;
    for (i = 0; i < latxs_itemp_status_num; ++i) {
        if ((mask_bk >> i) & 0x1) {
            int ireg = p[i].physical_id;
            IR2_OPND gpr = latxs_ir2_opnd_new(IR2_OPND_GPR, ireg);
            latxs_append_ir2_opnd2i(LISA_LD_D,
                    &gpr, &latxs_env_ir2_opnd,
                    lsenv_offset_of_mips_regs(lsenv, ireg));
        }
    }

    td->itemp_saved = 0;
}
