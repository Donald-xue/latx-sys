#ifndef _LATX_CONFIG_H_
#define _LATX_CONFIG_H_

#include "latx-types.h"
#include "qemu/osdep.h"
#include "exec/exec-all.h"

extern ADDR context_switch_bt_to_native;

int target_latx_host(CPUArchState *env, struct TranslationBlock *tb);
int target_latx_prologue(void *code_buf_addr);
int target_latx_epilogue(void *code_buf_addr);
int target_latx_fpu_rotate(void *code_buf_addr);
void latx_tb_set_jmp_target(struct TranslationBlock *, int, struct TranslationBlock *);

#ifdef CONFIG_LATX_DEBUG
void latx_before_exec_trace_tb(CPUArchState *env, struct TranslationBlock *tb);
void latx_after_exec_trace_tb(CPUArchState *env, struct TranslationBlock *tb);
void latx_profile(void);
void trace_tb_execution(struct TranslationBlock *tb);
#endif
void latx_before_exec_rotate_fpu(CPUArchState *env, struct TranslationBlock *tb);
void latx_after_exec_rotate_fpu(CPUArchState *env, struct TranslationBlock *tb);
struct TranslationBlock *latx_tb_find(void *cpu_state, ADDRX pc);
void latx_exit(void);


void latx_lsenv_init(CPUArchState *env);
void latx_set_tls_ibtc_table(CPUArchState *env);

/* defined in etb.c, but declared here
 * to avoid strange compile warning
 */
ETB *etb_find(ADDRX pc);
bool etb_remove(ADDRX pc);
#endif /* _LATX_CONFIG_H_ */
