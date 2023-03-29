#ifndef _LATX_SIGINT_FN_SYS_H_
#define _LATX_SIGINT_FN_SYS_H_

/* sigint handler for SIGINT in LATX-SYS */

void latxs_init_rr_thread_signal(CPUState *cpu);
void latxs_rr_interrupt_self(CPUState *cpu);

void latxs_sigint_check_in_hamt(CPUX86State *env, void *epc);

#endif
