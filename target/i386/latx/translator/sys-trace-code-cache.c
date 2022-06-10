#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "trace.h"
#include "latxs-code-cache.h"

/*
 * option_trace_code_cache
 * [0] : trace TB translation
 * [1] : trace TB execution at the first time
 * [2] : trace TB invalidate
 * [3] : trace TB flush
 * [4] : print all TBs that are flushed
 */

int tracecc_has_tb_tr(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_TR;
}

int tracecc_has_tb_exec(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_EXEC;
}

int tracecc_has_tb_inv(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_INV;
}

int tracecc_has_tb_flush(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_FLUSH;
}

int tracecc_has_tb_flush_print(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_FLUSH_PRINT;
}



void latxs_tracecc_target_to_host(
        CPUX86State *env, TranslationBlock *tb)
{
    if (!tracecc_has_tb_tr()) {
        return;
    }
    fprintf(stderr, "[CC] TR %p %x %x %x %d %p %x\n",
            (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->icount,
            (void *)tb->tc.ptr,
            (uint32_t)env->cr[3]);
}

static int tb_trace_cc_done(TranslationBlock *tb)
{
    return tb->trace_cc & 0x1;
}

void latxs_tracecc_before_exec_tb(
        CPUX86State *env, TranslationBlock *tb)
{
    if (!tracecc_has_tb_exec() || tb_trace_cc_done(tb)) {
        return;
    }
    fprintf(stderr, "[CC] EXEC %p %x %x %x %x %d %p %lx %x %x %x\n",
            (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)env->cr[3],
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1]);
    tb->trace_cc |= 0x1;
}

static gboolean latxs_tracecc_tb(
        gpointer key, gpointer value, gpointer data)
{
    const TranslationBlock *tb = value;
    int *flushnr = data;

    fprintf(stderr, "[CC] FLUSH %d %p %x %x %x %x %d %p %lx %x %x\n",
            *flushnr, (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1]);

    return false;
}

void latxs_tracecc_do_tb_flush(void)
{
    if (tracecc_has_tb_flush()) {
        fprintf(stderr, "[CC] TB FLUSH\n");
    }

    if (tracecc_has_tb_flush_print()) {
        tcg_tb_foreach(latxs_tracecc_tb, &tb_ctx.tb_flush_count);
    }
}

void latxs_tracecc_tb_inv(TranslationBlock *tb)
{
    if (!tracecc_has_tb_inv()) {
        return;
    }

    fprintf(stderr, "[CC] INV %p %x %x %x %x %d %p %lx %x %x\n", (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1]);
}
