#include "lsenv.h"
#include "latx-config.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "qemu/timer.h"
#include "trace.h"
#include "latx-sigint-sys.h"
#include "latx-sigint-fn-sys.h"
#include "latx-intb-sys.h"

#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#include <signal.h>
#include <ucontext.h>

#define SIGINT_LINK_LOCK

static QemuSpin latx_sigint_slk;
#ifdef SIGINT_LINK_LOCK
static QemuSpin latx_sigint_link_lock;
#endif

static
void __attribute__((__constructor__)) latx_sigint_init_lock(void)
{
    qemu_spin_init(&latx_sigint_slk);
#ifdef SIGINT_LINK_LOCK
    qemu_spin_init(&latx_sigint_link_lock);
#endif
}

#include "sigint-i386-tcg-la.h"

/*#define SIGINT_DO_TRACE*/

#ifdef SIGINT_DO_TRACE
#define LATX_TRACE(name, ...) do {                  \
    trace_latx_sigint_##name(                       \
            pthread_self(), get_clock()             \
           , __VA_ARGS__);                          \
} while (0)
#else /* no SIGINT_DO_TRACE */
#define LATX_TRACE(name, ...)
#endif /* SIGINT_DO_TRACE */

static uint64_t code_buffer_lo;
static uint64_t code_buffer_hi;

int sigint_enabled(void)
{
    return option_sigint ? 1 : 0;
    /* return 1 : sigint */
    /* return 2 : old tcg sigint */
}

static void latxsigint_unlink_tb_recursive(TranslationBlock *tb);

static
void latxsigint_unlink_tb_recursive_2(TranslationBlock *tb, int n)
{
    TranslationBlock *tb_next =
        (TranslationBlock *)tb->jmp_dest[n];

    if (tb_next) {
        tcgsigint_remove_tb_from_jmp_list(tb, n);
        tb->jmp_dest[n] = (uintptr_t)NULL;

        /* reset tb's jump link */
        if (tb->jmp_reset_offset[n] !=
                TB_JMP_RESET_OFFSET_INVALID)  {
            uintptr_t addr =
                (uintptr_t)(tb->tc.ptr +
                            tb->jmp_reset_offset[n]);
            tb_set_jmp_target(tb, n, addr);
        }

        latxsigint_unlink_tb_recursive(tb_next);
    }
}

static
void latxsigint_unlink_tb_recursive(TranslationBlock *tb)
{
    latxsigint_unlink_tb_recursive_2(tb, 0);
    latxsigint_unlink_tb_recursive_2(tb, 1);
}

static
void latxsigint_unlink_tb_all(CPUX86State *env)
{
    lsassertm(sigint_enabled() == 2,
            "SIGINT type is not 2\n");

    TranslationBlock *tb;

    qemu_spin_lock(&latx_sigint_slk);

    tb = env->latxs_int_tb;
    if (tb) {
        env->latxs_int_tb = NULL;
        latxsigint_unlink_tb_recursive(tb);
    }

    qemu_spin_unlock(&latx_sigint_slk);
}

/* no relink for direct jmp */
//#define SIGINT_NO_RELINK

static
void latxs_sigint_block_signal(void)
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, 63);
    sigprocmask(SIG_BLOCK,&sigset,NULL);
}

static
void latxs_sigint_unblock_signal(void)
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, 63);
    sigprocmask(SIG_UNBLOCK,&sigset,NULL);
}

void latxs_sigint_cpu_id(CPUState *cpu)
{
    static int cpu_id = 0;
    qemu_spin_lock(&latx_sigint_slk);
    cpu->sigint_id = cpu_id;
    cpu_id += 1;
    qemu_spin_unlock(&latx_sigint_slk);
}

static void __latxs_tb_unlink(TranslationBlock *ctb)
{
    LATX_TRACE(unlink, ctb->pc, ctb->is_indir_tb);

    uintptr_t addr = 0;

    if (ctb->jmp_reset_offset[0] != TB_JMP_RESET_OFFSET_INVALID) {
        addr = (uintptr_t)(ctb->tc.ptr + ctb->jmp_reset_offset[0]);
        tb_set_jmp_target(ctb, 0, addr);
#ifdef SIGINT_NO_RELINK
        if (!ctb->is_indir_tb) {
            tcgsigint_remove_tb_from_jmp_list(ctb, 0);
            qatomic_set(&ctb->jmp_dest[0], (uintptr_t)NULL);
        }
#endif
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
#ifdef SIGINT_NO_RELINK
        if (!ctb->is_indir_tb) {
            tcgsigint_remove_tb_from_jmp_list(ctb, 1);
            qatomic_set(&ctb->jmp_dest[1], (uintptr_t)NULL);
        }
#endif
    }
}

void latxs_tb_unlink(TranslationBlock *ctb)
{
    if (sigint_enabled() != 1) return;
    if (!ctb) return;

    latxs_sigint_block_signal();
#ifdef SIGINT_LINK_LOCK
    qemu_spin_lock(&latx_sigint_link_lock);
    
    int sid = current_cpu->sigint_id;
    int i = 0;

    for (i = 0; i < 4; ++i) {
        /* check this thread */
        if (i == sid) {
            if (ctb->sigint_link_flag[sid] == -1) {
                /* mark this thread unlink this TB */
                ctb->sigint_link_flag[sid] = sid;
            } else {
                lsassertm(ctb->sigint_link_flag[sid] == sid,
                        "unexpected sigint %d\n", sid);
                /* this TB is already unlinked by this thread */
                goto unlink_done;
            }
            continue;
        }
        /* check other thread */
        if (ctb->sigint_link_flag[sid] != -1) {
            /* this TB is already unlinked by other thread */
            goto unlink_done;
        }
    }
#endif

    __latxs_tb_unlink(ctb);

#ifdef SIGINT_LINK_LOCK
unlink_done:
    qemu_spin_unlock(&latx_sigint_link_lock);
#endif
    latxs_sigint_unblock_signal();
}

static void __latxs_tb_relink(TranslationBlock *utb)
{
    LATX_TRACE(relink, utb->pc, utb->is_indir_tb);

#ifdef LATXS_INTB_LINK_ENABLE
    if (utb->is_indir_tb) {
        tb_set_jmp_target(utb, 0, GET_SC_TABLE(utb->region_id, intb_lookup));
        return;
    }
#endif

#ifdef SIGINT_NO_RELINK
    return;
#endif

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

void latxs_tb_relink(TranslationBlock *utb)
{
    if (sigint_enabled() != 1) return;

    if (!utb) {
        return;
    }

    latxs_sigint_block_signal();
#ifdef SIGINT_LINK_LOCK
    qemu_spin_lock(&latx_sigint_link_lock);

    int sid = current_cpu->sigint_id;
    int i = 0;
    for (; i < 4; ++i) {
        if (i == sid) {
            continue;
        }
        if (utb->sigint_link_flag[i] != -1) {
            /* Other thread also unink it */
            goto relink_done;
        }
    }
    utb->sigint_link_flag[sid] = -1;
#endif

    __latxs_tb_relink(utb);

relink_done:
#ifdef SIGINT_LINK_LOCK
    qemu_spin_unlock(&latx_sigint_link_lock);
#endif
    latxs_sigint_unblock_signal();
}

void latxs_rr_interrupt_self(CPUState *cpu)
{
    if (!sigint_enabled()) return;

    if (sigint_enabled() == 2) {
        latxsigint_unlink_tb_all(lsenv->cpu_state);
        return;
    }

    CPUX86State *env = cpu->env_ptr;

    TranslationBlock *ctb = NULL;
    TranslationBlock *otb = NULL;

    if (env->sigint_flag) {
        LATX_TRACE(event, "noexectb", 0, __func__);
        return;
    }

    ctb = env->latxs_int_tb;
    LATX_TRACE(event, "tbinenv", ctb ? ctb->pc : 0, __func__);

    otb = lsenv->sigint_data.tb_unlinked;
    if (otb == ctb && !ctb) {
        LATX_TRACE(event, "alreadyunlinkedtb", ctb ? ctb->pc : 0, __func__);
        return;
    } else {
        LATX_TRACE(event, "relinkoldtb", otb ? otb->pc : 0, __func__);
        latxs_tb_relink(otb);
    }

    lsenv->sigint_data.tb_unlinked = ctb;
    latxs_tb_unlink(ctb);
}

static int sigint_check_intb_lookup_st;
static int sigint_check_intb_lookup_ed;

static ADDR intb_lookup_sigint_check_st;
static ADDR intb_lookup_sigint_check_ed;

//#define LATXS_SIGINT_EXTRA_OVERHEAD

#ifdef LATXS_SIGINT_EXTRA_OVERHEAD
static int res[64];
static
int __attribute__((noinline)) signal_handler_extra_overhead(int asd)
{
    int qwe = 2;
    /*
     * ----------------- loop 100
     *  2740 ns 3670 ns
     * ----------------- loop 1,000
     *  5560 ns 5500 ns
     * ----------------- loop 2,000
     *  7660 ns 8260 ns 7830 ns
     * ----------------- loop   4,000
     * 13070 ns 12530 ns 12610 ns
     * ----------------- loop   8,000
     * 22210 ns 22180 ns
     * ----------------- loop  16,000
     * 41330 ns
     */
    int loop = 128000;
    int i = 1;
    for (; i < loop; ++i) {
        qwe = qwe * i + res[i & 0x3f];
        res[i & 0x3f] = asd;
    }
    return qwe;
//    struct timespec ts, ts1;
//    ts.tv_sec  = 0;
//    ts.tv_nsec = ns;
//    nanosleep(&ts, &ts1);
//    return ts1.tv_nsec;
}

static int avg_eo;
static int avg_eo_nr;

static
void __attribute__((__constructor__)) latx_sigint_extra_overhead_test(void)
{
    int64_t st, ed;
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    st = ts.tv_sec * 1000000000LL + ts.tv_nsec;

    res[1] = res[0];
    int r = signal_handler_extra_overhead(1);
    res[0] = r;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    ed = ts.tv_sec * 1000000000LL + ts.tv_nsec;

    printf("SIGINT extra overhead test %ld ns\n", ed - st);

    avg_eo = ed - st;
    avg_eo_nr = 1;
}
#endif

void latxs_sigint_check_in_hamt(CPUX86State *env, void *epc)
{
    if (sigint_enabled() != 1) {
        return;
    }

    /* we must be executing TBs */
    lsassertm(env->sigint_flag == 0,
            "SIGINT die in %s\n", __func__);

    CPUState *cpu = env_cpu(env);
    int f = (int16_t)qatomic_read(&cpu->icount_decr_ptr->u16.high);
    if (f < 0) {
        TranslationBlock *ltb = lsenv->sigint_data.tb_unlinked;
        if (ltb &&
            ((uint64_t)epc) >= (uint64_t)(ltb->tc.ptr) &&
            ((uint64_t)epc) <  (uint64_t)(ltb->tc.ptr + ltb->tc.size)) {
            /* this TB is already unlinked, no need to lookup */
            return;
        }
        TranslationBlock *ctb = tcg_tb_lookup((uintptr_t)epc);
        if (ctb) {
            /*fprintf(stderr, "SIGINT in hamt %p\n", epc);*/
            if (ltb) {
                if (ltb != ctb) {
                    /*fprintf(stderr, "SIGINT in hamt relink last Tb %x\n", ltb->pc);*/
                    latxs_tb_relink(ltb);
                } else {
                    lsassertm(0, "SIGINT ltb == ctb\n");
                }
           }
            lsenv->sigint_data.tb_unlinked = ctb;
            latxs_tb_unlink(ctb);
        } else {
            lsassertm(0, "SIGINT no TB in %s\n", __func__);
        }
    }
}

static void latxs_rr_interrupt_signal_handler(
        int n, siginfo_t *siginfo, void *ctx)
{
    if (!sigint_enabled()) {
        return;
    }

    if (sigint_enabled() == 2) {
        latxsigint_unlink_tb_all(lsenv->cpu_state);
        return;
    }

    lsassertm(sigint_enabled() == 1,
            "SIGINT mode wrong %s\n", __func__);

    ucontext_t *uc = ctx;

    uintptr_t pc = (uintptr_t)uc->uc_mcontext.__pc;

#ifdef LATXS_SIGINT_EXTRA_OVERHEAD
    struct timespec ts;
    int64_t st, ed;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    st = ts.tv_sec * 1000000000LL + ts.tv_nsec;
    res[1] = res[0];
    int r = signal_handler_extra_overhead(pc);
    res[0] = r;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ed = ts.tv_sec * 1000000000LL + ts.tv_nsec;
    avg_eo = (int)((double)avg_eo * ((double)avg_eo_nr / (double)(avg_eo_nr + 1)) + (double)(ed - st) / (double)(avg_eo_nr + 1));
    avg_eo_nr += 1;
#endif

    LATX_TRACE(handler, pc);

    CPUX86State *env = lsenv->cpu_state;
    TranslationBlock *ctb = NULL;
    TranslationBlock *oldtb = NULL;

    /* signal comes inside hamt exception */
    if (env->sigint_hamt_flag == 1) {
        /* we must be executing TBs */
        lsassertm(env->sigint_flag == 0, "SIGINT die %s\n", __func__);
        /* just do nothing here. */
        /* interrupt will be checked befoer hamt return */
        return;
    }

    if (code_buffer_lo <= pc && pc <= code_buffer_hi) {
        ctb = tcg_tb_lookup(pc);

        if (ctb == NULL) {
            ctb = env->latxs_int_tb;
            LATX_TRACE(event, "tbinenv", ctb ? ctb->pc : 0, __func__);
#ifdef LATXS_INTB_LINK_ENABLE
            if (intb_lookup_sigint_check_st <= pc &&
                intb_lookup_sigint_check_ed >= pc) {
                fprintf(stderr, "[warning] in %s unhandled case\n",
                        __func__);
            }
#endif
        } else {
            LATX_TRACE(event, "tcglookuptb", ctb ? ctb->pc : 0, __func__);
        }

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
            LATX_TRACE(event, "noexectb", 0, __func__);
            return;
        }

        ctb = env->latxs_int_tb;
        LATX_TRACE(event, "tbinenv", ctb ? ctb->pc : 0, __func__);
    }

    oldtb = lsenv->sigint_data.tb_unlinked;
    if (oldtb == ctb) {
        /* This TB is already unlinked */
        LATX_TRACE(event, "alreadyunlinkedtb", ctb ? ctb->pc : 0, __func__);
        return;
    } else {
        /* Prev TB is unlinked and not relinked */
        LATX_TRACE(event, "relinkoldtb", oldtb ? oldtb->pc : 0, __func__);
        latxs_tb_relink(oldtb);
    }

    lsenv->sigint_data.tb_unlinked = ctb;
    latxs_tb_unlink(ctb);
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

    if (sigint_enabled() == 1) {
        /* 3. set the range of code buffer */
        code_buffer_lo = (uint64_t)tcg_ctx->code_gen_buffer;
        code_buffer_hi = (uint64_t)tcg_ctx->code_gen_buffer +
                         (uint64_t)tcg_ctx->code_gen_buffer_size;
        fprintf(stderr, "[SIGINT] monitor code buffer %llx to %llx\n",
                (unsigned long long)code_buffer_lo,
                (unsigned long long)code_buffer_hi);
    }
}

void latxs_tr_gen_save_currtb_for_int(void)
{
    if (sigint_enabled() != 1) return;

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

#ifdef LATXS_INTB_LINK_ENABLE

void latxs_sigint_prepare_check_intb_lookup(
        IR2_OPND lst, IR2_OPND led)
{
    if (sigint_enabled() != 1) return;

    int code_nr = 0;

    int st_id = latxs_ir2_opnd_label_id(&lst);
    int ed_id = latxs_ir2_opnd_label_id(&led);

    int lid = 0;

    TRANSLATION_DATA *td = lsenv->tr_data;
    int rid = td->region_id;
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    IR2_INST *pir2 = NULL;

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

    int ir2_id = 0;
    int ir2_nr = td->ir2_cur_nr;
    for (ir2_id = 0; ir2_id < ir2_nr; ++ir2_id) {
        pir2 = ir2_get(ir2_id);

        IR2_OPCODE opc = ir2_opcode(pir2);

        if (opc == LISA_X86_INST) {
            continue;
        }

        if (opc == LISA_LABEL) {
            lid = latxs_ir2_opnd_label_id(&pir2->_opnd[0]);
            if (lid == st_id) {
                sigint_check_intb_lookup_st = code_nr;
            }
            if (lid == ed_id) {
                sigint_check_intb_lookup_ed = code_nr - 1;
            }

            continue;
        }

        code_nr += 1;
    }

    intb_lookup_sigint_check_st = GET_SC_TABLE(rid, intb_lookup) +
        (sigint_check_intb_lookup_st << 2);
    intb_lookup_sigint_check_ed = GET_SC_TABLE(rid, intb_lookup) +
        (sigint_check_intb_lookup_ed << 2);

    fprintf(stderr, "[SIGINT] intb lookup check start %d at %llx\n",
            sigint_check_intb_lookup_st,
            (unsigned long long)intb_lookup_sigint_check_st);
    fprintf(stderr, "[SIGINT] intb lookup  check end   %d at %llx\n",
            sigint_check_intb_lookup_ed,
            (unsigned long long)intb_lookup_sigint_check_ed);
}

#else

void latxs_sigint_prepare_check_intb_lookup(IR2_OPND s, IR2_OPND e) {}

#endif
