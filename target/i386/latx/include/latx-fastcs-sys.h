#ifndef _LATX_FASTCS_SYS_H_
#define _LATX_FASTCS_SYS_H_

int latxs_fastcs_enabled(void);
int latxs_fastcs_enable_tbctx(void);

int latxs_fastcs_is_no_link(void);
int latxs_fastcs_is_jmp_glue(void);
int latxs_fastcs_is_jmp_glue_direct(void);
int latxs_fastcs_is_ld_excp(void);
int latxs_fastcs_is_ld_branch(void);
int latxs_fastcs_is_ld_branch_inline(void);

/* Implement in latx/optimization/sys-fascs.c */
void tb_reset_fastcs_jmp_glue(TranslationBlock *tb, int n);

/* Implement in accel/tcg/cpu-exec.c */
int tb_set_jmp_target_fastcs(TranslationBlock *tb, int n,
        TranslationBlock *nextb, uintptr_t fastcs_jmp_glue,
        int is_jmp_glue_return);

/* Implement in tcg/loongarch */
void tb_target_reset_fastcs_jmp_glue(uintptr_t tc_ptr,
        uintptr_t jmp_rx, uintptr_t jmp_rw);
void tb_target_set_jmp_target_fastcs(uintptr_t tc_ptr,
        uintptr_t jmp_rx, uintptr_t jmp_rw, uintptr_t addr, int link);

#endif
