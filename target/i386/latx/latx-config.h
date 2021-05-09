#ifndef _LATX_CONFIG_H_
#define _LATX_CONFIG_H_

#include "types.h"
#include "qemu-def.h"

int target_latx_host(CPUArchState *env, struct TranslationBlock *tb);
int target_latx_prologue(void *code_buf_addr);
int target_latx_epilogue(void *code_buf_addr);
int target_latx_fpu_rotate(void *code_buf_addr);
void latx_tb_set_jmp_target(TranslationBlock *, int, TranslationBlock *);

void latx_before_exec_tb(CPUArchState *env, struct TranslationBlock *tb);
void latx_after_exec_tb(CPUArchState *env, struct TranslationBlock *tb);
struct TranslationBlock *latx_tb_find(void *cpu_state, ADDRX pc);

extern ADDR context_switch_bt_to_native;
extern ADDR context_switch_native_to_bt_ret_0;
extern ADDR context_switch_native_to_bt;
extern int context_switch_is_init;
extern ADDR ss_match_fail_native;

void trace_tb_execution(struct TranslationBlock *tb);
extern void etb_check_top_in(struct TranslationBlock *tb, int top);

#endif /* _LATX_CONFIG_H_ */
