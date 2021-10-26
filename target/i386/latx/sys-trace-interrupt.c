#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>
#include "trace.h"

/* do_interrupt_protected */
void sys_trace_do_interrupt_pt(void *env,
        int intno, int is_int, int error_code, int is_hw)
{
    trace_latx_do_int_pt(pthread_self(), intno, is_int, get_clock());
}

/* do_interrupt_real */
void sys_trace_do_interrupt_rl(void *env,
        int intno, int is_int, int error_code)
{
    trace_latx_do_int_rl(pthread_self(), intno, is_int, get_clock());
}

/* latxs_rr_interrupt_signal_handler */
void sys_trace_sigint_handler(unsigned long pc)
{
    trace_latx_sigint_handler(pthread_self(), get_clock(), pc);
}

/* latxs_rr_interrupt_signal_handler */
void sys_trace_sigint_event(const char *eventstr, void *_tb)
{
    TranslationBlock *tb = _tb;
    trace_latx_sigint_event(pthread_self(), get_clock(),
            eventstr, tb ? tb->pc : 0);
}

/* latxs_tb_unlink */
void sys_trace_sigint_unlink(void *_tb)
{
    TranslationBlock *tb = _tb;
    trace_latx_sigint_unlink(pthread_self(), get_clock(),
            tb ? tb->pc : 0,
            tb ? tb->is_indir_tb : -1);
}

/* latxs_tb_relink */
void sys_trace_sigint_relink(void *_tb)
{
    TranslationBlock *tb = _tb;
    trace_latx_sigint_relink(pthread_self(), get_clock(),
            tb ? tb->pc : 0,
            tb ? tb->is_indir_tb : -1);
}

/* x86_cpu_exec_interrupt */
void sys_trace_x86_exec_interrupt(int intreqin, int intreqout)
{
    trace_latx_exe_int(pthread_self(), get_clock(),
            intreqin, intreqout);
}

/* tcg_handle_interrupt */
void sys_trace_send_interrupt(int mask)
{
    trace_latx_send_int(pthread_self(), get_clock(), mask);
}
