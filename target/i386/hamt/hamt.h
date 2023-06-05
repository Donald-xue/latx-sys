#ifndef __HAMT_H
#define __HAMT_H
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#define HAMT_MODE_RR    1
#define HAMT_MODE_MT    2

int dune_enter(int mode, int cpuid);
#define hamt_enter(mode, cpuid) (dune_enter(mode, cpuid))

int hamt_enable(void);

int hamt_base(void);
int hamt_interpreter(void);
int hamt_pg_asid(void);
int hamt_softmmu(void);

int hamt_have_tlbr_fastpath(void);
int hamt_have_tlbr_ultra_fastpath(void);
int hamt_have_stlb(void);
int hamt_have_spt(void);

extern pthread_key_t in_hamt;
bool hamt_started(void);
void start_hamt(bool *enable);
void stop_hamt(bool *disable);

void hamt_local_flush_page(uint32_t i386_addr);

void hamt_flush_all(void);

void hamt_alloc_target_addr_space(void);

void delete_pgtable(uint64_t cr3);

void hamt_set_context(uint64_t new_cr3);
 
void hamt_need_flush(uint64_t old_cr3, bool del_pgtable);

extern uint64_t from_tlb_flush;
extern uint64_t from_tlb_flush_page_locked;
extern uint64_t from_by_mmuidx;

int hamt_fast_load(void *env, uint64_t addr);
int hamt_fast_store(void *env, uint64_t addr);

#endif
