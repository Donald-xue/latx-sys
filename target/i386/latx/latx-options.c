#include "error.h"
#include "qemu/osdep.h"
#include "latx-options.h"

#include "latx-tracecc-init.h"

#include "tcg/tcg-bg-jc.h"
#include "tcg/tcg-bg-tlb-init.h"

int option_by_hand;
#ifdef CONFIG_LATX_FLAG_PATTERN
int option_flag_pattern = 0;
#endif
#ifdef CONFIG_LATX_FLAG_REDUCTION
int option_flag_reduction = 0;
#endif
int option_dump;
int option_dump_host;
int option_dump_ir1;
int option_dump_ir2;
int option_dump_cpl3;
int option_trace_tb;
int option_trace_ir1;
int option_check;
int option_tb_link;
int option_shadow_stack;
int option_lsfpu;
int option_xmm128map;
int option_ibtc;
int option_bne_b;

#ifdef CONFIG_SOFTMMU
/* For debug in softmmu */
int option_smmu_slow;
int option_break_point;
unsigned long long option_break_point_addrx;
unsigned long long option_break_point_count;
int option_native_printer;
int option_tb_max_insns;
int option_traceir1;
int option_traceir1_data;
int option_trace_code_cache;

/* For generate TB execution trace */
int option_trace_simple;
unsigned long long option_trace_start_nr;
unsigned long long option_trace_start_tb;
int option_trace_start_tb_set;

/* Optimization in softmmu */
int option_staticcs;
int option_sigint;
int option_cross_page_check;
int option_cross_page_jmp_link;
int option_by_hand_64;
int option_intb_link;
int option_intb_njc;
int option_intb_pb;
int option_soft_fpu;
int option_fast_fpr_ldst;
int option_large_code_cache;
int option_fastcs;
int option_hamt;
int option_hamt_delay;
int option_code_cache_pro;
int option_code_cache_region;
int option_code_cache_multi_region;
size_t option_code_cache_multi_region_size[2];
int    option_code_cache_multi_region_part[2];
int option_sys_flag_reduction;
int option_instptn;
int option_risk_mem_offset;
int option_jr_ra;

/* For QEMU monitor in softmmu */
int option_monitor_sc; /* Simple Counter */
int option_monitor_tc; /* Timer  Counter */
int option_monitor_rs; /* Register    Stat */
int option_monitor_ts; /* Translation Stat */

int option_monitor_cs; /* Complex Stat */
int option_monitor_jc; /* Jmp Cache Stat */
int option_monitor_tbht; /* TB Hash Table Stat */
int option_monitor_ps; /* Prilivage Switch */

int option_monitor_tbf; /* TB.flags */

void latxs_options_init(void);
#endif

unsigned long long counter_tb_exec;
unsigned long long counter_tb_tr;

unsigned long long counter_ir1_tr;
unsigned long long counter_mips_tr;

void options_init(void)
{
    option_dump_host = 0;
    option_dump_ir1 = 0;
    option_dump_ir2 = 0;
    option_dump = 0;
    option_trace_tb = 0;
    option_trace_ir1 = 0;
    option_check = 0;
    option_by_hand = 0;
    option_tb_link = 1;
    option_ibtc= 0;
    option_shadow_stack = 0;
    option_lsfpu = 0;
    option_xmm128map = 1;
	option_bne_b = 0;

    counter_tb_exec = 0;
    counter_tb_tr = 0;

    counter_ir1_tr = 0;
    counter_mips_tr = 0;

#ifdef CONFIG_SOFTMMU
    latxs_options_init();
#endif
}

#define OPTIONS_DUMP_FUNC 0
#define OPTIONS_DUMP_IR1 1
#define OPTIONS_DUMP_IR2 2
#define OPTIONS_DUMP_HOST 3
#define OPTIONS_DUMP_CPL3 4

static inline void __enable_dump_all(void)
{
    option_dump      = 1;
    option_dump_ir1  = 1;
    option_dump_ir2  = 1;
    option_dump_host = 1;
}

static inline void __disable_dump_all(void)
{
    option_dump      = 0;
    option_dump_ir1  = 0;
    option_dump_ir2  = 0;
    option_dump_host = 0;
}

void option_config_dump(int cpl)
{
    if (option_dump_cpl3) {
        if (cpl == 3) __enable_dump_all();
        else __disable_dump_all();
    }
}

void options_parse_dump(const char *bits)
{
    if (!bits) {
        return;
    }

    if (bits[OPTIONS_DUMP_CPL3] == '1') {
        option_dump_cpl3 = 1;
    } else if (bits[OPTIONS_DUMP_CPL3] == '0') {
        option_dump_cpl3 = 0;
    } else {
        lsassertm(0, "wrong options for dump cpl3.");
    }

    if (bits[OPTIONS_DUMP_IR1] == '1') {
        option_dump_ir1 = 1;
    } else if (bits[OPTIONS_DUMP_IR1] == '0') {
        option_dump_ir1 = 0;
    } else {
        lsassertm(0, "wrong options for dump ir1.");
    }

    if (bits[OPTIONS_DUMP_IR2] == '1') {
        option_dump_ir2 = 1;
    } else if (bits[OPTIONS_DUMP_IR2] == '0') {
        option_dump_ir2 = 0;
    } else {
        lsassertm(0, "wrong options for dump ir2.");
    }

    if (bits[OPTIONS_DUMP_HOST] == '1') {
        option_dump_host = 1;
    } else if (bits[OPTIONS_DUMP_HOST] == '0') {
        option_dump_host = 0;
    } else {
        lsassertm(0, "wrong options for dump host.");
    }

    if (bits[OPTIONS_DUMP_FUNC] == '1') {
        option_dump = 1;
    } else if (bits[OPTIONS_DUMP_FUNC] == '0') {
        option_dump = 0;
    } else {
        lsassertm(0, "wrong options for dump func.");
    }
}

#define OPTIONS_TRACE_TB 0
#define OPTIONS_TRACE_IR1 1

void options_parse_trace(const char *bits)
{
    if (!bits) {
        return;
    }

    if (bits[OPTIONS_TRACE_TB] == '1') {
        option_trace_tb = 1;
    } else if (bits[OPTIONS_TRACE_TB] == '0') {
        option_trace_tb = 0;
    } else {
        lsassertm(0, "wrong options for trace tb.");
    }

    if (bits[OPTIONS_TRACE_IR1] == '1') {
        option_trace_ir1 = 1;
    } else if (bits[OPTIONS_TRACE_IR1] == '0') {
        option_trace_ir1 = 0;
    } else {
        lsassertm(0, "wrong options for trace ir1 .");
    }
}

uint8 options_to_save(void)
{
    uint8 option_bitmap = 0;
    if (option_shadow_stack)
        option_bitmap |= 0x10;
    return option_bitmap;
}

#ifdef CONFIG_SOFTMMU

#include "qemu-def.h"
#include "qemu/option.h"
#include "qemu/option_int.h"

int latxs_allow_cross_page_link(void)
{
    return option_cross_page_jmp_link;
}

#define LATXS_OPT_by_hand               0
#define LATXS_OPT_tb_link               1
#define LATXS_OPT_intb_link             2
#define LATXS_OPT_intb_njc              3
#define LATXS_OPT_intb_pb               4
#define LATXS_OPT_lsfpu                 5
#define LATXS_OPT_staticcs              6
#define LATXS_OPT_smmu_slow             7
#define LATXS_OPT_trace_simple          8
#define LATXS_OPT_cross_page_check      9
#define LATXS_OPT_sigint                10
#define LATXS_OPT_cross_page_jmp_link   11
#define LATXS_OPT_large_code_cache      12
#define LATXS_OPT_soft_fpu              13
#define LATXS_OPT_fast_fpr_ldst         14
#define LATXS_OPT_by_hand_64            15
#define LATXS_OPT_fastcs                16
#define LATXS_OPT_bne_b                 17

void latx_sys_parse_options(QemuOpts *opts);
void parse_options_bool(int index, bool set);

void parse_options_traces(unsigned long long t);
void options_parse_trace_ir1_id(unsigned long long id);
void options_parse_tracestart_nr(unsigned long long ts);
void options_parse_tracestart_tb(unsigned long long tbpc);
void options_parse_bpc(unsigned long long addrx);
void options_parse_bpn(unsigned long long tbnum);
void options_parse_monitor(const char *bits);

void dump_latxs_options(void);

QemuOptsList qemu_latx_opts = {
    .name = "latx",
    .implied_opt_name="optm",
    .head = QTAILQ_HEAD_INITIALIZER(qemu_latx_opts.head),
    .desc = {
        {
            .name = "optm",
            .type = QEMU_OPT_STRING,
            .help = "none/select/all optimizations to enable",
        },{
            .name = "trbh",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable translate by hand",
        },{
            .name = "tblink",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable TB-link(aka TB-chaining)",
        }, {
            .name = "intblink",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable indirect TB-link",
        }, {
            .name = "bne_b",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable jcc branch combination",
        }, {
            .name = "trbh64",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable translate by hand 64",
        }, {
            .name = "lsfpu",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable using loongson FPU",
        }, {
            .name = "staticcs",
            .type = QEMU_OPT_BOOL,
            .help = "use static generated codes for context switch",
        }, {
            .name = "largecc",
            .type = QEMU_OPT_BOOL,
            .help = "use pcaddu18i and jirl for large than 128MB code cache",
        }, {
            .name = "softfpu",
            .type = QEMU_OPT_BOOL,
            .help = "use soft float for x87 emulation",
        }, {
            .name = "ffldst",
            .type = QEMU_OPT_BOOL,
            .help = "fast fpr load/store",
        }, {
            .name = "njc",
            .type = QEMU_OPT_BOOL,
            .help = "use Native Jmp Cache lookup",
        }, {
            .name = "pb",
            .type = QEMU_OPT_BOOL,
            .help = "private buffer for indirect branch",
        }, {
            .name = "jc",
            .type = QEMU_OPT_NUMBER,
            .help = "jmp cache size in bits (1<<bits)",
        }, {
            .name = "jcpage",
            .type = QEMU_OPT_NUMBER,
            .help = "jmp cache page size in bits (1<<bits)",
        }, {
            .name = "sigint",
            .type = QEMU_OPT_BOOL,
            .help = "signal interrupt handle",
        }, {
            .name = "fastcs",
            .type = QEMU_OPT_NUMBER,
            .help = "fast context switch",
        }, {
            .name = "hamt",
            .type = QEMU_OPT_NUMBER,
            .help = "hardware accelerated memory translation",
        }, {
            .name = "hamtdelay",
            .type = QEMU_OPT_NUMBER,
            .help = "delay on HAMT exception handler",
        }, {
            .name = "tlbsize",
            .type = QEMU_OPT_NUMBER,
            .help = "SoftTLB size",
        }, {
            .name = "ccpro",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache pro",
        }, {
            .name = "ccmultiregion",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache multi region",
        }, {
            .name = "ccregion",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache region",
        }, {
            .name = "ccmrsize0",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache region 0 size",
        }, {
            .name = "ccmrsize1",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache region 1 size",
        }, {
            .name = "ccmrpart0",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache region 0 part",
        }, {
            .name = "ccmrpart1",
            .type = QEMU_OPT_NUMBER,
            .help = "code cache region 1 part",
        }, {
            .name = "sysflag",
            .type = QEMU_OPT_NUMBER,
            .help = "optimization for eflags",
        }, {
            .name = "instptn",
            .type = QEMU_OPT_NUMBER,
            .help = "instruction pattern",
        }, {
            .name = "risky",
            .type = QEMU_OPT_NUMBER,
            .help = "optimization with risk",
        }, {
            .name = "jrra",
            .type = QEMU_OPT_NUMBER,
            .help = "call return using SCR",
        }, {
            .name = "smmu",
            .type = QEMU_OPT_BOOL,
            .help = "use softmmu slow path only (debug)",
        }, {
            .name = "tbsizei",
            .type = QEMU_OPT_NUMBER,
            .help = "limit TB's max num of instructions. 0 for default 255",
        }, {
            .name = "np",
            .type = QEMU_OPT_NUMBER,
            .help = "enable print helper to be called from native codes",
        }, {
            .name = "dump",
            .type = QEMU_OPT_STRING,
            .help = "dump flags for func,ir1,ir2,host",
        }, {
            .name = "trace",
            .type = QEMU_OPT_STRING,
            .help = "trace flags for TB,ir1",
        }, {
            .name = "traces",
            .type = QEMU_OPT_NUMBER,
            .help = "simple trace tb execution",
        }, {
            .name = "tracestnr",
            .type = QEMU_OPT_NUMBER,
            .help = "start to print trace after @number of TB's execution",
        }, {
            .name = "tracesttb",
            .type = QEMU_OPT_NUMBER,
            .help = "start to print trace after TB.PC",
        }, {
            .name = "tracecc",
            .type = QEMU_OPT_NUMBER,
            .help = "trace code cache block",
        }, {
            .name = "cclog",
            .type = QEMU_OPT_BOOL,
            .help = "enable trace cc log file",
        }, {
            .name = "cclogname",
            .type = QEMU_OPT_STRING,
            .help = "input trace cc log file",
        }, {
            .name = "traceir1",
            .type = QEMU_OPT_NUMBER,
            .help = "trace ir1 by type or id",
        }, {
            .name = "traceir1data",
            .type = QEMU_OPT_NUMBER,
            .help = "another value for trace ir1",
        }, {
            .name = "breakpoint",
            .type = QEMU_OPT_NUMBER,
            .help = "build break point code to use",
        }, {
            .name = "bpnum",
            .type = QEMU_OPT_NUMBER,
            .help = "break point TB exec number",
        }, {
            .name = "cpc",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable cross page check in system",
        }, {
            .name = "cpjl",
            .type = QEMU_OPT_BOOL,
            .help = "enable/disable cross page direct jmp link",
        }, {
            .name = "monitor",
            .type = QEMU_OPT_STRING,
            .help = "enable/disable XQM's monitor information",
        }, {
            .name = "verbose",
            .type = QEMU_OPT_BOOL,
            .help = "pint the options parsing result",
        },
        { /* end of list */ }
    },
};

static
void latx_sys_check_options(void)
{
    /* This function currently does nothing. */
}

static
void set_options(int index, int v)
{
    switch(index) {
    case LATXS_OPT_by_hand:
        option_by_hand = v;
        break;
    case LATXS_OPT_tb_link:
        option_tb_link = v;
        break;
    case LATXS_OPT_lsfpu:
        option_lsfpu = v;
        break;
    case LATXS_OPT_staticcs:
        option_staticcs = v;
        break;
    case LATXS_OPT_large_code_cache:
        option_large_code_cache = v;
        break;
    case LATXS_OPT_soft_fpu:
        option_soft_fpu = v;
        break;
    case LATXS_OPT_fast_fpr_ldst:
        option_fast_fpr_ldst = v;
        break;
    case LATXS_OPT_sigint:
        option_sigint = v;
        break;
    case LATXS_OPT_smmu_slow:
        option_smmu_slow = v;
        break;
    case LATXS_OPT_trace_simple:
        option_trace_simple = v;
        break;
    case LATXS_OPT_cross_page_check:
        option_cross_page_check = v;
        break;
    case LATXS_OPT_cross_page_jmp_link:
        option_cross_page_jmp_link = v;
        break;
    case LATXS_OPT_intb_link:
        option_intb_link = v;
        break;
    case LATXS_OPT_intb_njc:
        option_intb_njc = v;
        break;
    case LATXS_OPT_intb_pb:
        option_intb_pb = v;
        break;
    case LATXS_OPT_by_hand_64:
        option_by_hand_64 = v;
        break;
	case LATXS_OPT_bne_b:
        option_bne_b = v;
        break;
    default: break;
    }
}

static void parse_options_native_printer(unsigned long long t)
{
    option_native_printer = t;
}

static void parse_options_trace_code_cache(unsigned long long t)
{
    /*
     * [0] : trace TB translation
     * [1] : trace TB execution at the first time
     * [2] : trace TB invalidate
     * [3] : trace TB flush
     * [4] : print all TBs that are flushed
     */
    option_trace_code_cache = t;
}

static void parse_options_cclog(bool enable)
{
    latx_tracecc_log_init(enable, NULL);
}

static void parse_options_cclogname(const char *name)
{
    latx_tracecc_log_init(true, name);
}

static void parse_options_traceir1(unsigned long long t)
{
    /*
     * t = 0 : disable
     * t = 1 : print each type of IR1 once
     * t = 2 : keep printing one type of IR1
     *         IR1 id is stored in @option_trace_ir1_data
     */
    option_traceir1 = t;
}

static void parse_options_traceir1_data(unsigned long long t)
{
    option_traceir1_data = t;
}

static void parse_options_hamt(unsigned long long t)
{
    option_hamt = t;
}

static void parse_options_tlb_size(unsigned long long t)
{
    tcg_bg_tlb_init_size(t);
}

static void parse_options_jmp_cache(unsigned long long t)
{
    tcg_bg_jc_init_jmp_cache_bits(t);
}

static void parse_options_jmp_cache_page(unsigned long long t)
{
    tcg_bg_jc_init_jmp_cache_page_bits(t);
}

static void parse_options_hamt_delay(unsigned long long t)
{
    option_hamt_delay = t;
}

static void parse_options_fastcs(unsigned long long t)
{
    option_fastcs = t;
}

static void parse_options_code_cache_pro(unsigned long long t)
{
    option_code_cache_pro = t;
}

static void parse_options_code_cache_region(unsigned long long t)
{
    option_code_cache_region = t;
}

static void parse_options_code_cache_multi_region(unsigned long long t)
{
    option_code_cache_multi_region = t;
}

static void parse_options_code_cache_multi_region_size(int rid, 
        unsigned long long t)
{
    option_code_cache_multi_region_size[rid] = t;
}
static void parse_options_code_cache_multi_region_part(int rid, 
        unsigned long long t)
{
    option_code_cache_multi_region_part[rid] = t;
}

#define LATXS_OPT_FLAG_REDUCTION    (1 << 0)
static void parse_options_sys_flag(unsigned long long t)
{
    option_sys_flag_reduction = t & LATXS_OPT_FLAG_REDUCTION;
}

static void parse_options_instptn(unsigned long long t)
{
    option_instptn = t;
}

#define LATXS_RISKY_MEM_OFFSET  0x1
static void parse_options_risky(unsigned long long t)
{
    option_risk_mem_offset = t & LATXS_RISKY_MEM_OFFSET;
}

static void parse_options_jrra(unsigned long long t)
{
    option_jr_ra = t;
}

static void parse_options_tbsizei(unsigned long long i)
{
    if (i && i < 255) {
        option_tb_max_insns = i;
    }
}

void latx_sys_parse_options(QemuOpts *opts)
{
    QemuOpt     *opt;
    const char  *str;
    int         need_parse_optimizations = 1;
    int         verbose = 0;

    opt = qemu_opt_find(opts, "verbose");
    if (opt && opt->value.boolean) {
        verbose = 1;
    }

    if (verbose) {
        qemu_opts_print(opts,"\n");
        printf("\n");
    }

    /* optm=none|select|all */
    opt = qemu_opt_find(opts, "optm");
    str = opt->str;
    if (strncmp(str, "all", 3) == 0) {
        if (verbose) {
            printf("[optimization] enable all optimizations.\n");
        }
        set_options(LATXS_OPT_by_hand, 1);
        set_options(LATXS_OPT_tb_link, 1);
        set_options(LATXS_OPT_lsfpu, 1);
        set_options(LATXS_OPT_bne_b, 1);
        set_options(LATXS_OPT_staticcs, 1);
        set_options(LATXS_OPT_intb_njc, 1);
        need_parse_optimizations = 0;
    } else if (strncmp(str, "none", 4) == 0) {
        if (verbose) {
            printf("[optimization] disable all optimizations.\n");
        }
        need_parse_optimizations = 0;
    } else if (strncmp(str, "select", 6) == 0) {
        if (verbose) {
            printf("[optimization] select optimizations to enable.\n");
        }
    } else {
        lsassertm(0, "optm must be one of {none, select, all}\n");
    }

    opt = qemu_opt_find(opts, "dump");
    if (opt) options_parse_dump(opt->str);

    opt = qemu_opt_find(opts, "trace");
    if (opt) options_parse_trace(opt->str);

    opt = qemu_opt_find(opts, "monitor");
    if (opt) options_parse_monitor(opt->str);

    opt = qemu_opt_find(opts, "traces");
    if (opt) parse_options_traces(opt->value.uint);

    opt = qemu_opt_find(opts, "tracestnr");
    if (opt) options_parse_tracestart_nr(opt->value.uint);

    opt = qemu_opt_find(opts, "tracesttb");
    if (opt) options_parse_tracestart_tb(opt->value.uint);

    opt = qemu_opt_find(opts, "breakpoint");
    if (opt) options_parse_bpc(opt->value.uint);

    opt = qemu_opt_find(opts, "bpnum");
    if (opt) options_parse_bpn(opt->value.uint);

    opt = qemu_opt_find(opts, "smmu");
    if (opt) parse_options_bool(LATXS_OPT_smmu_slow, opt->value.boolean);

    opt = qemu_opt_find(opts, "tbsizei");
    if (opt) {
        parse_options_tbsizei(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "cpc");
    if (opt) parse_options_bool(LATXS_OPT_cross_page_check, opt->value.boolean);

    opt = qemu_opt_find(opts, "cpjl");
    if (opt) {
        parse_options_bool(LATXS_OPT_cross_page_jmp_link,
                opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "np");
    if (opt) {
        parse_options_native_printer(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "tracecc");
    if (opt) {
        parse_options_trace_code_cache(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "cclog");
    if (opt) {
        parse_options_cclog(opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "cclogname");
    if (opt) {
        parse_options_cclogname(opt->str);
    }

    opt = qemu_opt_find(opts, "traceir1");
    if (opt) {
        parse_options_traceir1(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "traceir1data");
    if (opt) {
        parse_options_traceir1_data(opt->value.uint);
    }

    /* parse all optimizations as last! */
    if (!need_parse_optimizations) {
        goto _OUT;
    }

    opt = qemu_opt_find(opts, "trbh");
    if (opt) parse_options_bool(LATXS_OPT_by_hand, opt->value.boolean);

    opt = qemu_opt_find(opts, "tblink");
    if (opt) parse_options_bool(LATXS_OPT_tb_link, opt->value.boolean);

    opt = qemu_opt_find(opts, "lsfpu");
    if (opt) parse_options_bool(LATXS_OPT_lsfpu, opt->value.boolean);

	opt = qemu_opt_find(opts, "bne_b");                              
	if (opt) parse_options_bool(LATXS_OPT_bne_b, opt->value.boolean);

    opt = qemu_opt_find(opts, "staticcs");
    if (opt) parse_options_bool(LATXS_OPT_staticcs, opt->value.boolean);

    opt = qemu_opt_find(opts, "largecc");
    if (opt) {
        parse_options_bool(LATXS_OPT_large_code_cache, opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "softfpu");
    if (opt) {
        parse_options_bool(LATXS_OPT_soft_fpu, opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "ffldst");
    if (opt) {
        parse_options_bool(LATXS_OPT_fast_fpr_ldst, opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "njc");
    if (opt) parse_options_bool(LATXS_OPT_intb_njc, opt->value.boolean);

    opt = qemu_opt_find(opts, "pb");
    if (opt) parse_options_bool(LATXS_OPT_intb_pb, opt->value.boolean);

    opt = qemu_opt_find(opts, "sigint");
    if (opt) {
        parse_options_bool(LATXS_OPT_sigint, opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "hamt");
    if (opt) {
        parse_options_hamt(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "tlbsize");
    if (opt) {
        parse_options_tlb_size(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "jc");
    if (opt) {
        parse_options_jmp_cache(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "jcpage");
    if (opt) {
        parse_options_jmp_cache_page(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "hamtdelay");
    if (opt) {
        parse_options_hamt_delay(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "fastcs");
    if (opt) {
        parse_options_fastcs(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "ccpro");
    if (opt) {
        parse_options_code_cache_pro(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "ccregion");
    if (opt) {
        parse_options_code_cache_region(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "ccmrsize0");
    if (opt) {
        parse_options_code_cache_multi_region_size(0, opt->value.uint);
    }
    opt = qemu_opt_find(opts, "ccmrsize1");
    if (opt) {
        parse_options_code_cache_multi_region_size(1, opt->value.uint);
    }
    opt = qemu_opt_find(opts, "ccmrpart0");
    if (opt) {
        parse_options_code_cache_multi_region_part(0, opt->value.uint);
    }
    opt = qemu_opt_find(opts, "ccmrpart1");
    if (opt) {
        parse_options_code_cache_multi_region_part(1, opt->value.uint);
    }

    opt = qemu_opt_find(opts, "ccmultiregion");
    if (opt) {
        parse_options_code_cache_multi_region(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "sysflag");
    if (opt) {
        parse_options_sys_flag(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "instptn");
    if (opt) {
        parse_options_instptn(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "risky");
    if (opt) {
        parse_options_risky(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "jrra");
    if (opt) {
        parse_options_jrra(opt->value.uint);
    }

    opt = qemu_opt_find(opts, "intblink");
    if (opt) {
        parse_options_bool(LATXS_OPT_intb_link, opt->value.boolean);
    }

    opt = qemu_opt_find(opts, "trbh64");
    if (opt) {
        parse_options_bool(LATXS_OPT_by_hand_64, opt->value.boolean);
    }

_OUT:
    if (verbose) dump_latxs_options();

    latx_sys_check_options();

    return;
}

void parse_options_bool(int index, bool set)
{
    if (set) {
        set_options(index, 1);
    } else {
        set_options(index, 0);
    }
}

void options_parse_monitor(const char *bits)
{
    if (!bits) return;

    int len = strlen(bits);

#define OPTION_MONITOR_PARSE(idx, var) \
    if (len >= (idx+ 1)) { \
        switch (bits[idx]) { \
        case '7': var = 7; break; \
        case '6': var = 6; break; \
        case '5': var = 5; break; \
        case '4': var = 4; break; \
        case '3': var = 3; break; \
        case '2': var = 2; break; \
        case '1': var = 1; break; \
        case '0': var = 0; break; \
        default: break; \
        } \
    }

OPTION_MONITOR_PARSE(0, option_monitor_sc)
OPTION_MONITOR_PARSE(1, option_monitor_tc)
OPTION_MONITOR_PARSE(2, option_monitor_rs)
OPTION_MONITOR_PARSE(3, option_monitor_ts)

OPTION_MONITOR_PARSE(4, option_monitor_cs)
OPTION_MONITOR_PARSE(5, option_monitor_jc)
OPTION_MONITOR_PARSE(6, option_monitor_tbht)
OPTION_MONITOR_PARSE(7, option_monitor_ps)

OPTION_MONITOR_PARSE(8, option_monitor_tbf)
}

void parse_options_traces(unsigned long long t)
{
    /*
     * latx/sys-tb-trace.c
     * latx/include/latx-tb-trace-sys.h
     *
     * tb   = t[3:0]
     * cpl  = t[7:4]
     * more = t[11:8]
     *
     * The larger the number is, the more information to print:
     *     tb == 1 : PC/EFLAGS/REGS
     *     tb >= 2 : PC/EFLAGS/REGS/FPU
     */
    option_trace_simple = t;
}

void options_parse_tracestart_nr(unsigned long long ts)
{
    option_trace_start_nr = ts;
}

void options_parse_tracestart_tb(unsigned long long tbpc)
{
    option_trace_start_tb = tbpc;
    option_trace_start_tb_set = 1;
}

void options_parse_bpc(unsigned long long addrx)
{
    option_break_point = 1;
    option_break_point_addrx = addrx;
}

void options_parse_bpn(unsigned long long tbnum)
{
    option_break_point_count = tbnum;
}

void dump_latxs_options(void)
{
    printf("[LATXS-OPT] dump = %d\n", option_dump);
    printf("[LATXS-OPT] dump_host = %d\n", option_dump_host);
    printf("[LATXS-OPT] dump_ir1 = %d\n", option_dump_ir1);
    printf("[LATXS-OPT] dump_ir2 = %d\n", option_dump_ir2);
    printf("[LATXS-OPT] trace_tb = %d\n", option_trace_tb);
    printf("[LATXS-OPT] trace_tb_ir1 = %d\n", option_trace_ir1);
    printf("[LATXS-OPT] trace_simple = %d\n", option_trace_simple);
    printf("[LATXS-OPT] trace_start_nr = %lld\n", option_trace_start_nr);
    printf("[LATXS-OPT] trace_start_tb = %#llx\n", option_trace_start_tb);
    printf("[LATXS-OPT] by_hand = %d\n", option_by_hand);
    printf("[LATXS-OPT] tb_link = %d\n", option_tb_link);
    printf("[LATXS-OPT] lsfpu = %d\n", option_lsfpu);
    printf("[LATXS-OPT] staticcs = %d\n", option_staticcs);
    printf("[LATXS-OPT] large code cache = %d\n", option_large_code_cache);
    printf("[LATXS-OPT] large soft fpu = %d\n", option_soft_fpu);
    printf("[LATXS-OPT] fast frp ldst = %d\n", option_fast_fpr_ldst);
    printf("[LATXS-OPT] indirect branch loolup helper = %d\n", option_intb_link);
    printf("[LATXS-OPT] native jmp cache loolup = %d\n", option_intb_njc);
    printf("[LATXS-OPT] private buffer loolup = %d\n", option_intb_pb);
    printf("[LATXS-OPT] by_hand_64 = %d\n", option_by_hand_64);
    printf("[LATXS-OPT] fastcs = %d\n", option_fastcs);
    printf("[LATXS-OPT] Code Cache Pro = %d\n", option_code_cache_pro);
    printf("[LATXS-OPT] bne_b = %d\n", option_bne_b);
#ifdef CONFIG_SOFTMMU
    printf("[LATXS-OPT] monitor 1 simple    counter = %d\n", option_monitor_sc);
    printf("[LATXS-OPT] monitor 2 timer     counter = %d\n", option_monitor_tc);
    printf("[LATXS-OPT] monitor 3 register    stat  = %d\n", option_monitor_rs);
    printf("[LATXS-OPT] monitor 4 translation stat  = %d\n", option_monitor_ts);
    printf("[LATXS-OPT] monitor 5 complex     stat  = %d\n", option_monitor_cs);
    printf("[LATXS-OPT] monitor 6 jmp cache profile = %d\n", option_monitor_jc);
    printf("[LATXS-OPT] monitor 7 TB hash table     = %d\n", option_monitor_tbht);
    printf("[LATXS-OPT] monitor 8 prilivage switch  = %d\n", option_monitor_ps);
    printf("[LATXS-OPT] monitor 9 TB flags change   = %d\n", option_monitor_tbf);
#endif
    printf("[LATXS-OPT] break point = %d\n", option_break_point);
    printf("[LATXS-OPT] break point TB = %#llx\n", option_break_point_addrx);
    printf("[LATXS-OPT] break point nr = %lld\n", option_break_point_count);
    printf("[LATXS-OPT] cross page check = %d\n", option_cross_page_check);

}

void latxs_options_init(void)
{
    option_tb_link = 1;
    option_by_hand = 1;
    option_staticcs = 1;
    option_lsfpu = 1;
    option_bne_b = 1;

    option_intb_link = 0;
    option_intb_njc  = 0;
    option_intb_pb   = 0;
    option_sigint = 0;
    option_fastcs = 0;
    option_hamt = 0;
    option_hamt_delay = 0;
    option_cross_page_check = 0;
    option_cross_page_jmp_link = 0;
    option_large_code_cache = 0;
    option_soft_fpu = 0;
    option_fast_fpr_ldst = 0;
    option_by_hand_64 = 0;
    option_code_cache_pro = 0;
    /* one region for one code cache */
    option_code_cache_region = 1;
    option_code_cache_multi_region = 0;
    option_code_cache_multi_region_size[0] = 128;
    option_code_cache_multi_region_size[1] = 128;
    option_code_cache_multi_region_part[0] = 8;
    option_code_cache_multi_region_part[1] = 8;
    option_sys_flag_reduction = 0;
    option_instptn = 0;

    option_smmu_slow = 0;

    option_tb_max_insns = 0;

    option_break_point = 0;
    option_break_point_addrx = 0;
    option_break_point_count = 0;

    option_native_printer = 0;

    option_traceir1 = 0;
    option_traceir1_data = 0;

    option_trace_simple = 0;
    option_trace_start_nr = 0;
    option_trace_start_tb = 0;
    option_trace_start_tb_set = 0;

    option_monitor_sc = 0;
    option_monitor_tc = 0;
    option_monitor_rs = 0;
    option_monitor_ts = 0;
    option_monitor_cs = 0;
    option_monitor_jc = 0;
    option_monitor_tbht = 0;
    option_monitor_ps = 0;
    option_monitor_tbf = 0;

    option_test_sys = 0;
}

#endif
