#include "common.h"
#include "latx-counter-sys.h"
#include "tcg/tcg-bg-thread.h"
#include "tcg/tcg-bg-log.h"
#include <string.h>

/*#define BG_COUNTER_ENABLE*/

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

#define IMP_COUNTER_FUNC(name, var)         \
void latxs_counter_ ## name (void *cpu)     \
{                                           \
    __latxs_counter_data[0].var += 1;       \
}

IMP_COUNTER_FUNC(tb_tr,                 tb_tr_nr)
IMP_COUNTER_FUNC(tb_inv,                tb_inv_nr)
IMP_COUNTER_FUNC(tb_flush,              tb_flush_nr)
IMP_COUNTER_FUNC(tb_lookup,             tb_lookup_nr)

IMP_COUNTER_FUNC(jc_flush,              jc_flush_nr)
IMP_COUNTER_FUNC(jc_flush_page,         jc_flush_page_nr)

IMP_COUNTER_FUNC(helper_store,          helper_store_nr)
IMP_COUNTER_FUNC(helper_store_io,       helper_store_io_nr)
IMP_COUNTER_FUNC(helper_store_stlbfill, helper_store_stlbfill_nr)
IMP_COUNTER_FUNC(helper_load,           helper_load_nr)
IMP_COUNTER_FUNC(helper_load_io,        helper_load_io_nr)
IMP_COUNTER_FUNC(helper_load_stlbfill,  helper_load_stlbfill_nr)

#define BG_LOG_DIFF(n, var) \
(__latxs_counter_data[n].var - __local_latxs_counter_data[n].var)

/* worker function */
void latxs_counter_bg_log(int sec)
{
    qemu_bglog("[%7d] TR %-6d Lookup %-6d "\
            "ST[%-6d/%-6d/%-6d] LD[%-6d/%-6d/%-6d] "\
            "JCF[%-6d/%-6d] INV %-6d\n",
            sec,
            BG_LOG_DIFF(0, tb_tr_nr                 ),
            BG_LOG_DIFF(0, tb_lookup_nr             ),

            BG_LOG_DIFF(0, helper_store_nr          ),
            BG_LOG_DIFF(0, helper_store_stlbfill_nr ),
            BG_LOG_DIFF(0, helper_store_io_nr       ),
            BG_LOG_DIFF(0, helper_load_nr           ),
            BG_LOG_DIFF(0, helper_load_stlbfill_nr  ),
            BG_LOG_DIFF(0, helper_load_io_nr        ),

            BG_LOG_DIFF(0, jc_flush_nr              ),
            BG_LOG_DIFF(0, jc_flush_page_nr         ),
            BG_LOG_DIFF(0, tb_inv_nr                )
            );
    qemu_bglog_flush();
    memcpy(&__latxs_counter_data[0], &__local_latxs_counter_data[0], sizeof(latxs_counter_t));
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
        tcg_bg_counter_wake(st);
    }
}

#else

#define IMP_COUNTER_FUNC(name, var)         \
void latxs_counter_ ## name (void *cpu) {}

IMP_COUNTER_FUNC(tb_tr,                 tb_tr_nr)
IMP_COUNTER_FUNC(tb_inv,                tb_inv_nr)
IMP_COUNTER_FUNC(tb_flush,              tb_flush_nr)
IMP_COUNTER_FUNC(tb_lookup,             tb_lookup_nr)
IMP_COUNTER_FUNC(jc_flush,              jc_flush_nr)
IMP_COUNTER_FUNC(jc_flush_page,         jc_flush_page_nr)
IMP_COUNTER_FUNC(helper_store,          helper_store_nr)
IMP_COUNTER_FUNC(helper_store_io,       helper_store_io_nr)
IMP_COUNTER_FUNC(helper_store_stlbfill, helper_store_stlbfill_nr)
IMP_COUNTER_FUNC(helper_load,           helper_load_nr)
IMP_COUNTER_FUNC(helper_load_io,        helper_load_io_nr)
IMP_COUNTER_FUNC(helper_load_stlbfill,  helper_load_stlbfill_nr)

void latxs_counter_bg_log(int sec) {}
void latxs_counter_wake(void *cpu) {}

#endif
