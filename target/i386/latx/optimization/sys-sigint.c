#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

#include <signal.h>
#include <ucontext.h>

/* #define LATXS_DEBUG_SIGINT */
#ifdef LATXS_DEBUG_SIGINT
#define DS_PRINT(...) do {          \
    fprintf(stderr, __VA_ARGS__); \
} while (0)

#else
#define DS_PRINT(...)
#endif

static uint64_t code_buffer_lo;
static uint64_t code_buffer_hi;

int sigint_enabled(void)
{
    return option_sigint;
}

void latxs_tb_unlink(TranslationBlock *ctb)
{
    if (!ctb) {
        return;
    }

    uintptr_t addr = 0;

    if (ctb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
        addr = (uintptr_t)(ctb->tc.ptr + ctb->jmp_reset_offset[0]);
        tb_set_jmp_target(ctb, 0, addr);
    }

    /* if (xtm_branch_opt() && TODO */
            /* ctb->extra_tb->branch_to_target_direct_in_mips_branch */
                /* == 1) { */
        /* uint32_t *addr = */
            /* (uint32_t *)(ctb->tc.ptr + */
                    /* ctb->extra_tb->mips_branch_inst_offset); */
        /* *addr = ctb->extra_tb->mips_branch_backup; */
        /* ctb->extra_tb->mips_branch_backup = 0; */
        /* ctb->extra_tb->branch_to_target_direct_in_mips_branch = 0; */
        /* flush_icache_range((uintptr_t)addr, (uintptr_t)addr + 4); */
        /* return; */
    /* } */

    if (ctb->jmp_reset_offset[1] != TB_JMP_RESET_OFFSET_INVALID) {
        addr = (uintptr_t)(ctb->tc.ptr + ctb->jmp_reset_offset[1]);
        tb_set_jmp_target(ctb, 1, addr);
    }
}

void latxs_tb_relink(TranslationBlock *utb)
{
    if (!utb) {
        return;
    }

    if (utb->is_indir_tb) {
        tb_set_jmp_target(utb, 0, native_jmp_glue_2);
        return;
    }

    TranslationBlock *utb_next = NULL;

    utb_next = utb->next_tb[0];
    if (utb_next && !(tb_cflags(utb_next) & CF_INVALID)) {
        latx_tb_set_jmp_target(utb, 0, utb_next);
    }

    utb_next = utb->next_tb[1];
    if (utb_next && !(tb_cflags(utb_next) & CF_INVALID)) {
        latx_tb_set_jmp_target(utb, 1, utb_next);
    }
}

static int sigint_check_jmp_glue_2_st;
static int sigint_check_jmp_glue_2_ed;

static ADDR native_jmp_glue_2_sigint_check_st;
static ADDR native_jmp_glue_2_sigint_check_ed;

static void latxs_rr_interrupt_signal_handler(
        int n, siginfo_t *siginfo, void *ctx)
{
    if (!sigint_enabled()) {
        return;
    }

    ucontext_t *uc = ctx;

    uintptr_t pc = (uintptr_t)uc->uc_mcontext.__pc;
    DS_PRINT("sigint, %llx, ", (unsigned long long)pc);

    CPUX86State *env = lsenv->cpu_state;

    if (code_buffer_lo <= pc && pc <= code_buffer_hi) {
        DS_PRINT("1, ");
        TranslationBlock *ctb = tcg_tb_lookup(pc);

        if (ctb == NULL) {
            ctb = env->latxs_int_tb;
            DS_PRINT("0\n");
            if (native_jmp_glue_2_sigint_check_st <= pc &&
                native_jmp_glue_2_sigint_check_ed >= pc) {
                fprintf(stderr, "[warning] in %s unhandled case\n",
                        __func__);
            }
        } else {
            DS_PRINT("1\n");
        }

        TranslationBlock *oldtb = lsenv->sigint_data.tb_unlinked;
        if (oldtb == ctb) {
            /* This TB is already unlinked */
            return;
        } else {
            /* Prev TB is unlinked and not relinked */
            latxs_tb_relink(oldtb);
        }

        lsenv->sigint_data.tb_unlinked = ctb;
        latxs_tb_unlink(ctb);
    } else {

        if (env->sigint_flag) {
            /*
             * vCPU is not executing TB
             *
             * Since we always check interrupt in context switch
             * bt to native before jump to TB, then we could
             * simply do nothing.
             *
             * And at this time, do unlink is meanless too.
             */
            return;
        }

        TranslationBlock *ctb = env->latxs_int_tb;
        DS_PRINT("0, 0\n");

        TranslationBlock *oldtb = lsenv->sigint_data.tb_unlinked;
        if (oldtb == ctb) {
            /* This TB is already unlinked */
            return;
        } else {
            /* Prev TB is unlinked and not relinked */
            latxs_tb_relink(oldtb);
        }

        lsenv->sigint_data.tb_unlinked = ctb;
        latxs_tb_unlink(ctb);
    }
}

void latxs_init_rr_thread_signal(CPUState *cpu)
{
    if (!sigint_enabled()) {
        return;
    }

    /* 1. unblock the signal for vCPU thread */
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, 63);

    int ret = pthread_sigmask(SIG_UNBLOCK, &set, &oldset);
    if (ret < 0) {
        fprintf(stderr, "[SIGINT] unblock SIG63 failed\n");
        exit(-1);
    }

    /* 2. set the handler for signal */

    struct sigaction act;
    struct sigaction old_act;

    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = latxs_rr_interrupt_signal_handler;

    ret = sigaction(63, &act, &old_act);
    if (ret < 0) {
        fprintf(stderr, "[SIGINT] set handler failed\n");
        exit(-1);
    }

    pthread_t tid = pthread_self();
    fprintf(stderr, "[SIGINT] thread %x set signal 63 handler\n",
            (unsigned int)tid);

    /* 3. set the range of code buffer */
    code_buffer_lo = (uint64_t)tcg_ctx->code_gen_buffer;
    code_buffer_hi = (uint64_t)tcg_ctx->code_gen_buffer +
                     (uint64_t)tcg_ctx->code_gen_buffer_size;

    fprintf(stderr, "[SIGINT] monitor code buffer %llx to %llx\n",
            (unsigned long long)code_buffer_lo,
            (unsigned long long)code_buffer_hi);
}

void latxs_tr_gen_save_currtb_for_int(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    void *tb = td->curr_tb;
    if (tb && td->need_save_currtb_for_int) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_load_addr_to_ir2(&tmp, (ADDR)tb);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp,
                &latxs_env_ir2_opnd,
                offsetof(CPUX86State, latxs_int_tb));
        latxs_ra_free_temp(&tmp);
    }
}

void latxs_sigint_prepare_check_jmp_glue_2(
        IR2_OPND lst, IR2_OPND led)
{
    int code_nr = 0;

    int st_id = latxs_ir2_opnd_label_id(&lst);
    int ed_id = latxs_ir2_opnd_label_id(&led);

    int lid = 0;

    TRANSLATION_DATA *td = lsenv->tr_data;

    IR2_INST *pir2 = td->ir2_inst_array;

    /*
     * example:
     *
     *   +-----> address = base
     *   |   +-> address = base + (1 << 2)
     *   |   |   ...
     *   0   1         2   3   4       5   6
     * +---+---+-----+---+---+---+---+---+---+
     * |IR2|IR2|start|IR2|IR2|IR2|end|IR2|IR2|
     * +---+---+-----+---+---+---+---+---+---+
     *          ^      |       |  ^
     *          label  |       |  label
     *                 |       |
     * start = 2       |       +--> address = base + (end   << 2)
     * end   = 4       +----------> address = base + (start << 2)
     */

    while (pir2) {
        IR2_OPCODE opc = latxs_ir2_opcode(pir2);

        if (opc == LISA_X86_INST) {
            pir2 = ir2_next(pir2);
            continue;
        }

        if (opc == LISA_LABEL) {
            lid = latxs_ir2_opnd_label_id(&pir2->_opnd[0]);
            if (lid == st_id) {
                sigint_check_jmp_glue_2_st = code_nr;
            }
            if (lid == ed_id) {
                sigint_check_jmp_glue_2_ed = code_nr - 1;
            }

            pir2 = ir2_next(pir2);
            continue;
        }

        code_nr += 1;
        pir2 = ir2_next(pir2);
    }

    native_jmp_glue_2_sigint_check_st = native_jmp_glue_2 +
        (sigint_check_jmp_glue_2_st << 2);
    native_jmp_glue_2_sigint_check_ed = native_jmp_glue_2 +
        (sigint_check_jmp_glue_2_ed << 2);

    fprintf(stderr, "[SIGINT] jmp glue 2 check start %d at %llx\n",
            sigint_check_jmp_glue_2_st,
            native_jmp_glue_2_sigint_check_st);
    fprintf(stderr, "[SIGINT] jmp glue 2 check end   %d at %llx\n",
            sigint_check_jmp_glue_2_ed,
            native_jmp_glue_2_sigint_check_ed);
}