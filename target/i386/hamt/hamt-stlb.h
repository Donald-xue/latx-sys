#ifndef _HAMT_STLB_H_
#define _HAMT_STLB_H_

#define HAMT_USE_STLB



#ifdef HAMT_USE_STLB

typedef struct hamt_stlb_entry {
    uint64_t page;
    uint64_t ehi;
    uint64_t elo[2];
} hamt_stlb_entry;

void hamt_stlb_set_page(CPUArchState *env,
        uint64_t vaddr, uint64_t paddr, int prot,
        uint64_t ehi, uint64_t elo0, uint64_t elo1);

void hamt_stlb_flush_page(CPUArchState *env,
        int midx, uint64_t page);

void hamt_stlb_apply(hamt_stlb_entry *stlb, int is_tlbr);

#endif

#endif
