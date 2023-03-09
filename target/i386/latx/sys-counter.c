#include "common.h"
#include "latx-counter-sys.h"
#include "tcg/tcg-bg-thread.h"
#include "tcg/tcg-bg-log.h"
#include <string.h>

#define BG_COUNTER_ENABLE

typedef struct {
    uint64_t tb_tr_nr;
    uint64_t tb_inv_nr;
    uint64_t tb_flush_nr;
    uint64_t tb_lookup_nr;

    uint64_t jc_flush_nr;
    uint64_t jc_flush_page_nr;

    uint64_t helper_store_nr;
    uint64_t helper_store_io_nr;
    uint64_t helper_store_stlbfill_nr;
    uint64_t helper_load_nr;
    uint64_t helper_load_io_nr;
    uint64_t helper_load_stlbfill_nr;
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
}

#ifdef BG_COUNTER_ENABLE

#define COUNTER_OP_INC(id, name) do {       \
__latxs_counter_data[id].name ## _nr += 1;  \
} while (0)


#define IMP_COUNTER_FUNC(name)                  \
void latxs_counter_ ## name (void *cpu)         \
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

IMP_COUNTER_FUNC(jc_flush)
IMP_COUNTER_FUNC(jc_flush_page)

IMP_COUNTER_FUNC(helper_store)
IMP_COUNTER_FUNC(helper_store_io)
IMP_COUNTER_FUNC(helper_store_stlbfill)
IMP_COUNTER_FUNC(helper_load)
IMP_COUNTER_FUNC(helper_load_io)
IMP_COUNTER_FUNC(helper_load_stlbfill)

#define BG_LOG_DIFF(n, var) \
(__latxs_counter_data[n].var ## _nr - __local_latxs_counter_data[n].var ## _nr)

static void __latxs_counter_bg_log(int n, int sec)
{
    qemu_bglog("[%7d][%1d] TR %-6d Lookup %-6d "\
            "ST[%-6d/%-6d/%-6d] LD[%-6d/%-6d/%-6d] "\
            "JCF[%-6d/%-6d] INV %-6d\n",
            sec, n,
            BG_LOG_DIFF(n, tb_tr                 ),
            BG_LOG_DIFF(n, tb_lookup             ),

            BG_LOG_DIFF(n, helper_store          ),
            BG_LOG_DIFF(n, helper_store_stlbfill ),
            BG_LOG_DIFF(n, helper_store_io       ),
            BG_LOG_DIFF(n, helper_load           ),
            BG_LOG_DIFF(n, helper_load_stlbfill  ),
            BG_LOG_DIFF(n, helper_load_io        ),

            BG_LOG_DIFF(n, jc_flush              ),
            BG_LOG_DIFF(n, jc_flush_page         ),
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

void latxs_counter_wake(void *cpu)
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

#else

#define IMP_COUNTER_FUNC(name) \
void latxs_counter_ ## name (void *cpu) {}

IMP_COUNTER_FUNC(tb_tr)
IMP_COUNTER_FUNC(tb_inv)
IMP_COUNTER_FUNC(tb_flush)
IMP_COUNTER_FUNC(tb_lookup)

IMP_COUNTER_FUNC(jc_flush)
IMP_COUNTER_FUNC(jc_flush_page)

IMP_COUNTER_FUNC(helper_store)
IMP_COUNTER_FUNC(helper_store_io)
IMP_COUNTER_FUNC(helper_store_stlbfill)
IMP_COUNTER_FUNC(helper_load)
IMP_COUNTER_FUNC(helper_load_io)
IMP_COUNTER_FUNC(helper_load_stlbfill)

void latxs_counter_wake(void *cpu) {}

#endif
