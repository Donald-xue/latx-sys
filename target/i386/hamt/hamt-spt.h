#ifndef _HAMT_SPT_H_
#define _HAMT_SPT_H_

#define HAMT_USE_SPT



#ifdef HAMT_USE_SPT

/*
 * 16 KB / 8 B = 2K = 2048 = 2^11
 *
 * @1 - 10 - 31 ... 22
 * @2 - 10 - 21 ... 12
 * @3 - entry
 */

#define HAMT_SPT1_BITS  10
#define HAMT_SPT2_BITS  10

#define HAMT_SPT1_NUM   (1<<HAMT_SPT1_BITS)
#define HAMT_SPT2_NUM   (1<<HAMT_SPT2_BITS)

typedef struct hamt_spt_entry {
    uint64_t elo;
} hamt_spt_entry;

typedef struct hamt_spt2 {
    hamt_spt_entry entry[HAMT_SPT2_NUM];
} hamt_spt2;

typedef struct hamt_spt1 {
    hamt_spt2 *spt2[HAMT_SPT1_NUM];
} hamt_spt1;

void hamt_spt_set_page(CPUArchState *env, uint64_t vaddr,
        uint64_t ehi, uint64_t elo0, uint64_t elo1);

void hamt_spt_flush_page(CPUArchState *env,
        int midx, uint64_t page);

void hamt_spt_free2(hamt_spt1 *spt);

int hamt_spt_try_apply(CPUArchState *env, uint64_t vaddr, int mode);

#endif

#endif
