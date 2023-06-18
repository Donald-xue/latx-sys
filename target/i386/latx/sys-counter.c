#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
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
    BG_COUNTER_DEF_TB
    BG_COUNTER_DEF_EXCP
    BG_COUNTER_DEF_INT
    BG_COUNTER_DEF_JC
    BG_COUNTER_DEF_HP_LDST
    BG_COUNTER_DEF_EXEC
    BG_COUNTER_DEF_STLB
    BG_COUNTER_DEF_HAMT
    BG_COUNTER_DEF_INDIRBR
    BG_COUNTER_DEF_DUMMY
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

#define COUNTER_OP_GEN_INC(id, name, tmp1, tmp2) do {       \
    IR2_OPND *t1 = (IR2_OPND *)tmp1;                        \
    IR2_OPND *t2 = (IR2_OPND *)tmp2;                        \
    uint64_t addr = (uint64_t)(&__latxs_counter_data[id].name ## _nr);  \
    latxs_load_imm64(t1, addr);                             \
    latxs_append_ir2_opnd2i(LISA_LD_D,   t2, t1, 0); \
    latxs_append_ir2_opnd2i(LISA_ADDI_D, t2, t2, 1); \
    latxs_append_ir2_opnd2i(LISA_ST_D,   t2, t1, 0); \
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

#define IMP_COUNTER_CPL_FUNC(name)              \
void __latxs_counter_cpl_ ## name (void *_cpu)  \
{                                               \
    CPUState *cpu = _cpu;                       \
    CPUX86State *env = cpu->env_ptr;            \
    int cpuid = 0;                              \
    if (qemu_tcg_mttcg_enabled()) {             \
        cpuid = cpu->cpu_index;                 \
    }                                           \
    int cpl = env->hflags & 0x3;                \
    if (cpl == 0) {                             \
        COUNTER_OP_INC(cpuid, cpl0_ ## name);   \
    }                                           \
    if (cpl == 3) {                             \
        COUNTER_OP_INC(cpuid, cpl3_ ## name);   \
    }                                           \
}

#define IMP_COUNTER_GEN_FUNC(name)                  \
void __latxs_counter_gen_ ## name (void *cpu,       \
        void *tmp1, void *tmp2)                     \
{                                                   \
    int cpuid = 0;                                  \
    if (qemu_tcg_mttcg_enabled()) {                 \
        cpuid = ((CPUState *)cpu)->cpu_index;       \
    }                                               \
    COUNTER_OP_GEN_INC(cpuid, name, tmp1, tmp2);    \
}

#define IMP_COUNTER_STATIC_GEN_FUNC(name)           \
void __latxs_counter_gen_ ## name (                 \
        void *tmp1, void *tmp2)                     \
{                                                   \
    COUNTER_OP_GEN_INC(0, name, tmp1, tmp2);        \
}

#ifdef BG_COUNTER_GROUP_TB
IMP_COUNTER_FUNC(tb_tr)
IMP_COUNTER_FUNC(tb_inv)
IMP_COUNTER_FUNC(tb_flush)
IMP_COUNTER_FUNC(tb_lookup)
IMP_COUNTER_FUNC(tb_lookup_ht)
IMP_COUNTER_FUNC(tb_lookup_cpl3)
IMP_COUNTER_FUNC(tb_lookup_ht_cpl3)
#endif

#ifdef BG_COUNTER_GROUP_EXCP
IMP_COUNTER_FUNC(excp_pf)
IMP_COUNTER_FUNC(excp_pf_cpl3)
#endif

#ifdef BG_COUNTER_GROUP_INT
IMP_COUNTER_FUNC(interrupt)
#endif

#ifdef BG_COUNTER_GROUP_JC
IMP_COUNTER_FUNC(jc_flush)
IMP_COUNTER_FUNC(jc_flush_bg)
IMP_COUNTER_FUNC(jc_flush_page)
IMP_COUNTER_FUNC(jc_flush_page_go)
IMP_COUNTER_FUNC(jc_flush_page_do)
#endif

#ifdef BG_COUNTER_GROUP_HP_LDST
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
#endif

#ifdef BG_COUNTER_GROUP_EXEC
IMP_COUNTER_FUNC(exe_write_cr0)
IMP_COUNTER_FUNC(exe_write_cr3)
IMP_COUNTER_FUNC(exe_invlpg)
#endif

#ifdef BG_COUNTER_GROUP_STLB
IMP_COUNTER_FUNC(stlb_flush_full)
IMP_COUNTER_FUNC(stlb_flush_part)
IMP_COUNTER_FUNC(stlb_flush_elide)
IMP_COUNTER_FUNC(stlb_flush_page)
IMP_COUNTER_FUNC(stlb_resize)
IMP_COUNTER_FUNC(stlb_resize_inc)
IMP_COUNTER_FUNC(stlb_resize_dec)
#endif

#ifdef BG_COUNTER_GROUP_HAMT
#ifdef __BG_COUNTER_HAMT_P1
IMP_COUNTER_FUNC(hamt_fast)
IMP_COUNTER_FUNC(hamt_fast_badv0)
IMP_COUNTER_FUNC(hamt_fast_undef)
IMP_COUNTER_FUNC(hamt_fast_ld)
IMP_COUNTER_FUNC(hamt_fast_st)
IMP_COUNTER_FUNC(hamt_fast_ld_spt_ok)
IMP_COUNTER_FUNC(hamt_fast_ld_stlb_ok)
IMP_COUNTER_FUNC(hamt_fast_ld_ok)
IMP_COUNTER_FUNC(hamt_fast_st_spt_ok)
IMP_COUNTER_FUNC(hamt_fast_st_stlb_ok)
IMP_COUNTER_FUNC(hamt_fast_st_ok)
#endif
#ifdef __BG_COUNTER_HAMT_P2
IMP_COUNTER_FUNC(hamt_ld_spt_ok)
IMP_COUNTER_FUNC(hamt_ld_stlb_ok)
IMP_COUNTER_FUNC(hamt_st_spt_ok)
IMP_COUNTER_FUNC(hamt_st_stlb_ok)
#endif
#endif

#ifdef BG_COUNTER_GROUP_INDIRBR
IMP_COUNTER_GEN_FUNC(inbr_cpl0_ret)
IMP_COUNTER_GEN_FUNC(inbr_cpl0_call)
IMP_COUNTER_GEN_FUNC(inbr_cpl0_jmp)
IMP_COUNTER_GEN_FUNC(inbr_cpl3_ret)
IMP_COUNTER_GEN_FUNC(inbr_cpl3_call)
IMP_COUNTER_GEN_FUNC(inbr_cpl3_jmp)

IMP_COUNTER_STATIC_GEN_FUNC(inbr_cpl0_njc)
IMP_COUNTER_STATIC_GEN_FUNC(inbr_cpl0_njc_hit)
IMP_COUNTER_STATIC_GEN_FUNC(inbr_cpl3_njc)
IMP_COUNTER_STATIC_GEN_FUNC(inbr_cpl3_njc_hit)

IMP_COUNTER_CPL_FUNC(inbr_hp)
IMP_COUNTER_CPL_FUNC(inbr_hp_hit)
#endif

#define BG_LOG_DIFF(n, var) \
(__latxs_counter_data[n].var ## _nr - __local_latxs_counter_data[n].var ## _nr)

extern uint64_t data_storage_s[4];

static void __latxs_counter_bg_log(int n, int sec)
{
    qemu_bglog("[%7d][%1d] "\
            BG_COUNTER_LOG_TB       \
            BG_COUNTER_LOG_EXCP     \
            BG_COUNTER_LOG_INT      \
            BG_COUNTER_LOG_JC       \
            BG_COUNTER_LOG_HP_LDST  \
            BG_COUNTER_LOG_EXEC     \
            BG_COUNTER_LOG_STLB     \
            BG_COUNTER_LOG_HAMT_P0  \
            BG_COUNTER_LOG_HAMT_P1  \
            BG_COUNTER_LOG_HAMT_P2  \
            BG_COUNTER_LOG_INDIRBR  \
            BG_COUNTER_LOG_INDIRBR_NJC  \
            BG_COUNTER_LOG_MISC     \
            "\n" , sec, n
            BG_COUNTER_LOG_DATA_TB(n)
            BG_COUNTER_LOG_DATA_EXCP(n)
            BG_COUNTER_LOG_DATA_INT(n)
            BG_COUNTER_LOG_DATA_JC(n)
            BG_COUNTER_LOG_DATA_HP_LDST(n)
            BG_COUNTER_LOG_DATA_EXEC(n)
            BG_COUNTER_LOG_DATA_STLB(n)
            BG_COUNTER_LOG_DATA_HAMT_P0(n)
            BG_COUNTER_LOG_DATA_HAMT_P1(n)
            BG_COUNTER_LOG_DATA_HAMT_P2(n)
            BG_COUNTER_LOG_DATA_INDIRBR(n)
            BG_COUNTER_LOG_DATA_INDIRBR_NJC(n)
            BG_COUNTER_LOG_DATA_MISC(n)
            );
}

#define BG_COUNTER_COPY(n) do {             \
memcpy(&__local_latxs_counter_data[n],            \
       &__latxs_counter_data[n],      \
       sizeof(latxs_counter_t));            \
} while (0)

/* worker function */
static void latxs_counter_bg_log(int sec)
{
    BG_COUNTER_SYNC_HAMT(data_storage_s, __latxs_counter_data, 0);

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
