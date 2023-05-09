#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include <pthread.h>

#include "qemu/osdep.h"
#include "qemu/compiler.h"
#include "qemu/host-utils.h"
#include "qemu/typedefs.h"
#include "exec/cpu-defs.h"
#include "cpu.h"
#include "exec/cpu-all.h"
#include "exec/exec-all.h"
#include "qemu/xxhash.h"
#include "exec/ram_addr.h"

#include "svm.h"
#include "tcg/helper-tcg.h"

#include "hamt.h"
#include "hamt_misc.h"
#include "internal.h"
#include "info.h"
#include "hamt-tlb.h"
#include "hamt-stlb.h"

#include "monitor/monitor.h"
#include "monitor/hmp-target.h"
#include "monitor/hmp.h"
#include "qapi/qmp/qdict.h"

#include "latx-options.h"
#include "latx-sigint-fn-sys.h"

#include "latx-counter-sys.h"

#define STLB_PAGE_INVALID(p)     (p >> 32)
#define STLB_PAGE_VALID(p)       !(STLB_PAGE_INVALID(p))

void hamt_stlb_set_page(CPUArchState *env,
        uint64_t vaddr, uint64_t paddr, int prot,
        uint64_t ehi, uint64_t elo0, uint64_t elo1)
{
    int mmu_idx = cpu_mmu_index(env, false);

    CPUTLB *tlb = env_tlb(env);
    CPUTLBDescFast *fast = &tlb->f[mmu_idx];

    uint64_t size_mask = fast->mask >> CPU_TLB_ENTRY_BITS;
    int index0 = ((vaddr >> 12) & size_mask) & ~0x1;

    hamt_stlb_entry *stlb = fast->stlb;
    hamt_stlb_entry *ste0 = &stlb[index0];

    int n  = (vaddr >> 12) & 0x1;
    int n2 = ~n & 0x1;
    assert((n==0&&n2==1) || (n==1&&n2==0));
    uint64_t page = vaddr & ~0x1fffULL;
    uint64_t elon = n ? elo1 : elo0;

    ste0[n].page = page;
    ste0[n].ehi  = ehi;
    ste0[n].elo[n]  = elon;

    if (STLB_PAGE_INVALID(ste0[n2].page)) {
        ste0[n].elo[n2] = 0;

        ste0[n2].page = page;
        ste0[n2].ehi  = ehi;
        ste0[n2].elo[n]  = elon;
        ste0[n2].elo[n2] = 0;
    } else {
        if (ste0[n2].page == page) {
            ste0[n].elo[n2] = ste0[n2].elo[n2];
            ste0[n2].elo[n] = elon;
        } else {
            ste0[n].elo[n2] = 0;
            ste0[n2].elo[n] = 0;
        }
    }
}

void hamt_stlb_flush_page(CPUArchState *env,
        int mmu_idx, uint64_t vaddr)
{
    CPUTLB *tlb = env_tlb(env);
    CPUTLBDescFast *fast = &tlb->f[mmu_idx];

    uint64_t size_mask = fast->mask >> CPU_TLB_ENTRY_BITS;
    int index0 = ((vaddr >> 12) & size_mask) & ~0x1;

    hamt_stlb_entry *stlb = fast->stlb;
    hamt_stlb_entry *ste0 = &stlb[index0];

    int n  = (vaddr >> 12) & 0x1;
    int n2 = ~n & 0x1;
    uint64_t page = vaddr & ~0x1fffULL;

    /*ste0[n].page = ~0;*/
    if (STLB_PAGE_VALID(ste0[n].page)) {
        if (ste0[n].page == page) {
            if (ste0[n].elo[n2] == 0) {
                ste0[n].page = ~0;
            } else {
                ste0[n].elo[n] = 0;
            }
        }
    }

    /*if (ste0[n2].page == page) {*/
        /*ste0[n2].page = ~0;*/
    /*}*/
    if (STLB_PAGE_VALID(ste0[n2].page)) {
        if (ste0[n2].page == page) {
            if (ste0[n2].elo[n2] == 0) {
                ste0[n2].page = ~0;
            } else {
                ste0[n2].elo[n] = 0;
            }
        }
    }
}

void hamt_stlb_apply(hamt_stlb_entry *stlb, int is_tlbr)
{
    uint64_t ehi = stlb->ehi;
    uint64_t elo0 = stlb->elo[0];
    uint64_t elo1 = stlb->elo[1];

    hamt_set_tlb_simple(ehi, elo0, elo1, is_tlbr);
}
