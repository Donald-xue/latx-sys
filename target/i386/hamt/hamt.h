#ifndef __HAMT_H
#define __HAMT_H
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

int dune_enter(void);
#define hamt_enter() (dune_enter())

extern bool hamt_enabled;
#define hamt_enable() (hamt_enabled) 

extern pthread_key_t in_hamt;
bool hamt_started(void);
void start_hamt(bool *enable);
void stop_hamt(bool *disable);

void hamt_invlpg_helper(uint32_t i386_addr);

void hamt_flush_all(void);

void alloc_target_addr_space(void);

void delete_pgtable(uint64_t cr3);

void hamt_set_context(uint64_t new_cr3);
 
void hamt_need_flush(void);

extern uint64_t from_tlb_flush;
extern uint64_t from_tlb_flush_page_locked;
extern uint64_t from_by_mmuidx;

#endif
