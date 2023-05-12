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
#include "hamt-spt.h"

#include "monitor/monitor.h"
#include "monitor/hmp-target.h"
#include "monitor/hmp.h"
#include "qapi/qmp/qdict.h"

#include "latx-options.h"
#include "latx-sigint-fn-sys.h"

#include "latx-counter-sys.h"

#ifdef HAMT_USE_SPT

void hamt_spt_set_page(CPUArchState *env,
        uint64_t vaddr, uint64_t paddr, int prot,
        uint64_t ehi, uint64_t elo0, uint64_t elo1)
{
    int mmu_idx = cpu_mmu_index(env, false);

    CPUTLB *tlb = env_tlb(env);
    CPUTLBDescFast *fast = &tlb->f[mmu_idx];

    int n  = (vaddr >> 12) & 0x1;
    uint64_t elon = n ? elo1 : elo0;

    int spt1_idx = (vaddr >> 22) & 0x3ff;
    int spt2_idx = (vaddr >> 12) & 0x3ff;

    hamt_spt1 *spt1;
    hamt_spt2 *spt2;

    spt1 = fast->spt;

    if (spt1->spt2[spt1_idx]) {
        spt2 = spt1->spt2[spt1_idx];
    } else {
        spt2 = g_new(hamt_spt2, 1);
        memset(spt2, 0, sizeof(hamt_spt2));
        spt1->spt2[spt1_idx] = spt2;
    }

    spt2->entry[spt2_idx].elo = elon;
}

void hamt_spt_flush_page(CPUArchState *env,
        int mmu_idx, uint64_t vaddr)
{
    CPUTLB *tlb = env_tlb(env);
    CPUTLBDescFast *fast = &tlb->f[mmu_idx];

    int spt1_idx = (vaddr >> 22) & 0x3ff;
    int spt2_idx = (vaddr >> 12) & 0x3ff;

    hamt_spt1 *spt1 = fast->spt;
    hamt_spt2 *spt2 = spt1->spt2[spt1_idx];
    if (spt2 != NULL) {
        spt2->entry[spt2_idx].elo = 0;
    }
}

int hamt_spt_try_apply(CPUArchState *env, uint64_t vaddr, int mode)
{
    int mmu_idx = cpu_mmu_index(env, false);

    CPUTLB *tlb = env_tlb(env);
    CPUTLBDescFast *fast = &tlb->f[mmu_idx];

    int spt1_idx  = (vaddr >> 22) & 0x3ff;
    int spt2_idx  = (vaddr >> 12) & 0x3ff;
    int spt2_idx2 = spt2_idx ^ 0x1;

    int n  = (vaddr >> 12) & 0x1;
    int n2 = n ^ 0x1;

    uint64_t elo[2] = {0, 0};

    hamt_spt1 *spt1 = fast->spt;
    hamt_spt2 *spt2 = spt1->spt2[spt1_idx];
    if (spt2 == NULL) {
        return 0;
    }

    elo[n]  = spt2->entry[spt2_idx].elo;
    elo[n2] = spt2->entry[spt2_idx2].elo;

    uint64_t ehi = vaddr & ~0x1fffULL;

    /* tlbr from tlbrefill fast path */
    if (mode == 1 && elo[n] != 0) {
        hamt_set_tlb_simple(ehi, elo[0], elo[1], 1);
        return 1;
    }

    /* tlb invalid from load/stpre helper */
    if (mode == 2 && elo[0] != 0 && elo[1] != 0) {
        hamt_set_tlb_simple(ehi, elo[0], elo[1], 0);
        return 2;
    }

    return 0;
}

void hamt_spt_free2(hamt_spt1 *spt)
{
    if (!spt) return;

    int i = 0;
    for (; i < HAMT_SPT1_NUM; ++i) {
        if (spt->spt2[i]) g_free(spt->spt2[i]);
    }

    memset(spt, 0, sizeof(hamt_spt1));
}

#endif
