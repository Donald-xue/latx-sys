#ifndef _LATX_COUNTER_SYS_H_
#define _LATX_COUNTER_SYS_H_

#include "latx-counter-def.h"

#define BG_COUNTER_DEF_DUMMY    \
    uint64_t dummy;

/* ======================== TB ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_TB)

/*
 * case (1) TB -> helper lookup  -> lookup ------------> TB
 * case (2) TB -> context switch -> tb find -> lookup -> TB
 */
#define BG_COUNTER_DEF_TB                               \
    uint64_t tb_tr_nr;                                  \
    uint64_t tb_inv_nr;                                 \
    uint64_t tb_flush_nr;                               \
    uint64_t tb_lookup_nr;    /* case (2) */            \
    uint64_t tb_lookup_ht_nr; /* case (2) */            \
    uint64_t tb_lookup_cpl3_nr;    /* case (2) CPL3 */  \
    uint64_t tb_lookup_ht_cpl3_nr; /* case (2) CPL3 */

#define BG_COUNTER_LOG_TB       \
    "TR %d INV %d FLUSH %d Lookup %d %d %d %d "

#define BG_COUNTER_LOG_DATA_TB(n)       \
   ,BG_LOG_DIFF(n, tb_tr)               \
   ,BG_LOG_DIFF(n, tb_inv)              \
   ,BG_LOG_DIFF(n, tb_flush)            \
   ,BG_LOG_DIFF(n, tb_lookup)           \
   ,BG_LOG_DIFF(n, tb_lookup_ht)        \
   ,BG_LOG_DIFF(n, tb_lookup_cpl3)      \
   ,BG_LOG_DIFF(n, tb_lookup_ht_cpl3) 

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

void __latxs_counter_tb_tr(void *cpu);
void __latxs_counter_tb_inv(void *cpu);
void __latxs_counter_tb_flush(void *cpu);
void __latxs_counter_tb_lookup(void *cpu);
void __latxs_counter_tb_lookup_ht(void *cpu);
void __latxs_counter_tb_lookup_cpl3(void *cpu);
void __latxs_counter_tb_lookup_ht_cpl3(void *cpu);

#else

#define BG_COUNTER_DEF_TB
#define BG_COUNTER_LOG_TB
#define BG_COUNTER_LOG_DATA_TB(n)

#define latxs_counter_tb_tr(cpu)
#define latxs_counter_tb_inv(cpu)
#define latxs_counter_tb_flush(cpu)
#define latxs_counter_tb_lookup(cpu)
#define latxs_counter_tb_lookup_ht(cpu)
#define latxs_counter_tb_lookup_cpl3(env, cpu)
#define latxs_counter_tb_lookup_ht_cpl3(env, cpu)

#endif

/* ======================== EXCP ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_EXCP)

#define BG_COUNTER_DEF_EXCP     \
    uint64_t excp_pf_nr;        \
    uint64_t excp_pf_cpl3_nr;

#define BG_COUNTER_LOG_EXCP "Excp %d %d "
#define BG_COUNTER_LOG_DATA_EXCP(n) \
   ,BG_LOG_DIFF(n, excp_pf)         \
   ,BG_LOG_DIFF(n, excp_pf_cpl3)

#define latxs_counter_excp_pf(cpu) do {    \
      __latxs_counter_excp_pf(cpu);        \
} while (0)
#define latxs_counter_excp_pf_cpl3(env, cpu) do {   \
    if ((env->hflags & 0x3) == 0x3)                 \
      __latxs_counter_excp_pf_cpl3(cpu);            \
} while (0)

void __latxs_counter_excp_pf(void *cpu);
void __latxs_counter_excp_pf_cpl3(void *cpu);

#else

#define BG_COUNTER_DEF_EXCP
#define BG_COUNTER_LOG_EXCP
#define BG_COUNTER_LOG_DATA_EXCP(n)

#define latxs_counter_excp_pf(cpu)
#define latxs_counter_excp_pf_cpl3(env, cpu)

#endif

/* ======================== INT ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_INT)

#define BG_COUNTER_DEF_INT  \
    uint64_t interrupt_nr;

#define BG_COUNTER_LOG_INT "Intpt %d "
#define BG_COUNTER_LOG_DATA_INT(n) \
   ,BG_LOG_DIFF(n, interrupt)

#define latxs_counter_interrupt(cpu) do {    \
      __latxs_counter_interrupt(cpu);        \
} while (0)

void __latxs_counter_interrupt(void *cpu);

#else

#define BG_COUNTER_DEF_INT
#define BG_COUNTER_LOG_INT
#define BG_COUNTER_LOG_DATA_INT(n)

#define latxs_counter_interrupt(cpu)

#endif

/* ======================== JC ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_JC)

#define BG_COUNTER_DEF_JC           \
    uint64_t jc_flush_nr;           \
    uint64_t jc_flush_bg_nr;        \
    uint64_t jc_flush_page_nr;      \
    uint64_t jc_flush_page_go_nr;   \
    uint64_t jc_flush_page_do_nr;

#define BG_COUNTER_LOG_JC "JCFlush %d %d %d %d %d "
#define BG_COUNTER_LOG_DATA_JC(n)       \
   ,BG_LOG_DIFF(n, jc_flush)            \
   ,BG_LOG_DIFF(n, jc_flush_bg)         \
   ,BG_LOG_DIFF(n, jc_flush_page)       \
   ,BG_LOG_DIFF(n, jc_flush_page_go)    \
   ,BG_LOG_DIFF(n, jc_flush_page_do)

#define latxs_counter_jc_flush(cpu) do {    \
      __latxs_counter_jc_flush(cpu);        \
} while (0)
#define latxs_counter_jc_flush_bg(cpu) do {    \
      __latxs_counter_jc_flush_bg(cpu);        \
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

void __latxs_counter_jc_flush(void *cpu);
void __latxs_counter_jc_flush_bg(void *cpu);
void __latxs_counter_jc_flush_page(void *cpu);
void __latxs_counter_jc_flush_page_go(void *cpu);
void __latxs_counter_jc_flush_page_do(void *cpu);

#else

#define BG_COUNTER_DEF_JC
#define BG_COUNTER_LOG_JC
#define BG_COUNTER_LOG_DATA_JC(n)

#define latxs_counter_jc_flush(cpu)
#define latxs_counter_jc_flush_bg(cpu)
#define latxs_counter_jc_flush_page(cpu)
#define latxs_counter_jc_flush_page_go(cpu)
#define latxs_counter_jc_flush_page_do(cpu)

#endif

/* ======================== HP_LDST ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_HP_LDST)

#define BG_COUNTER_DEF_HP_LDST          \
    uint64_t helper_store_nr;           \
    uint64_t helper_store_io_nr;        \
    uint64_t helper_store_stlbfill_nr;  \
    uint64_t helper_load_nr;            \
    uint64_t helper_load_io_nr;         \
    uint64_t helper_load_stlbfill_nr;   \
    uint64_t helper_store_cpl3_nr;              \
    uint64_t helper_store_stlbfill_cpl3_nr;     \
    uint64_t helper_load_cpl3_nr;               \
    uint64_t helper_load_stlbfill_cpl3_nr;

#define BG_COUNTER_LOG_HP_LDST  \
    "ST %d %d %d LD %d %d %d ST3 %d %d LD3 %d %d "
#define BG_COUNTER_LOG_DATA_HP_LDST(n)      \
   ,BG_LOG_DIFF(n, helper_store)            \
   ,BG_LOG_DIFF(n, helper_store_stlbfill)   \
   ,BG_LOG_DIFF(n, helper_store_io)         \
   ,BG_LOG_DIFF(n, helper_load)             \
   ,BG_LOG_DIFF(n, helper_load_stlbfill)    \
   ,BG_LOG_DIFF(n, helper_load_io)          \
   ,BG_LOG_DIFF(n, helper_store_cpl3)           \
   ,BG_LOG_DIFF(n, helper_store_stlbfill_cpl3)  \
   ,BG_LOG_DIFF(n, helper_load_cpl3)            \
   ,BG_LOG_DIFF(n, helper_load_stlbfill_cpl3)

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

void __latxs_counter_helper_store(void *cpu);
void __latxs_counter_helper_store_io(void *cpu);
void __latxs_counter_helper_store_stlbfill(void *cpu);
void __latxs_counter_helper_load(void *cpu);
void __latxs_counter_helper_load_io(void *cpu);
void __latxs_counter_helper_load_stlbfill(void *cpu);

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

void __latxs_counter_helper_store_cpl3(void *cpu);
void __latxs_counter_helper_store_stlbfill_cpl3(void *cpu);
void __latxs_counter_helper_load_cpl3(void *cpu);
void __latxs_counter_helper_load_stlbfill_cpl3(void *cpu);

#else

#define BG_COUNTER_DEF_HP_LDST
#define BG_COUNTER_LOG_HP_LDST
#define BG_COUNTER_LOG_DATA_HP_LDST(n)

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

#endif

/* ======================== EXEC ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_EXEC)

#define BG_COUNTER_DEF_EXEC     \
    uint64_t exe_write_cr0_nr;  \
    uint64_t exe_write_cr3_nr;  \
    uint64_t exe_invlpg_nr;

#define BG_COUNTER_LOG_EXEC "wCR %d %d invlpg %d "
#define BG_COUNTER_LOG_DATA_EXEC(n) \
   ,BG_LOG_DIFF(n, exe_write_cr0)   \
   ,BG_LOG_DIFF(n, exe_write_cr3)   \
   ,BG_LOG_DIFF(n, exe_invlpg)

#define latxs_counter_exe_write_cr0(cpu) do {   \
      __latxs_counter_exe_write_cr0(cpu);       \
} while (0)
#define latxs_counter_exe_write_cr3(cpu) do {   \
      __latxs_counter_exe_write_cr3(cpu);       \
} while (0)

void __latxs_counter_exe_write_cr0(void *cpu);
void __latxs_counter_exe_write_cr3(void *cpu);

#define latxs_counter_exe_invlpg(cpu) do {  \
      __latxs_counter_exe_invlpg(cpu);      \
} while (0)

void __latxs_counter_exe_invlpg(void *cpu);

#else

#define BG_COUNTER_DEF_EXEC
#define BG_COUNTER_LOG_EXEC
#define BG_COUNTER_LOG_DATA_EXEC(n)

#define latxs_counter_exe_write_cr0(cpu)
#define latxs_counter_exe_write_cr3(cpu)
#define latxs_counter_exe_invlpg(cpu)

#endif

/* ======================== STLB ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_STLB)

#define BG_COUNTER_DEF_STLB         \
    uint64_t stlb_flush_full_nr;    \
    uint64_t stlb_flush_part_nr;    \
    uint64_t stlb_flush_elide_nr;   \
    uint64_t stlb_flush_page_nr;    \
    uint64_t stlb_resize_nr;        \
    uint64_t stlb_resize_inc_nr;    \
    uint64_t stlb_resize_dec_nr;

#define BG_COUNTER_LOG_STLB "sTLB %d %d %d %d RS %d %d %d "
#define BG_COUNTER_LOG_DATA_STLB(n)     \
   ,BG_LOG_DIFF(n, stlb_flush_full)     \
   ,BG_LOG_DIFF(n, stlb_flush_part)     \
   ,BG_LOG_DIFF(n, stlb_flush_elide)    \
   ,BG_LOG_DIFF(n, stlb_flush_page)     \
   ,BG_LOG_DIFF(n, stlb_resize)         \
   ,BG_LOG_DIFF(n, stlb_resize_inc)     \
   ,BG_LOG_DIFF(n, stlb_resize_dec)

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

void __latxs_counter_stlb_flush_full(void *cpu);
void __latxs_counter_stlb_flush_part(void *cpu);
void __latxs_counter_stlb_flush_elide(void *cpu);
void __latxs_counter_stlb_flush_page(void *cpu);
void __latxs_counter_stlb_resize(void *cpu);
void __latxs_counter_stlb_resize_inc(void *cpu);
void __latxs_counter_stlb_resize_dec(void *cpu);

#else

#define BG_COUNTER_DEF_STLB
#define BG_COUNTER_LOG_STLB
#define BG_COUNTER_LOG_DATA_STLB(n)

#define latxs_counter_stlb_flush_full(cpu)
#define latxs_counter_stlb_flush_part(cpu)
#define latxs_counter_stlb_flush_elide(cpu)
#define latxs_counter_stlb_flush_page(cpu)
#define latxs_counter_stlb_resize(cpu)
#define latxs_counter_stlb_resize_inc(cpu)
#define latxs_counter_stlb_resize_dec(cpu)

#endif

/* ======================== HAMT ======================== */
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_HAMT)

#ifdef  __BG_COUNTER_HAMT_P0
#define   BG_COUNTER_DEF_HAMT_P0    \
    uint64_t hamt_ufast_nr;         \
    uint64_t qemu_ufast_nr;
#else
#define   BG_COUNTER_DEF_HAMT_P0
#endif

#ifdef  __BG_COUNTER_HAMT_P1
#define   BG_COUNTER_DEF_HAMT_P1    \
    uint64_t hamt_fast_nr;          \
    uint64_t hamt_fast_badv0_nr;    \
    uint64_t hamt_fast_undef_nr;    \
    uint64_t hamt_fast_ld_nr;       \
    uint64_t hamt_fast_st_nr;       \
    uint64_t hamt_fast_ld_spt_ok_nr;   /* hamt spt */           \
    uint64_t hamt_fast_ld_stlb_ok_nr;  /* hamt stlb */          \
    uint64_t hamt_fast_ld_ok_nr;       /* qemu software tlb */  \
    uint64_t hamt_fast_st_spt_ok_nr;   /* hamt spt */           \
    uint64_t hamt_fast_st_stlb_ok_nr;  /* hamt stlb */          \
    uint64_t hamt_fast_st_ok_nr;       /* qemu software tlb */
#else
#define   BG_COUNTER_DEF_HAMT_P1
#endif

#ifdef  __BG_COUNTER_HAMT_P2
#define   BG_COUNTER_DEF_HAMT_P2    \
    uint64_t hamt_ld_spt_ok_nr;  /* hamt spt  in  load_helper */    \
    uint64_t hamt_ld_stlb_ok_nr; /* hamt stlb in  load_helper */    \
    uint64_t hamt_st_spt_ok_nr;  /* hamt spt  in store_helper */    \
    uint64_t hamt_st_stlb_ok_nr; /* hamt stlb in store_helper */
#else
#define   BG_COUNTER_DEF_HAMT_P2
#endif



#ifdef __BG_COUNTER_HAMT_P0
#define BG_COUNTER_SYNC_HAMT(ds, dd, n) do {    \
    uint64_t *dsptr = (void *)ds[0];            \
    dd[n].hamt_ufast_nr = dsptr[100];           \
    dd[n].qemu_ufast_nr = dsptr[101];           \
} while (0)
#define BG_COUNTER_LOG_HAMT_P0 "hamtUF %d qemuUF %d "
#define BG_COUNTER_LOG_DATA_HAMT_P0(n)  \
   ,BG_LOG_DIFF(n, hamt_ufast)  \
   ,BG_LOG_DIFF(n, qemu_ufast)
#endif

#ifdef __BG_COUNTER_HAMT_P1
#define BG_COUNTER_LOG_HAMT_P1   \
    "hamtF %d %d %d %d %d LD %d %d %d ST %d %d %d "
#define BG_COUNTER_LOG_DATA_HAMT_P1(n)  \
   ,BG_LOG_DIFF(n, hamt_fast)           \
   ,BG_LOG_DIFF(n, hamt_fast_badv0)     \
   ,BG_LOG_DIFF(n, hamt_fast_undef)     \
   ,BG_LOG_DIFF(n, hamt_fast_ld)        \
   ,BG_LOG_DIFF(n, hamt_fast_st)        \
   ,BG_LOG_DIFF(n, hamt_fast_ld_spt_ok)     \
   ,BG_LOG_DIFF(n, hamt_fast_ld_stlb_ok)    \
   ,BG_LOG_DIFF(n, hamt_fast_ld_ok)         \
   ,BG_LOG_DIFF(n, hamt_fast_st_spt_ok)     \
   ,BG_LOG_DIFF(n, hamt_fast_st_stlb_ok)    \
   ,BG_LOG_DIFF(n, hamt_fast_st_ok)
#endif

#ifdef __BG_COUNTER_HAMT_P2
#define BG_COUNTER_LOG_HAMT_P2   \
    "hstlb %d %d hspt %d %d "
#define BG_COUNTER_LOG_DATA_HAMT_P2(n)  \
   ,BG_LOG_DIFF(n, hamt_ld_spt_ok)      \
   ,BG_LOG_DIFF(n, hamt_ld_stlb_ok)     \
   ,BG_LOG_DIFF(n, hamt_st_spt_ok)      \
   ,BG_LOG_DIFF(n, hamt_st_stlb_ok)
#endif

#define BG_COUNTER_DEF_HAMT         \
        BG_COUNTER_DEF_HAMT_P0      \
        BG_COUNTER_DEF_HAMT_P1      \
        BG_COUNTER_DEF_HAMT_P2



#ifdef __BG_COUNTER_HAMT_P1

#define latxs_counter_hamt_fast(cpu) do {    \
      __latxs_counter_hamt_fast(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_badv0(cpu) do {    \
      __latxs_counter_hamt_fast_badv0(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_undef(cpu) do {    \
      __latxs_counter_hamt_fast_undef(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_ld(cpu) do {    \
      __latxs_counter_hamt_fast_ld(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_st(cpu) do {    \
      __latxs_counter_hamt_fast_st(cpu);        \
} while (0)

void __latxs_counter_hamt_fast(void *cpu);
void __latxs_counter_hamt_fast_badv0(void *cpu);
void __latxs_counter_hamt_fast_undef(void *cpu);
void __latxs_counter_hamt_fast_ld(void *cpu);
void __latxs_counter_hamt_fast_st(void *cpu);

#define latxs_counter_hamt_fast_ld_ok(cpu) do { \
      __latxs_counter_hamt_fast_ld_ok(cpu);     \
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
#define latxs_counter_hamt_fast_ld_spt_ok(cpu) do {    \
      __latxs_counter_hamt_fast_ld_spt_ok(cpu);        \
} while (0)
#define latxs_counter_hamt_fast_st_spt_ok(cpu) do {    \
      __latxs_counter_hamt_fast_st_spt_ok(cpu);        \
} while (0)

void __latxs_counter_hamt_fast_ld_ok(void *cpu);
void __latxs_counter_hamt_fast_ld_spt_ok(void *cpu);
void __latxs_counter_hamt_fast_ld_stlb_ok(void *cpu);
void __latxs_counter_hamt_fast_st_ok(void *cpu);
void __latxs_counter_hamt_fast_st_spt_ok(void *cpu);
void __latxs_counter_hamt_fast_st_stlb_ok(void *cpu);

#endif /* __BG_COUNTER_HAMT_P1 */

#ifdef __BG_COUNTER_HAMT_P2

#define latxs_counter_hamt_ld_spt_ok(cpu) do {    \
      __latxs_counter_hamt_ld_spt_ok(cpu);        \
} while (0)
#define latxs_counter_hamt_st_spt_ok(cpu) do {    \
      __latxs_counter_hamt_st_spt_ok(cpu);        \
} while (0)
#define latxs_counter_hamt_ld_stlb_ok(cpu) do {    \
      __latxs_counter_hamt_ld_stlb_ok(cpu);        \
} while (0)
#define latxs_counter_hamt_st_stlb_ok(cpu) do {    \
      __latxs_counter_hamt_st_stlb_ok(cpu);        \
} while (0)

void __latxs_counter_hamt_ld_spt_ok(void *cpu);
void __latxs_counter_hamt_ld_stlb_ok(void *cpu);
void __latxs_counter_hamt_st_spt_ok(void *cpu);
void __latxs_counter_hamt_st_stlb_ok(void *cpu);

#endif /* __BG_COUNTER_HAMT_P2 */

#else
#define BG_COUNTER_DEF_HAMT
#endif /* BG_COUNTER_ENABLE && BG_COUNTER_GROUP_HAMT */



#ifndef __BG_COUNTER_HAMT_P0
#define BG_COUNTER_SYNC_HAMT(ds, dd, n)
#define BG_COUNTER_LOG_HAMT_P0
#define BG_COUNTER_LOG_DATA_HAMT_P0(n)
#endif

#ifndef __BG_COUNTER_HAMT_P1
#define BG_COUNTER_LOG_HAMT_P1
#define BG_COUNTER_LOG_DATA_HAMT_P1(n)
#define latxs_counter_hamt_fast(cpu)
#define latxs_counter_hamt_fast_badv0(cpu)
#define latxs_counter_hamt_fast_undef(cpu)
#define latxs_counter_hamt_fast_ld(cpu)
#define latxs_counter_hamt_fast_ld_ok(cpu)
#define latxs_counter_hamt_fast_st(cpu)
#define latxs_counter_hamt_fast_st_ok(cpu)
#define latxs_counter_hamt_fast_ld_stlb_ok(cpu)
#define latxs_counter_hamt_fast_st_stlb_ok(cpu)
#define latxs_counter_hamt_fast_ld_spt_ok(cpu)
#define latxs_counter_hamt_fast_st_spt_ok(cpu)
#endif

#ifndef __BG_COUNTER_HAMT_P2
#define BG_COUNTER_LOG_HAMT_P2
#define BG_COUNTER_LOG_DATA_HAMT_P2(n)
#define latxs_counter_hamt_ld_spt_ok(cpu)
#define latxs_counter_hamt_st_spt_ok(cpu)
#define latxs_counter_hamt_ld_stlb_ok(cpu)
#define latxs_counter_hamt_st_stlb_ok(cpu)
#endif

/* ======================== INDIRBR ======================== */

#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_INDIRBR)

#define BG_COUNTER_DEF_INDIRBR  \
    uint64_t inbr_cpl0_ret_nr;  \
    uint64_t inbr_cpl0_call_nr; \
    uint64_t inbr_cpl0_jmp_nr;  \
    uint64_t cpl0_inbr_hp_nr;       \
    uint64_t cpl0_inbr_hp_hit_nr;   \
    uint64_t inbr_cpl0_njc_nr;      \
    uint64_t inbr_cpl0_njc_hit_nr;  \
    uint64_t inbr_cpl3_ret_nr;  \
    uint64_t inbr_cpl3_call_nr; \
    uint64_t inbr_cpl3_jmp_nr;  \
    uint64_t cpl3_inbr_hp_nr;       \
    uint64_t cpl3_inbr_hp_hit_nr;   \
    uint64_t inbr_cpl3_njc_nr;      \
    uint64_t inbr_cpl3_njc_hit_nr;  \

#define BG_COUNTER_LOG_INDIRBR  \
    "inbr C0 %d %d %d C3 %d %d %d C0HP %d %d C3HP %d %d "
#define BG_COUNTER_LOG_INDIRBR_NJC  \
    "C0njc %d %d C3njc %d %d "
#define BG_COUNTER_LOG_DATA_INDIRBR(n)  \
   ,BG_LOG_DIFF(n, inbr_cpl0_ret)       \
   ,BG_LOG_DIFF(n, inbr_cpl0_call)      \
   ,BG_LOG_DIFF(n, inbr_cpl0_jmp)       \
   ,BG_LOG_DIFF(n, inbr_cpl3_ret)       \
   ,BG_LOG_DIFF(n, inbr_cpl3_call)      \
   ,BG_LOG_DIFF(n, inbr_cpl3_jmp)       \
   ,BG_LOG_DIFF(n, cpl0_inbr_hp)        \
   ,BG_LOG_DIFF(n, cpl0_inbr_hp_hit)    \
   ,BG_LOG_DIFF(n, cpl3_inbr_hp)        \
   ,BG_LOG_DIFF(n, cpl3_inbr_hp_hit)
#define BG_COUNTER_LOG_DATA_INDIRBR_NJC(n)  \
   ,BG_LOG_DIFF(n, inbr_cpl0_njc)           \
   ,BG_LOG_DIFF(n, inbr_cpl0_njc_hit)       \
   ,BG_LOG_DIFF(n, inbr_cpl3_njc)           \
   ,BG_LOG_DIFF(n, inbr_cpl3_njc_hit)

#define latxs_counter_gen_inbr_cpl0_ret(cpu, t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl0_ret(cpu, t1, t2);        \
} while (0)
#define latxs_counter_gen_inbr_cpl0_call(cpu, t1, t2) do {   \
      __latxs_counter_gen_inbr_cpl0_call(cpu, t1, t2);       \
} while (0)
#define latxs_counter_gen_inbr_cpl0_jmp(cpu, t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl0_jmp(cpu, t1, t2);        \
} while (0)
#define latxs_counter_gen_inbr_cpl3_ret(cpu, t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl3_ret(cpu, t1, t2);        \
} while (0)
#define latxs_counter_gen_inbr_cpl3_call(cpu, t1, t2) do {   \
      __latxs_counter_gen_inbr_cpl3_call(cpu, t1, t2);       \
} while (0)
#define latxs_counter_gen_inbr_cpl3_jmp(cpu, t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl3_jmp(cpu, t1, t2);        \
} while (0)

#define latxs_counter_gen_inbr_cpl0_njc(t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl0_njc(t1, t2);        \
} while (0)
#define latxs_counter_gen_inbr_cpl0_njc_hit(t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl0_njc_hit(t1, t2);        \
} while (0)
#define latxs_counter_gen_inbr_cpl3_njc(t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl3_njc(t1, t2);        \
} while (0)
#define latxs_counter_gen_inbr_cpl3_njc_hit(t1, t2) do {    \
      __latxs_counter_gen_inbr_cpl3_njc_hit(t1, t2);        \
} while (0)

#define latxs_counter_cpl_inbr_hp(cpu) do {    \
      __latxs_counter_cpl_inbr_hp(cpu);        \
} while (0)
#define latxs_counter_cpl_inbr_hp_hit(cpu) do {    \
      __latxs_counter_cpl_inbr_hp_hit(cpu);        \
} while (0)

void __latxs_counter_gen_inbr_cpl0_ret(void *cpu, void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl0_call(void *cpu, void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl0_jmp(void *cpu, void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl3_ret(void *cpu, void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl3_call(void *cpu, void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl3_jmp(void *cpu, void *t1, void *t2);

void __latxs_counter_gen_inbr_cpl0_njc(void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl0_njc_hit(void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl3_njc(void *t1, void *t2);
void __latxs_counter_gen_inbr_cpl3_njc_hit(void *t1, void *t2);

void __latxs_counter_cpl_inbr_hp(void *cpu);
void __latxs_counter_cpl_inbr_hp_hit(void *cpu);

#else

#define latxs_counter_gen_inbr_ret(cpu, tmp)
#define latxs_counter_gen_inbr_call(cpu, tmp)
#define latxs_counter_gen_inbr_jmp(cpu, tmp)

#define latxs_counter_gen_inbr_hp(cpu)
#define latxs_counter_gen_inbr_hp_hit(cpu)

#define latxs_counter_cpl_inbr_hp(cpu)
#define latxs_counter_cpl_inbr_hp_hit(cpu)

#endif

/* ======================== MISC ======================== */

#define BG_COUNTER_DEF_MISC
#define BG_COUNTER_LOG_MISC
#define BG_COUNTER_LOG_DATA_MISC(n)

/* ======================== WAKE ======================== */

#if defined(BG_COUNTER_ENABLE)

#define latxs_counter_wake(cpu) do {    \
      __latxs_counter_wake(cpu);        \
} while (0)
void __latxs_counter_wake(void *cpu);

#else /* disable bg thread counter */

#define latxs_counter_wake(cpu)

#endif /* BG_COUNTER_ENABLE */

#endif
