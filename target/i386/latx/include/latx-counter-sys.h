#ifndef _LATX_COUNTER_SYS_H_
#define _LATX_COUNTER_SYS_H_

/*#define BG_COUNTER_ENABLE*/

#ifdef BG_COUNTER_ENABLE

#define latxs_counter_tb_tr(cpu) do {   \
      __latxs_counter_tb_tr(cpu);       \
} while (0)
#define latxs_counter_tb_inv(cpu) do {  \
      __latxs_counter_tb_inv(cpu);      \
} while (0)
#define latxs_counter_tb_flush(cpu) do {    \
      __latxs_counter_tb_flush(cpu);        \
} while (0)
#define latxs_counter_tb_lookup(cpu) do {   \
      __latxs_counter_tb_lookup(cpu);       \
} while (0)
#define latxs_counter_tb_lookup_ht(cpu) do {   \
      __latxs_counter_tb_lookup_ht(cpu);       \
} while (0)

#define latxs_counter_jc_flush(cpu) do {    \
      __latxs_counter_jc_flush(cpu);        \
} while (0)
#define latxs_counter_jc_flush_page(cpu) do {   \
      __latxs_counter_jc_flush_page(cpu);       \
} while (0)
#define latxs_counter_jc_flush_page_go(cpu) do {    \
      __latxs_counter_jc_flush_page_go(cpu);        \
} while (0)
#define latxs_counter_jc_flush_page_do(cpu) do {    \
      __latxs_counter_jc_flush_page_do(cpu);        \
} while (0)

#define latxs_counter_helper_store(cpu) do {    \
      __latxs_counter_helper_store(cpu);        \
} while (0)
#define latxs_counter_helper_store_io(cpu) do { \
      __latxs_counter_helper_store_io(cpu);     \
} while (0)
#define latxs_counter_helper_store_stlbfill(cpu) do {   \
      __latxs_counter_helper_store_stlbfill(cpu);       \
} while (0)
#define latxs_counter_helper_load(cpu) do { \
      __latxs_counter_helper_load(cpu);     \
} while (0)
#define latxs_counter_helper_load_io(cpu) do {  \
      __latxs_counter_helper_load_io(cpu);      \
} while (0)
#define latxs_counter_helper_load_stlbfill(cpu) do {    \
      __latxs_counter_helper_load_stlbfill(cpu);        \
} while (0)

#define latxs_counter_helper_store_cpl3(env, cpu) do {  \
    if ((env->hflags & 0x3) == 0x3)                     \
      __latxs_counter_helper_store_cpl3(cpu);           \
} while (0)
#define latxs_counter_helper_store_stlbfill_cpl3(env, cpu) do { \
    if ((env->hflags & 0x3) == 0x3)                             \
      __latxs_counter_helper_store_stlbfill_cpl3(cpu);          \
} while (0)
#define latxs_counter_helper_load_cpl3(env, cpu) do {   \
    if ((env->hflags & 0x3) == 0x3)                     \
      __latxs_counter_helper_load_cpl3(cpu);            \
} while (0)
#define latxs_counter_helper_load_stlbfill_cpl3(env, cpu) do {  \
    if ((env->hflags & 0x3) == 0x3)                             \
      __latxs_counter_helper_load_stlbfill_cpl3(cpu);           \
} while (0)

#define latxs_counter_wake(cpu) do {    \
      __latxs_counter_wake(cpu);        \
} while (0)

void __latxs_counter_tb_tr(void *cpu);
void __latxs_counter_tb_inv(void *cpu);
void __latxs_counter_tb_flush(void *cpu);
void __latxs_counter_tb_lookup(void *cpu);
void __latxs_counter_tb_lookup_ht(void *cpu);

void __latxs_counter_jc_flush(void *cpu);
void __latxs_counter_jc_flush_page(void *cpu);
void __latxs_counter_jc_flush_page_go(void *cpu);
void __latxs_counter_jc_flush_page_do(void *cpu);

void __latxs_counter_helper_store(void *cpu);
void __latxs_counter_helper_store_io(void *cpu);
void __latxs_counter_helper_store_stlbfill(void *cpu);
void __latxs_counter_helper_load(void *cpu);
void __latxs_counter_helper_load_io(void *cpu);
void __latxs_counter_helper_load_stlbfill(void *cpu);

void __latxs_counter_helper_store_cpl3(void *cpu);
void __latxs_counter_helper_store_stlbfill_cpl3(void *cpu);
void __latxs_counter_helper_load_cpl3(void *cpu);
void __latxs_counter_helper_load_stlbfill_cpl3(void *cpu);

void __latxs_counter_wake(void *cpu);

#else /* disable bg thread counter */

#define latxs_counter_tb_tr(cpu)
#define latxs_counter_tb_inv(cpu)
#define latxs_counter_tb_flush(cpu)
#define latxs_counter_tb_lookup(cpu)
#define latxs_counter_tb_lookup_ht(cpu)

#define latxs_counter_jc_flush(cpu)
#define latxs_counter_jc_flush_page(cpu)
#define latxs_counter_jc_flush_page_go(cpu)
#define latxs_counter_jc_flush_page_do(cpu)

#define latxs_counter_helper_store(cpu)
#define latxs_counter_helper_store_io(cpu)
#define latxs_counter_helper_store_stlbfill(cpu)
#define latxs_counter_helper_load(cpu)
#define latxs_counter_helper_load_io(cpu)
#define latxs_counter_helper_load_stlbfill(cpu)

#define latxs_counter_helper_store_cpl3(env, cpu)
#define latxs_counter_helper_store_stlbfill_cpl3(env, cpu)
#define latxs_counter_helper_load_cpl3(env, cpu)
#define latxs_counter_helper_load_stlbfill_cpl3(env, cpu)

#define latxs_counter_wake(cpu)

#endif /* BG_COUNTER_ENABLE */

#endif
