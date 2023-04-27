#include "qemu/osdep.h"
#include "qemu-common.h"
#include "sysemu/tcg.h"
#include "sysemu/replay.h"
#include "qemu/main-loop.h"
#include "qemu/guest-random.h"
#include "exec/exec-all.h"

#include "tcg-accel-ops.h"
#include "tcg-accel-ops-rr.h"
#include "tcg-accel-ops-icount.h"
#include "tcg/tcg-bg-thread.h"

#include "tcg/tcg-bg-log.h"
#include "tcg/tcg-bg-jc.h"
#include "tcg/tcg-bg-tlb.h"
#include "latx-counter-sys.h"

#define BG_TLB_DEBUG

/* ================== BG thread CPU Soft TLB ======================= */

#ifdef BG_TLB_ENABLE

#define TCG_BG_TLB_MAX  64
#define TCG_BG_TLB_MASK (0x3F)

#define TCG_BG_TLB_SIZE_MIN 6
#define TCG_BG_TLB_SIZE_MAX 18

#define TCG_BG_TLB_SIZE_N   TCG_BG_TLB_SIZE_MAX
#define TCG_BG_TLB_SIZE (1<<TCG_BG_TLB_SIZE_N)

static int tcg_bg_tlb_initial_size;
static int tcg_bg_tlb_initial_size_n;
static int tcg_bg_tlb_size_n[TCG_BG_TLB_MAX];

void tcg_bg_tlb_init_size(int s)
{
#ifdef BG_TLB_DEBUG
    printf("BG TLB %s init SIZE %d\n",
            __func__, s);
#endif
    assert(TCG_BG_TLB_SIZE_MIN <= s &&
           TCG_BG_TLB_SIZE_MAX >= s);
    tcg_bg_tlb_initial_size = 1 << s;
    tcg_bg_tlb_initial_size_n = s;

    int i = 0;
    for (; i < TCG_BG_TLB_MAX; ++i) {
        tcg_bg_tlb_size_n[i] = tcg_bg_tlb_initial_size_n;
    }
}

CPUTLBEntry tcg_bg_tlb[TCG_BG_TLB_MAX][TCG_BG_TLB_SIZE];

static QemuMutex  *tcg_bg_tlb_mutex;
static int tcg_bg_tlb_free_ids[TCG_BG_TLB_MAX];
static int tcg_bg_tlb_free_head;
static int tcg_bg_tlb_free_tail;
static int tcg_bg_tlb_free_num;

#define HEAD_INC() do {                         \
    tcg_bg_tlb_free_head += 1;                  \
    tcg_bg_tlb_free_head &= TCG_BG_TLB_MASK;    \
} while (0)
#define TAIL_INC() do {                         \
    tcg_bg_tlb_free_tail += 1;                  \
    tcg_bg_tlb_free_tail &= TCG_BG_TLB_MASK;    \
} while (0)

static int tcg_bg_tlb_get_freeid_locked(void)
{
    int free_id = -1;
    int free_head = tcg_bg_tlb_free_head;
    int free_tail = tcg_bg_tlb_free_tail;
    int *free_ids = tcg_bg_tlb_free_ids;

    if (free_head != free_tail)  {
        assert(tcg_bg_tlb_free_num > 0);
        free_id = free_ids[free_head];
        free_ids[free_head] = -1;
        assert(free_id >= 0);

        HEAD_INC();
        tcg_bg_tlb_free_num -= 1;
    } else {
        assert(tcg_bg_tlb_free_num == 0);
    }

    return free_id;
}

static void tcg_bg_tlb_add_id_locked(int tlb_id)
{
    assert(tcg_bg_tlb_free_num < (TCG_BG_TLB_MAX - 1));
    if (tcg_bg_tlb_free_num == 0) {
        assert(tcg_bg_tlb_free_head == tcg_bg_tlb_free_tail);
    }

    tcg_bg_tlb_free_ids[tcg_bg_tlb_free_tail] = tlb_id;

    TAIL_INC();
    tcg_bg_tlb_free_num += 1;
}



static void tcg_bg_worker_tlb_flush(int tlb_id)
{
    int s = tcg_bg_tlb_size_n[tlb_id];
    memset(tcg_bg_tlb[tlb_id], -1, sizeof(CPUTLBEntry) * (1 << s));

    qemu_mutex_lock(tcg_bg_tlb_mutex);
    tcg_bg_tlb_add_id_locked(tlb_id);
    qemu_mutex_unlock(tcg_bg_tlb_mutex);
}



void tcg_bg_tlb_flush(CPUTLBDesc *desc, CPUTLBDescFast *fast)
{
    qemu_mutex_lock(tcg_bg_tlb_mutex);
    int free_id = tcg_bg_tlb_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_tlb_mutex);

    if (free_id >= 0) {
        int old_id = desc->bg_tlb_id;

        desc->bg_tlb_id = free_id;
        fast->table = tcg_bg_tlb[free_id];

        tcg_bg_tlb_wake(tcg_bg_worker_tlb_flush, old_id);
    } else {
        int s = tcg_bg_tlb_size_n[desc->bg_tlb_id];
        memset(fast->table, -1, sizeof(CPUTLBEntry) * (1 << s));
    }

    desc->n_used_entries = 0;
    desc->large_page_addr = -1;
    desc->large_page_mask = -1;
    desc->vindex = 0;
    memset(desc->vtable, -1, sizeof(desc->vtable));
}

void tcg_bg_tlb_init(CPUTLBDesc *desc, CPUTLBDescFast *fast)
{
    qemu_mutex_lock(tcg_bg_tlb_mutex);
    int free_id = tcg_bg_tlb_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_tlb_mutex);

    int s = tcg_bg_tlb_size_n[free_id];

#ifdef BG_TLB_DEBUG
    printf("BG TLB %s ID %d SIZE %d\n",
            __func__, free_id, s);
#endif

    desc->n_used_entries = 0;
    fast->mask = ((1 << s) - 1) << CPU_TLB_ENTRY_BITS;
    fast->table = tcg_bg_tlb[free_id];
    desc->iotlb = g_new(CPUIOTLBEntry, TCG_BG_TLB_SIZE);

    desc->bg_tlb_id = free_id;

    /* flush */
    desc->n_used_entries = 0;
    desc->large_page_addr = -1;
    desc->large_page_mask = -1;
    desc->vindex = 0;
    memset(fast->table, -1, sizeof(CPUTLBEntry) * (1 << s));
    memset(desc->vtable, -1, sizeof(desc->vtable));
}

void tcg_bg_tlb_init_static(void)
{
    tcg_bg_tlb_initial_size = -1;
    tcg_bg_tlb_initial_size_n = -1;

    int i = 0;
    for (; i < TCG_BG_TLB_MAX; ++i) {
        memset(tcg_bg_tlb[i], -1, sizeof(CPUTLBEntry) * TCG_BG_TLB_SIZE);
        tcg_bg_tlb_free_ids[i] = i;
        tcg_bg_tlb_size_n[i] = TCG_BG_TLB_SIZE_N;
    }
    tcg_bg_tlb_free_ids[i - 1] = -1;

    tcg_bg_tlb_free_head = 0;
    tcg_bg_tlb_free_tail = TCG_BG_TLB_MAX - 1;
    tcg_bg_tlb_free_num  = TCG_BG_TLB_MAX - 1;

    tcg_bg_tlb_mutex = g_malloc0(sizeof(QemuMutex));
    qemu_mutex_init(tcg_bg_tlb_mutex);
}

#endif
