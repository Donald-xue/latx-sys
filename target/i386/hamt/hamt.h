#ifndef __HAMT_H
#define __HAMT_H
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

int dune_enter(void);
#define hamt_enter() (dune_enter())

int hamt_enable(void);

int hamt_base(void);
int hamt_interpreter(void);
int hamt_pg_asid(void);
int hamt_softmmu(void);

extern pthread_key_t in_hamt;
bool hamt_started(void);
void start_hamt(bool *enable);
void stop_hamt(bool *disable);

void hamt_invlpg_helper(uint32_t i386_addr);

void hamt_flush_all(void);

void hamt_alloc_target_addr_space(void);

void delete_pgtable(uint64_t cr3);

void hamt_set_context(uint64_t new_cr3);
 
void hamt_need_flush(uint64_t old_cr3, bool del_pgtable);

extern uint64_t from_tlb_flush;
extern uint64_t from_tlb_flush_page_locked;
extern uint64_t from_by_mmuidx;

void hamt_cpu_restore_state_from_tb(void *cpu);
void hamt_protect_code(uint64_t guest_pc, int is_page2);
void hamt_unprotect_code(uint64_t guest_pc);
void hamt_set_hardware_tlb(uint32_t vaddr, uint64_t paddr, int prot);

#endif
