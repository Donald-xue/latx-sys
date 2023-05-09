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
#define latxs_counter_tb_lookup_cpl3(env, cpu) do { \
    if ((env->hflags & 0x3) == 0x3)                 \
      __latxs_counter_tb_lookup_cpl3(cpu);          \
} while (0)
#define latxs_counter_tb_lookup_ht_cpl3(env, cpu) do {  \
    if ((env->hflags & 0x3) == 0x3)                     \
      __latxs_counter_tb_lookup_ht_cpl3(cpu);           \
} while (0)

#define latxs_counter_excp_pf(cpu) do {    \
      __latxs_counter_excp_pf(cpu);        \
} while (0)
#define latxs_counter_excp_pf_cpl3(env, cpu) do {   \
    if ((env->hflags & 0x3) == 0x3)                 \
      __latxs_counter_excp_pf_cpl3(cpu);            \
} while (0)

#define latxs_counter_interrupt(cpu) do {    \
      __latxs_counter_interrupt(cpu);        \
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

#define latxs_counter_exe_write_cr0(cpu) do {   \
      __latxs_counter_exe_write_cr0(cpu);       \
} while (0)
#define latxs_counter_exe_write_cr3(cpu) do {   \
      __latxs_counter_exe_write_cr3(cpu);       \
} while (0)

#define latxs_counter_exe_invlpg(cpu) do {  \
      __latxs_counter_exe_invlpg(cpu);      \
} while (0)

#define latxs_counter_stlb_flush_full(cpu) do { \
      __latxs_counter_stlb_flush_full(cpu);     \
} while (0)
#define latxs_counter_stlb_flush_part(cpu) do { \
      __latxs_counter_stlb_flush_part(cpu);     \
} while (0)
#define latxs_counter_stlb_flush_elide(cpu) do {    \
      __latxs_counter_stlb_flush_elide(cpu);        \
} while (0)
#define latxs_counter_stlb_flush_page(cpu) do { \
      __latxs_counter_stlb_flush_page(cpu);     \
} while (0)
#define latxs_counter_stlb_resize(cpu) do { \
      __latxs_counter_stlb_resize(cpu);     \
} while (0)
#define latxs_counter_stlb_resize_inc(cpu) do { \
      __latxs_counter_stlb_resize_inc(cpu);     \
} while (0)
#define latxs_counter_stlb_resize_dec(cpu) do { \
      __latxs_counter_stlb_resize_dec(cpu);     \
} while (0)

#define latxs_counter_hamt_fast(cpu) do {    \
      __latxs_counter_hamt_fast(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_ld(cpu) do {    \
      __latxs_counter_hamt_fast_ld(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_ld_ok(cpu) do { \
      __latxs_counter_hamt_fast_ld_ok(cpu);     \
} while (0)
#define latxs_counter_hamt_fast_st(cpu) do {    \
      __latxs_counter_hamt_fast_st(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_st_ok(cpu) do { \
      __latxs_counter_hamt_fast_st_ok(cpu);     \
} while (0)
#define latxs_counter_hamt_fast_ld_stlb_ok(cpu) do {    \
      __latxs_counter_hamt_fast_ld_stlb_ok(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_st_stlb_ok(cpu) do {    \
      __latxs_counter_hamt_fast_st_stlb_ok(cpu);        \
} while (0)

#define latxs_counter_hamt_ld_stlb_ok(cpu) do {    \
      __latxs_counter_hamt_ld_stlb_ok(cpu);        \
} while (0)
#define latxs_counter_hamt_st_stlb_ok(cpu) do {    \
      __latxs_counter_hamt_st_stlb_ok(cpu);        \
} while (0)


#define latxs_counter_wake(cpu) do {    \
      __latxs_counter_wake(cpu);        \
} while (0)

void __latxs_counter_tb_tr(void *cpu);
void __latxs_counter_tb_inv(void *cpu);
void __latxs_counter_tb_flush(void *cpu);
void __latxs_counter_tb_lookup(void *cpu);
void __latxs_counter_tb_lookup_ht(void *cpu);
void __latxs_counter_tb_lookup_cpl3(void *cpu);
void __latxs_counter_tb_lookup_ht_cpl3(void *cpu);

void __latxs_counter_excp_pf(void *cpu);
void __latxs_counter_excp_pf_cpl3(void *cpu);

void __latxs_counter_interrupt(void *cpu);

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

void __latxs_counter_exe_write_cr0(void *cpu);
void __latxs_counter_exe_write_cr3(void *cpu);

void __latxs_counter_exe_invlpg(void *cpu);

void __latxs_counter_stlb_flush_full(void *cpu);
void __latxs_counter_stlb_flush_part(void *cpu);
void __latxs_counter_stlb_flush_elide(void *cpu);
void __latxs_counter_stlb_flush_page(void *cpu);
void __latxs_counter_stlb_resize(void *cpu);
void __latxs_counter_stlb_resize_inc(void *cpu);
void __latxs_counter_stlb_resize_dec(void *cpu);

void __latxs_counter_hamt_fast(void *cpu);
void __latxs_counter_hamt_fast_ld(void *cpu);
void __latxs_counter_hamt_fast_ld_ok(void *cpu);
void __latxs_counter_hamt_fast_ld_stlb_ok(void *cpu);
void __latxs_counter_hamt_fast_st(void *cpu);
void __latxs_counter_hamt_fast_st_ok(void *cpu);
void __latxs_counter_hamt_fast_st_stlb_ok(void *cpu);

void __latxs_counter_hamt_ld_stlb_ok(void *cpu);
void __latxs_counter_hamt_st_stlb_ok(void *cpu);

void __latxs_counter_wake(void *cpu);

#else /* disable bg thread counter */

#define latxs_counter_tb_tr(cpu)
#define latxs_counter_tb_inv(cpu)
#define latxs_counter_tb_flush(cpu)
#define latxs_counter_tb_lookup(cpu)
#define latxs_counter_tb_lookup_ht(cpu)
#define latxs_counter_tb_lookup_cpl3(env, cpu)
#define latxs_counter_tb_lookup_ht_cpl3(env, cpu)

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

#define latxs_counter_excp_pf(cpu)
#define latxs_counter_excp_pf_cpl3(env, cpu)

#define latxs_counter_interrupt(cpu)

#define latxs_counter_exe_write_cr0(cpu)
#define latxs_counter_exe_write_cr3(cpu)
#define latxs_counter_exe_invlpg(cpu)

#define latxs_counter_stlb_flush_full(cpu)
#define latxs_counter_stlb_flush_part(cpu)
#define latxs_counter_stlb_flush_elide(cpu)
#define latxs_counter_stlb_flush_page(cpu)
#define latxs_counter_stlb_resize(cpu)
#define latxs_counter_stlb_resize_inc(cpu)
#define latxs_counter_stlb_resize_dec(cpu)

#define latxs_counter_hamt_fast(cpu)
#define latxs_counter_hamt_fast_ld(cpu)
#define latxs_counter_hamt_fast_ld_ok(cpu)
#define latxs_counter_hamt_fast_ld_stlb_ok(cpu)
#define latxs_counter_hamt_fast_st(cpu)
#define latxs_counter_hamt_fast_st_ok(cpu)
#define latxs_counter_hamt_fast_st_stlb_ok(cpu)

#define latxs_counter_hamt_ld_stlb_ok(cpu)
#define latxs_counter_hamt_st_stlb_ok(cpu)

#define latxs_counter_wake(cpu)

#endif /* BG_COUNTER_ENABLE */

#endif
