#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "trace.h"
#include "latxs-code-cache.h"
#include "latxs-fastcs-cfg.h"
#include "latxs-cc-pro.h"

#ifdef LATXS_TRACECC_ENABLE

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

int tracecc_has_tb_link(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_LINK;
}

void latxs_tracecc_gen_tb_insert(TranslationBlock *tb,
        uint64_t p1, uint64_t p2, int exist)
{
    if (!tracecc_has_tb_tr()) {
        return;
    }

    fprintf(stderr, "[CC] Insert %p %p %p %d 0x%lx\n",
            (void *)tb, (void *)p1, (void *)p2, exist,
            tb->tc.size);
}

void latxs_tracecc_target_to_host(
        CPUX86State *env, TranslationBlock *tb)
{
    if (!tracecc_has_tb_tr()) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *ir1_list = td->ir1_inst_array;
    int ir1_nr = td->ir1_nr;

    uint8_t fcs = (latxs_fastcs_enable_tbctx()) ? tb->fastcs_ctx : -1;
    fprintf(stderr, "[CC] TR %p %x %x %x %d %p %x %x %d ",
            (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->icount,
            (void *)tb->tc.ptr,
            (uint32_t)env->cr[3],
            fcs, tb->cc_flags);

    int i = 0, j = 0;
    IR1_INST *pir1 = NULL;
    for (i = 0; i < ir1_nr; ++i) {
        pir1 = ir1_list + i;
        for (j = 0; j < pir1->info->size; j++) {
            fprintf(stderr, "%02x", pir1->info->bytes[j]);
        }
    }
    fprintf(stderr, "\n");
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
    uint8_t fcs = (latxs_fastcs_enable_tbctx()) ? tb->fastcs_ctx : -1;
    fprintf(stderr, "[CC] EXEC %p %x %x %x %x %d %p %lx %x %x %x %x\n",
            (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)env->cr[3],
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1], fcs);
    tb->trace_cc |= 0x1;
}

static gboolean latxs_tracecc_tb(
        gpointer key, gpointer value, gpointer data)
{
    const TranslationBlock *tb = value;
    int *flushnr = data;

    uint8_t fcs = (latxs_fastcs_enable_tbctx()) ? tb->fastcs_ctx : -1;
    fprintf(stderr, "[CC] FLUSH %d %p %x %x %x %x %d %p %lx %x %x %ld %x %d\n",
            *flushnr, (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1],
            (uint64_t)tb->tb_exec_nr,
            fcs, tb->cc_flags);

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

void latxs_tracecc_gen_tb_start(void)
{
    if (tracecc_has_tb_flush_print()) {
        TRANSLATION_DATA *td = lsenv->tr_data;
        TranslationBlock *tb = td->curr_tb;

        IR2_OPND tbbase = latxs_stmp1_ir2_opnd;
        IR2_OPND count = latxs_stmp2_ir2_opnd;

        tb->tb_exec_nr = 0;

        latxs_load_imm64(&tbbase, (int64_t)tb);
        latxs_append_ir2_opnd2i(LISA_LD_D, &count, &tbbase,
                offsetof(TranslationBlock, tb_exec_nr));
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &count, &count, 1);
        latxs_append_ir2_opnd2i(LISA_ST_D, &count, &tbbase,
                offsetof(TranslationBlock, tb_exec_nr));
    }
}

void latxs_tracecc_tb_link(TranslationBlock *tb, int n, TranslationBlock *ntb)
{
    if (!tracecc_has_tb_link()) {
        return;
    }

    fprintf(stderr, "[CC] LINK %p %d %p %p %p\n",
            (void *)tb, n, (void *)ntb,
            tb->tc.ptr, ntb->tc.ptr);
}

#else

int tracecc_has_tb_tr(void)          { return 0; }
int tracecc_has_tb_exec(void)        { return 0; }
int tracecc_has_tb_inv(void)         { return 0; }
int tracecc_has_tb_flush(void)       { return 0; }
int tracecc_has_tb_flush_print(void) { return 0; }
int tracecc_has_tb_link(void)        { return 0; }

void latxs_tracecc_gen_tb_insert(TranslationBlock *tb, uint64_t p1, uint64_t p2, int exist) {}
void latxs_tracecc_gen_tb_start(void) {}
void latxs_tracecc_before_exec_tb(CPUX86State *env, TranslationBlock *tb) {}
void latxs_tracecc_target_to_host(CPUX86State *env, TranslationBlock *tb) {}
void latxs_tracecc_do_tb_flush(void) {}
void latxs_tracecc_tb_inv(TranslationBlock *tb) {}
void latxs_tracecc_tb_link(TranslationBlock *tb, int n, TranslationBlock *ntb) {}

#endif
