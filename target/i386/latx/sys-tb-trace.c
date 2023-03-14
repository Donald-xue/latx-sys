#include "common.h"
#include "translate.h"
#include "latx-options.h"
#include "latx-tb-trace-sys.h"

#ifdef LATXS_TB_TRACE_ENABLE

void __latxs_tb_trace(CPUX86State *env, TranslationBlock *tb)
{
    if (!option_trace_simple) {
        return;
    }

    int cpl = (option_trace_simple & 0xF0) >> 4;
    if ((tb->flags & 0x3) < cpl) {
        return;
    }

    static int latxs_sttb;
    if (option_trace_start_tb_set &&
        !latxs_sttb &&
        tb->pc != option_trace_start_tb) {
        return;
    }
    latxs_sttb = 1;

    static int latxs_stnr;
    if (latxs_stnr < option_trace_start_nr) {
        latxs_stnr += 1;
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
    switch (option_trace_simple & 0xF) {
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
