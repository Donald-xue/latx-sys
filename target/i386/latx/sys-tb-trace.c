#include "common.h"
#include "translate.h"
#include "latx-options.h"
#include "latx-tb-trace-sys.h"

#ifdef LATXS_TB_TRACE_ENABLE

#define TRACE_TB_MASK       0xF
#define TRACE_CPL_MASK      0xF0
#define TRACE_MORE_MASK     0xF00

#define TRACE_MORE_TB_PER_SEC   0x1

#define TRACE_TB_SHIFT      0
#define TRACE_CPL_SHIFT     4
#define TRACE_MORE_SHIFT    8

#define GET_TRACE(name) \
((option_trace_simple & TRACE_##name##_MASK) >> TRACE_##name##_SHIFT)

static void latxs_tb_trace_more(int more,
        CPUX86State *env, TranslationBlock *tb)
{
    if (more & TRACE_MORE_TB_PER_SEC) {
        static uint64_t enter_tb_count = 0;
        static int64_t st = 0, cur = 0;
        enter_tb_count += 1;
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        cur = ts.tv_sec;
        if (cur != st) {
            st = cur;
            printf("[%ld] exec=%ld\n", cur, enter_tb_count);
        }
    }
}

void __latxs_tb_trace(CPUX86State *env, TranslationBlock *tb)
{
    if (!option_trace_simple) {
        return;
    }

    int trace_tb   = GET_TRACE(TB);
    int trace_cpl  = GET_TRACE(CPL);
    int trace_more = GET_TRACE(MORE);

    latxs_tb_trace_more(trace_more, env, tb);

    /* filter with CPL */
    if ((tb->flags & 0x3) < trace_cpl) {
        return;
    }

    /* start trace from given TB */
    static int latxs_sttb;
    if (option_trace_start_tb_set &&
        !latxs_sttb &&
        tb->pc != option_trace_start_tb) {
        return;
    }
    latxs_sttb = 1;

    /* start trace after given number of TB's execution */
    static int latxs_stnr;
    if (latxs_stnr < option_trace_start_nr) {
        latxs_stnr += 1;
        return;
    }

    /* print tb execution */
    if (!trace_tb) {
        return;
    }

    uint32_t eflags = cpu_compute_eflags(env);

    fprintf(stderr, "[tracesp] ");
    fprintf(stderr, "PC=0x"TARGET_FMT_lx" / ", tb->pc);
    fprintf(stderr, "CS=0x"TARGET_FMT_lx" / ", tb->cs_base);
    fprintf(stderr, "EF=0x%x / ", eflags);
    if (tb->cflags & CF_USE_ICOUNT) {
        X86CPU *xcpu = env_archcpu(env);
        fprintf(stderr, "ICOUNT=0x%x / ", xcpu->neg.icount_decr.u32);
    }
    switch (trace_tb) {
    case 2: /* Print with FPU state */
        fprintf(stderr, "TOP=%d / ",  env->fpstt);
        if (!option_lsfpu) {
            fprintf(stderr, "TOPin=%d / ", tb->_top_in);
            fprintf(stderr, "TOPot=%d / ", tb->_top_out);
        }
        fprintf(stderr, "RM=%d / ", env->fp_status.float_rounding_mode);
        fprintf(stderr, "FP=0x%x.0x%lx 0x%x.0x%lx 0x%x.0x%lx 0x%x.0x%lx"
                           "0x%x.0x%lx 0x%x.0x%lx 0x%x.0x%lx 0x%x.0x%lx / ",
                           env->fpregs[0].d.high, env->fpregs[0].d.low,
                           env->fpregs[1].d.high, env->fpregs[1].d.low,
                           env->fpregs[2].d.high, env->fpregs[2].d.low,
                           env->fpregs[3].d.high, env->fpregs[3].d.low,
                           env->fpregs[4].d.high, env->fpregs[4].d.low,
                           env->fpregs[5].d.high, env->fpregs[5].d.low,
                           env->fpregs[6].d.high, env->fpregs[6].d.low,
                           env->fpregs[7].d.high, env->fpregs[7].d.low);
        break;
    default:
        break;
    }
    fprintf(stderr, "REGS=0x"TARGET_FMT_lx",0x"TARGET_FMT_lx
                        ",0x"TARGET_FMT_lx",0x"TARGET_FMT_lx
                        ",0x"TARGET_FMT_lx",0x"TARGET_FMT_lx
                        ",0x"TARGET_FMT_lx",0x"TARGET_FMT_lx"\n",
            env->regs[0], env->regs[1], env->regs[2], env->regs[3],
            env->regs[4], env->regs[5], env->regs[6], env->regs[7]);
}

#endif
