#ifndef _LATX_STATIC_CODES_H_
#define _LATX_STATIC_CODES_H_

#ifdef CONFIG_SOFTMMU
/* latxs_sc_bpc in latx-bpc-sys.h */

extern ADDR latxs_sc_intb_njc;

extern ADDR latxs_sc_fcs_jmp_glue_fpu_0;
extern ADDR latxs_sc_fcs_jmp_glue_fpu_1;
extern ADDR latxs_sc_fcs_jmp_glue_xmm_0;
extern ADDR latxs_sc_fcs_jmp_glue_xmm_1;

/* latxs_native_printer in latx-np-sys.h */

extern ADDR latxs_sc_scs_prologue;
extern ADDR latxs_sc_scs_epilogue;

extern ADDR latxs_sc_fcs_F_0;
extern ADDR latxs_sc_fcs_F_1;
extern ADDR latxs_sc_fcs_S_0;
extern ADDR latxs_sc_fcs_S_1;
extern ADDR latxs_sc_fcs_FS_0;
extern ADDR latxs_sc_fcs_FS_1;
extern ADDR latxs_sc_fcs_check_load_F;
extern ADDR latxs_sc_fcs_check_load_S;
extern ADDR latxs_sc_fcs_check_load_FS;
extern ADDR latxs_sc_fcs_load_F;
extern ADDR latxs_sc_fcs_load_S;
extern ADDR latxs_sc_fcs_load_FS;

/* latxs_sc_intb_lookup in latx-intb-sys.h */
#endif

extern ADDR context_switch_bt_to_native;
extern ADDR context_switch_native_to_bt_ret_0;
extern ADDR context_switch_native_to_bt;
extern ADDR ss_match_fail_native;

extern ADDR native_rotate_fpu_by;
extern ADDR native_jmp_glue_0;
extern ADDR native_jmp_glue_1;
extern ADDR native_jmp_glue_2;

extern ADDR fpu_enable_top;
extern ADDR fpu_disable_top;
extern ADDR fpu_get_top;
extern ADDR fpu_set_top;
extern ADDR fpu_inc_top;
extern ADDR fpu_dec_top;

#endif
