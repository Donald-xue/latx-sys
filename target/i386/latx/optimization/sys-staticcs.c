#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

int scs_enabled(void)
{
    return option_staticcs;
}

/*
 * All temp registers should not be touch by static
 * generated prologue and epilogue codes.
 * Use stmp1_ir2_opnd & stmp2_ir2_opnd to mov data
 */

int gen_latxs_scs_prologue_cfg(
        void *code_ptr,
        helper_cfg_t cfg)
{
    if (!scs_enabled()) {
        lsassertm(0, "StaticCS not enabled but generated.\n");
        return 0;
    }

    latxs_tr_init(NULL);

    int ra_reg_num = latxs_ir2_opnd_reg(&latxs_ra_ir2_opnd);
    latxs_append_ir2_opnd2i(LISA_ST_D, &latxs_ra_ir2_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_mips_regs(lsenv, ra_reg_num));

    /* FPU TOP will be stored outside */
    if ((cfg.sv_allgpr)) {
        latxs_tr_gen_static_save_registers_to_env(
                0xff, 0xff, 0xff, 0xff, 0x2);
    } else {
        latxs_tr_gen_static_save_registers_to_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x2);
    }

    if (cfg.sv_eflags) {
        latxs_tr_gen_static_save_eflags();
    }

    if (cfg.cvt_fp80) {
        lsassertm(0, "cvtfp not supported in staticcs.\n");
    }

    latxs_append_ir2_opnd2i(LISA_LD_D, &latxs_ra_ir2_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_mips_regs(lsenv, ra_reg_num));
    latxs_append_ir2_opnd0_(lisa_return);

    /* */
    int code_nr  = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}

int gen_latxs_scs_epilogue_cfg(
        void *code_ptr,
        helper_cfg_t cfg)
{
    if (!scs_enabled()) {
        lsassertm(0, "StaticCS not enabled but generated.\n");
        return 0;
    }

    latxs_tr_init(NULL);

    int ra_reg_num = latxs_ir2_opnd_reg(&ra_ir2_opnd);
    latxs_append_ir2_opnd2i(LISA_ST_D, &latxs_ra_ir2_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_mips_regs(lsenv, ra_reg_num));

    if (cfg.cvt_fp80) {
        lsassertm(0, "cvtfp not supported in staticcs.\n");
    }

    if (cfg.sv_eflags) {
        latxs_tr_gen_static_load_eflags(0);
    }

    /* FPU TOP will be loaded outside */
    if (cfg.sv_allgpr) {
        latxs_tr_gen_static_load_registers_from_env(
                0xff, 0xff, 0xff, 0xff, 0x2);
    } else {
        latxs_tr_gen_static_load_registers_from_env(
                GPR_USEDEF_TO_SAVE,
                FPR_USEDEF_TO_SAVE,
                XMM_LO_USEDEF_TO_SAVE,
                XMM_HI_USEDEF_TO_SAVE,
                0x2);
    }

    latxs_append_ir2_opnd2i(LISA_LD_D, &latxs_ra_ir2_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_mips_regs(lsenv, ra_reg_num));
    latxs_append_ir2_opnd0_(lisa_return);

    int code_nr = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}

void latxs_tr_gen_static_save_registers_to_env(
        uint8_t gpr_to_save,
        uint8_t fpr_to_save,
        uint8_t xmm_lo_to_save,
        uint8_t xmm_hi_to_save,
        uint8_t vreg_to_save)
{
    latxs_tr_save_gprs_to_env(gpr_to_save);
    if (option_lsfpu && !option_soft_fpu) {
        latxs_tr_save_lstop_to_env(&latxs_stmp1_ir2_opnd);
        latxs_tr_fpu_disable_top_mode();
    }
    latxs_tr_save_fprs_to_env(fpr_to_save, 0);
    latxs_tr_save_xmms_to_env(xmm_lo_to_save, xmm_hi_to_save);
    latxs_tr_save_vreg_to_env(vreg_to_save);
}

void latxs_tr_gen_static_load_registers_from_env(
        uint8_t gpr_to_load,
        uint8_t fpr_to_load,
        uint8_t xmm_lo_to_load,
        uint8_t xmm_hi_to_load,
        uint8_t vreg_to_load)
{
    latxs_tr_load_vreg_from_env(vreg_to_load);
    latxs_tr_load_xmms_from_env(xmm_lo_to_load, xmm_hi_to_load);
    latxs_tr_load_fprs_from_env(fpr_to_load, 0);
    if (option_lsfpu && !option_soft_fpu) {
        latxs_tr_load_lstop_from_env(&latxs_stmp1_ir2_opnd);
        latxs_tr_fpu_enable_top_mode();
    }
    latxs_tr_load_gprs_from_env(gpr_to_load);
}
