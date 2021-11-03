#ifndef SIGINT_I386_TCG_LA_H
#define SIGINT_I386_TCG_LA_H

void tcgsigint_tb_gen_start(
        CPUState *cpu, TranslationBlock *tb);
void tcgsigint_tb_gen_end(
        CPUState *cpu, TranslationBlock *tb);

void rr_cpu_tcgsigint_init(CPUState *cpu);

void tcgsigint_cpu_loop_exit(CPUState *cpu);
void tcgsigint_before_tb_exec(CPUState *cpu, void *tb);
void tcgsigint_after_tb_exec(CPUState *cpu);

void tcgsigint_remove_tb_from_jmp_list(void *tb, int n);

#define TCG_SIGINT_MODE_NONE        0
#define TCG_SIGINT_MODE_UNLINK_ONE  1
#define TCG_SIGINT_MODE_UNLINK_ALL  2

int tcgsigint_mode(void);

void tcgsigint_unlink_tb_all(CPUState *cpu);

#endif
