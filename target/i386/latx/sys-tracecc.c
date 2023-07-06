#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "trace.h"
#include "latx-tracecc-sys.h"
#include "latx-tracecc-init.h"
#include "latxs-fastcs-cfg.h"
#include "latxs-cc-pro.h"

#define LATXS_TRACECC_TB_TR           (1 << 0)
#define LATXS_TRACECC_TB_EXEC         (1 << 1)
#define LATXS_TRACECC_TB_INV          (1 << 2)
#define LATXS_TRACECC_TB_FLUSH_FULL   (1 << 3)
#define LATXS_TRACECC_TB_FLUSH_FULL_PRINT  (1 << 4)
#define LATXS_TRACECC_TB_LINK         (1 << 5)
#define LATXS_TRACECC_TB_FLUSH_FIFO   (1 << 6)
#define LATXS_TRACECC_TB_FLUSH_FIFO_PRINT  (1 << 7)

#ifdef LATXS_TRACECC_ENABLE

static FILE *tracecc_log;

static
void __attribute__((__constructor__)) tracecc_log_init_static(void)
{
    tracecc_log = NULL;
}

void latx_tracecc_log_init(bool enable)
{
    char tracecc_file[64];

    if (enable) {
        sprintf(tracecc_file, "/tmp/tracecc-%d.log", getpid());
        tracecc_log = fopen(tracecc_file, "w");
        if (tracecc_log == NULL) {
            fprintf(stderr, "tracecc log open fail. output to stderr.\n");
        }
    } else {
        tracecc_log = NULL;
    }
}

static
void latx_tracecc_flush(void)
{
    if (tracecc_log) {
        fflush(tracecc_log);
    }
}

#define TRACECC_RECORD(format, ...) do {            \
    if (tracecc_log) {                              \
        fprintf(tracecc_log, format, __VA_ARGS__);  \
    } else {                                        \
        fprintf(stderr, format, __VA_ARGS__);       \
    }                                               \
} while (0)

#define TRACECC_RECORD_STR(str) do {        \
    if (tracecc_log) {                      \
        fprintf(tracecc_log, "%s", str);    \
    } else {                                \
        fprintf(stderr, "%s", str);         \
    }                                       \
} while (0)

#define TRACECC_RECORD_ENDLINE() do {   \
    if (tracecc_log) {                  \
        fprintf(tracecc_log, "\n");     \
        latx_tracecc_flush();           \
    } else {                            \
        fprintf(stderr, "\n");          \
    }                                   \
} while (0)

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

int tracecc_has_tb_flush_full(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_FLUSH_FULL;
}

int tracecc_has_tb_flush_full_print(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_FLUSH_FULL_PRINT;
}

int tracecc_has_tb_flush_fifo(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_FLUSH_FIFO;
}

int tracecc_has_tb_flush_fifo_print(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_FLUSH_FIFO_PRINT;
}

int tracecc_has_tb_link(void)
{
    return option_trace_code_cache & LATXS_TRACECC_TB_LINK;
}

void __latxs_tracecc_gen_tb_insert(TranslationBlock *tb,
        uint64_t p1, uint64_t p2, int exist)
{
    if (!tracecc_has_tb_tr()) {
        return;
    }

    TRACECC_RECORD("Insert %p %p %p %d 0x%lx",
            (void *)tb, (void *)p1, (void *)p2, exist,
            tb->tc.size);
    TRACECC_RECORD_ENDLINE();
}

void __latxs_tracecc_target_to_host(
        CPUX86State *env, TranslationBlock *tb)
{
    if (!tracecc_has_tb_tr()) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    IR1_INST *ir1_list = td->ir1_inst_array;
    int ir1_nr = td->ir1_nr;

    uint8_t fcs = (latxs_fastcs_enable_tbctx()) ? tb->fastcs_ctx : -1;
    TRACECC_RECORD("TR %p %x %x %x %d %p %x %x %d ",
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
            TRACECC_RECORD("%02x", pir1->info->bytes[j]);
        }
    }
    TRACECC_RECORD_ENDLINE();
}

static int tb_trace_cc_done(TranslationBlock *tb)
{
    return tb->trace_cc & 0x1;
}

void __latxs_tracecc_before_exec_tb(
        CPUX86State *env, TranslationBlock *tb)
{
    if (!tracecc_has_tb_exec() || tb_trace_cc_done(tb)) {
        return;
    }
    uint8_t fcs = (latxs_fastcs_enable_tbctx()) ? tb->fastcs_ctx : -1;
    TRACECC_RECORD("EXEC %p %x %x %x %x %d %p %lx %x %x %x %x",
            (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)env->cr[3],
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1], fcs);
    TRACECC_RECORD_ENDLINE();
    tb->trace_cc |= 0x1;
}

static gboolean latxs_tracecc_tb(
        gpointer key, gpointer value, gpointer data)
{
    const TranslationBlock *tb = value;
    int *flushnr = data;

    uint8_t fcs = (latxs_fastcs_enable_tbctx()) ? tb->fastcs_ctx : -1;
    TRACECC_RECORD("FLUSH %d %p %x %x %x %x %d %p %lx %x %x %ld %x %d",
            *flushnr, (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1],
            (uint64_t)tb->tb_exec_nr,
            fcs, tb->cc_flags);
    TRACECC_RECORD_ENDLINE();

    return false;
}

void __latxs_tracecc_do_tb_flush_full(void)
{
    if (tracecc_has_tb_flush_full()) {
        TRACECC_RECORD_STR("[CC] TB FULL FLUSH");
        TRACECC_RECORD_ENDLINE();
    }

    if (tracecc_has_tb_flush_full_print()) {
        tcg_tb_foreach(latxs_tracecc_tb, &tb_ctx.tb_flush_count);
    }
}

void __latxs_tracecc_do_tb_flush_fifo(int rid, int tid)
{
    if (tracecc_has_tb_flush_fifo()) {
        TRACECC_RECORD_STR("[CC] TB FIFO FLUSH");
        TRACECC_RECORD_ENDLINE();
    }

    if (tracecc_has_tb_flush_fifo_print()) {
        tcg_tb_foreach_region_tree(rid, tid,
                latxs_tracecc_tb, &tb_ctx.tb_flush_count);
    }
}

void __latxs_tracecc_tb_inv(TranslationBlock *tb)
{
    if (!tracecc_has_tb_inv()) {
        return;
    }

    TRACECC_RECORD("INV %p %x %x %x %x %d %p %lx %x %x", (void *)tb,
            tb->flags, tb->cflags,
            (int)tb->pc, tb->size, tb->icount,
            (void *)tb->tc.ptr, tb->tc.size,
            (uint32_t)tb->page_addr[0],
            (uint32_t)tb->page_addr[1]);
    TRACECC_RECORD_ENDLINE();
}

void __latxs_tracecc_gen_tb_start(void)
{
    if (tracecc_has_tb_flush_full_print()) {
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

void __latxs_tracecc_tb_link(TranslationBlock *tb, int n, TranslationBlock *ntb)
{
    if (!tracecc_has_tb_link()) {
        return;
    }

    TRACECC_RECORD("LINK %p %d %p %p %p\n",
            (void *)tb, n, (void *)ntb,
            tb->tc.ptr, ntb->tc.ptr);
    TRACECC_RECORD_ENDLINE();
}

#else

void latx_tracecc_log_init(bool enable) {}

#endif
