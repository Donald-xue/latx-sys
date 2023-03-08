#ifndef _TCG_BG_THREAD_H_
#define _TCG_BG_THREAD_H_

void tcg_bg_init_rr(CPUState *cpu);
void tcg_bg_init_mt(CPUState *cpu);

void tcg_bg_counter_wake(int sec);

#endif
