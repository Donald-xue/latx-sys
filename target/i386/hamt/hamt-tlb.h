#ifndef _HAMT_TLB_H_
#define _HAMT_TLB_H_

void hamt_set_hardware_tlb(void *env,
        uint32_t vaddr, uint64_t paddr,
        int prot, int istlbr);
void __hamt_set_hardware_tlb(void *env,
        uint32_t vaddr, uint64_t paddr,
        int prot, int is_tlbr);

void hamt_protect_code(uint64_t guest_pc, int is_page2);
void hamt_unprotect_code(uint64_t guest_pc);

void hamt_set_tlb(void *env,
        uint64_t vaddr, uint64_t paddr,
        int prot, bool mode);
void hamt_flush_tlb_all(void);

void hamt_set_tlb_simple(uint64_t ehi,
        uint64_t elo0, uint64_t elo1,
        int is_tlbr);

#endif
