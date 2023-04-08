#include "common.h"
#include "latx-counter-sys.h"
#include "tcg/tcg-bg-thread.h"
#include "tcg/tcg-bg-log.h"
#include "exec/tb-hash.h"
#include <string.h>

#ifdef BG_COUNTER_ENABLE

#define BUILD_ASSERT(cond)                  \
do {                                        \
    (void)sizeof(char[1 - 2 * !(cond)]);    \
} while (0)

typedef struct {
    uint64_t tb_tr_nr;
    uint64_t tb_inv_nr;
    uint64_t tb_flush_nr;
    /*
     * (1) TB -> helper lookup  -> lookup ------------> TB
     * (2) TB -> context switch -> tb find -> lookup -> TB
     */
    uint64_t tb_lookup_nr;    /* case (2) */
    uint64_t tb_lookup_ht_nr; /* case (2) */

    uint64_t excp_pf_nr;
    uint64_t excp_pf_cpl3_nr;

    uint64_t jc_flush_nr;
    uint64_t jc_flush_page_nr;
    uint64_t jc_flush_page_go_nr;
    uint64_t jc_flush_page_do_nr;

    uint64_t helper_store_nr;
    uint64_t helper_store_io_nr;
    uint64_t helper_store_stlbfill_nr;
    uint64_t helper_load_nr;
    uint64_t helper_load_io_nr;
    uint64_t helper_load_stlbfill_nr;

    uint64_t helper_store_cpl3_nr;
    uint64_t helper_store_stlbfill_cpl3_nr;
    uint64_t helper_load_cpl3_nr;
    uint64_t helper_load_stlbfill_cpl3_nr;
} latxs_counter_t;

latxs_counter_t *latxs_counter_data;

latxs_counter_t __latxs_counter_data[4];
latxs_counter_t __local_latxs_counter_data[4];

static
void __attribute__((__constructor__)) latx_counter_init(void)
{
    memset(&__latxs_counter_data[0], 0, sizeof(latxs_counter_t));
    memset(&__latxs_counter_data[1], 0, sizeof(latxs_counter_t));
    memset(&__latxs_counter_data[2], 0, sizeof(latxs_counter_t));
    memset(&__latxs_counter_data[3], 0, sizeof(latxs_counter_t));

    memset(&__local_latxs_counter_data[0], 0, sizeof(latxs_counter_t));
    memset(&__local_latxs_counter_data[1], 0, sizeof(latxs_counter_t));
    memset(&__local_latxs_counter_data[2], 0, sizeof(latxs_counter_t));
    memset(&__local_latxs_counter_data[3], 0, sizeof(latxs_counter_t));

    latxs_counter_data = __latxs_counter_data;

    BUILD_ASSERT(TB_JC_PAGE_BITS == TB_JMP_PAGE_BITS);
    lsassertm(TB_JC_PAGE_BITS == TB_JMP_PAGE_BITS,
            "TB_JC_PAGE_BITS != TB_JMP_PAGE_BITS " \
            "see hw/core/cpu.h and exec/tb-hash.h");
}

#define COUNTER_OP_INC(id, name) do {       \
__latxs_counter_data[id].name ## _nr += 1;  \
} while (0)


#define IMP_COUNTER_FUNC(name)                  \
void __latxs_counter_ ## name (void *cpu)       \
{                                               \
    int cpuid = 0;                              \
    if (qemu_tcg_mttcg_enabled()) {             \
        cpuid = ((CPUState *)cpu)->cpu_index;   \
    }                                           \
    COUNTER_OP_INC(cpuid, name);                \
}

IMP_COUNTER_FUNC(tb_tr)
IMP_COUNTER_FUNC(tb_inv)
IMP_COUNTER_FUNC(tb_flush)
IMP_COUNTER_FUNC(tb_lookup)
IMP_COUNTER_FUNC(tb_lookup_ht)

IMP_COUNTER_FUNC(excp_pf)
IMP_COUNTER_FUNC(excp_pf_cpl3)

IMP_COUNTER_FUNC(jc_flush)
IMP_COUNTER_FUNC(jc_flush_page)
IMP_COUNTER_FUNC(jc_flush_page_go)
IMP_COUNTER_FUNC(jc_flush_page_do)

IMP_COUNTER_FUNC(helper_store)
IMP_COUNTER_FUNC(helper_store_io)
IMP_COUNTER_FUNC(helper_store_stlbfill)
IMP_COUNTER_FUNC(helper_load)
IMP_COUNTER_FUNC(helper_load_io)
IMP_COUNTER_FUNC(helper_load_stlbfill)

IMP_COUNTER_FUNC(helper_store_cpl3)
IMP_COUNTER_FUNC(helper_store_stlbfill_cpl3)
IMP_COUNTER_FUNC(helper_load_cpl3)
IMP_COUNTER_FUNC(helper_load_stlbfill_cpl3)

#define BG_LOG_DIFF(n, var) \
(__latxs_counter_data[n].var ## _nr - __local_latxs_counter_data[n].var ## _nr)

static void __latxs_counter_bg_log(int n, int sec)
{
    qemu_bglog("[%7d][%1d] "\
            "TR %d "\
            "Lookup %d %d "\
            "ST %d %d %d LD %d %d %d "\
            "ST3 %d %d LD3 %d %d "\
            "JCF %d %d %d %d "\
            "E_PF %d %d " \
            "INV %d\n",
            sec, n,
            BG_LOG_DIFF(n, tb_tr                 ),
            BG_LOG_DIFF(n, tb_lookup             ),
            BG_LOG_DIFF(n, tb_lookup_ht          ),

            BG_LOG_DIFF(n, helper_store          ),
            BG_LOG_DIFF(n, helper_store_stlbfill ),
            BG_LOG_DIFF(n, helper_store_io       ),
            BG_LOG_DIFF(n, helper_load           ),
            BG_LOG_DIFF(n, helper_load_stlbfill  ),
            BG_LOG_DIFF(n, helper_load_io        ),

            BG_LOG_DIFF(n, helper_store_cpl3          ),
            BG_LOG_DIFF(n, helper_store_stlbfill_cpl3 ),
            BG_LOG_DIFF(n, helper_load_cpl3           ),
            BG_LOG_DIFF(n, helper_load_stlbfill_cpl3  ),

            BG_LOG_DIFF(n, jc_flush              ),
            BG_LOG_DIFF(n, jc_flush_page         ),
            BG_LOG_DIFF(n, jc_flush_page_go      ),
            BG_LOG_DIFF(n, jc_flush_page_do      ),

            BG_LOG_DIFF(n, excp_pf               ),
            BG_LOG_DIFF(n, excp_pf_cpl3          ),

            BG_LOG_DIFF(n, tb_inv                )
            );
}

#define BG_COUNTER_COPY(n) do {             \
memcpy(&__latxs_counter_data[n],            \
       &__local_latxs_counter_data[n],      \
       sizeof(latxs_counter_t));            \
} while (0)

/* worker function */
static void latxs_counter_bg_log(int sec)
{
    if (qemu_tcg_mttcg_enabled()) {
        __latxs_counter_bg_log(0, sec);
        __latxs_counter_bg_log(1, sec);
        __latxs_counter_bg_log(2, sec);
        __latxs_counter_bg_log(3, sec);
        qemu_bglog("\n");
    } else {
        __latxs_counter_bg_log(0, sec);
    }

    qemu_bglog_flush();

    if (qemu_tcg_mttcg_enabled()) {
        BG_COUNTER_COPY(0);
        BG_COUNTER_COPY(1);
        BG_COUNTER_COPY(2);
        BG_COUNTER_COPY(3);
    } else {
        BG_COUNTER_COPY(0);
    }
}

void __latxs_counter_wake(void *cpu)
{
    static int64_t st = 0, cur = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    cur = ts.tv_sec;
    if (cur != st) {
        st = cur;
        /* construct worker and wake up bg thread */
        tcg_bg_counter_wake(latxs_counter_bg_log, st);
    }
}

#endif
