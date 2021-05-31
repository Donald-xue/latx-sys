#include "common.h"
#include "diStorm/distorm.h"
#include "ir1.h"
#include "ir2.h"
#include "lsenv.h"
#include "etb.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "etb.h"
#include "ibtc.h"
#include "shadow-stack.h"
#include "flag-reduction.h"
#include "profile.h"
#include "trace.h"
#include "translate.h"
#include "latx-config.h"

int target_latx_host(CPUArchState *env, struct TranslationBlock *tb)
{
    counter_tb_tr += 1;

    trace_xtm_tr_tb((void *)tb, (void *)tb->tc.ptr,
                    (void *)(unsigned long long)tb->pc);

    if (option_dump) {
        fprintf(stderr, "=====================================\n");
        fprintf(stderr, "|| TB translation : %14p ||\n", tb);
        fprintf(stderr, "=====================================\n");
        fprintf(stderr, "Guest Base = 0x%lx\n", (unsigned long)guest_base);
        fprintf(stderr, "=====================================\n");
    }

    /* target => IR1
     * IR1 stored in lsenv->tr_data
     */
    void *etb = tr_disasm(tb);

    if (option_flag_reduction) {
        tb_flag_reduction((void *)tb);
    } else {
        tb_flag((void *)tb);
    }

    if (!option_lsfpu) {
        etb_check_top_in(tb, env->fpstt);
    }

    /* IR1 => IR2 => host
     * IR2 stored in lsenv->tr_data
     * host write into TB
     */
    return tr_translate_tb(tb, etb);
}

void trace_tb_execution(struct TranslationBlock *tb)
{
    lsassert(tb != NULL);

    if (!option_trace_tb && !option_trace_ir1) {
        return;
    }

    fprintf(stderr, "[trace] ========================\n");
    fprintf(stderr, "[trace] TB to execute\n");

    if (option_trace_tb) {
        fprintf(stderr, "[trace] ========================\n");
        fprintf(stderr, "[trace] TB      = %-18p , TB's address\n", (void *)tb);
        fprintf(stderr, "[trace] Counter = %-18lld , TB's execution\n",
                counter_tb_exec);
        fprintf(stderr, "[trace] Counter = %-18lld , TB's translation\n",
                counter_tb_tr);
        fprintf(stderr, "[trace] Counter = %-18lld , IR1 translated\n",
                counter_ir1_tr);
        fprintf(stderr, "[trace] Counter = %-18lld , MIPS generated\n",
                counter_mips_tr);
        fprintf(stderr, "[trace] PC      = %-18p , target's virtual address\n",
                (void *)(unsigned long)tb->pc);
        fprintf(stderr, "[trace] csbase  = %-18p , target's CS segment base\n",
                (void *)(unsigned long)tb->cs_base);
        fprintf(stderr, "[trace] size    = %-18ld , TB's target code size\n",
                (unsigned long)tb->size);
        fprintf(stderr, "[trace] tc.ptr  = %-18p , TB's host code size\n",
                (void *)tb->tc.ptr);
        fprintf(stderr, "[trace] tc.size = %-18ld , TB's host code size\n",
                (unsigned long)tb->tc.size);
        fprintf(stderr, "[trace] host nr = %-18ld , TB's host code number\n",
                (unsigned long)tb->tc.size / 4);
    }

    ETB *etb = &tb->extra_tb;

    IR1_INST *ir1_list = etb->_ir1_instructions;
    IR1_INST *pir1 = NULL;
    int ir1_nr = etb->_ir1_num;

    int i = 0;

    if (option_trace_ir1) {
        fprintf(stderr, "[trace] ========================\n");
        fprintf(stderr, "[trace] ir1_nr  = %-18ld , TB's IR1 code size\n",
                (unsigned long)ir1_nr);
        for (i = 0; i < ir1_nr; ++i) {
            pir1 = ir1_list + i;
            fprintf(stderr, "[trace] ");
            ir1_dump(pir1);
            fprintf(stderr, "\n");
        }
    }

    fprintf(stderr, "[trace] ========================\n");
}

/*
 * prologue <=> bt to native
 */
int target_latx_prologue(void *code_buf_addr)
{
    int code_nr = 0;

    lsassert(context_switch_bt_to_native == 0);
    context_switch_bt_to_native = (ADDR)code_buf_addr;

    if (option_dump)
        fprintf(stderr, "[LATX] context_switch_bt_to_native = %p\n",
                (void *)context_switch_bt_to_native);

    tr_init(NULL);
    generate_context_switch_bt_to_native(code_buf_addr);
    code_nr = tr_ir2_assemble((void *)context_switch_bt_to_native);
    tr_fini(true);

    return code_nr;
}

/*
 * epilogue <=> native to bt
 */
int target_latx_epilogue(void *code_buf_addr)
{
    int code_nr = 0;

    lsassert(context_switch_native_to_bt == 0);
    context_switch_native_to_bt_ret_0 = (ADDR)code_buf_addr;
    context_switch_native_to_bt = (ADDR)code_buf_addr + 4;

    if (option_dump)
        fprintf(stderr, "[LATX] context_switch_native_to_bt = %p\n",
                (void *)context_switch_native_to_bt);

    tr_init(NULL);
    generate_context_switch_native_to_bt();
    code_nr = tr_ir2_assemble((void *)context_switch_native_to_bt_ret_0);
    tr_fini(false);

    return code_nr;
}

void latx_before_exec_trace_tb(CPUArchState *env, struct TranslationBlock *tb)
{
    if (option_trace_tb)
        fprintf(stderr,
                "[LATX] before executing TB {PC = %p, code at %p}.\n",
                (void *)(unsigned long)tb->pc, (void *)tb->tc.ptr);
    counter_tb_exec += 1;
}

void latx_after_exec_trace_tb(CPUArchState *env, struct TranslationBlock *tb)
{
    if (option_trace_tb)
        fprintf(stderr,
                "[LATX] after  executing TB {PC = %p, code at %p}.\n",
                (void *)(unsigned long)tb->pc, (void *)tb->tc.ptr);
}

void latx_profile(void)
{
    if (option_profile)
        context_switch_time++;
}

void latx_before_exec_rotate_fpu(CPUArchState *env, struct TranslationBlock *tb)
{
    if (!option_lsfpu) {
        lsassert(lsenv_get_top_bias(lsenv) == 0);
        if (env->fpstt != etb_get_top_in(&tb->extra_tb)) {
            rotate_fpu_to_top(etb_get_top_in(&tb->extra_tb));
        }
    }
}

void latx_after_exec_rotate_fpu(CPUArchState *env, struct TranslationBlock *tb)
{
    if (!option_lsfpu) {
        /* if tb linked to other tbs, last executed tb might not be current tb
         * if last_executed_tb is null, it is not linked indirect jmps
         */
        TranslationBlock *last_executed_tb =
            (TranslationBlock *)(lsenv_get_last_executed_tb(lsenv));
#ifdef N64
        if(last_executed_tb) {
            last_executed_tb =(TranslationBlock*)((uintptr_t)last_executed_tb |\
                    ((uintptr_t)tb & 0xffffffff00000000)); 
            //lsenv_set_last_executed_tb(lsenv,(ADDR)last_executed_tb);
        }
#endif
        ETB *etb;
        if (last_executed_tb) {
            etb = &last_executed_tb->extra_tb;
        } else {
            etb = &tb->extra_tb;
        }
        env->fpstt = etb->_top_out;
        if (lsenv_get_top_bias(lsenv) != 0) {
            rotate_fpu_to_bias(0);
        }
    }
}
/*
 * native rotate fpu by
 */
int target_latx_fpu_rotate(void *code_buf_addr)
{
    return generate_native_rotate_fpu_by(code_buf_addr);
}

static void xtm_capstone_init(void)
{
    if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK) {
        fprintf(stderr, "%s %s %d error : cs_open \n", __FILE__, __func__,
                __LINE__);
        exit(-1);
    }
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
}

static QHT etb_qht_real;

/* defined in etb.c */
extern QHT *etb_qht;
static void __attribute__((__constructor__)) latx_init(void)
{
    context_switch_bt_to_native = 0;
    context_switch_native_to_bt_ret_0 = 0;
    context_switch_native_to_bt = 0;
    /* context_switch_is_init = 0; */
    native_rotate_fpu_by = 0;

    options_init();
    xtm_capstone_init();

    etb_qht = &etb_qht_real;
    etb_qht_init();
    ss_init(&shadow_stack);
}

void latx_exit(void)
{
    if (option_profile) {
        profile_generate();
        profile_dump(10);
    }
}

static __thread ENV lsenv_real;
static __thread TRANSLATION_DATA tr_data_real;
static __thread FLAG_PATTERN_DATA fp_data_real;

/* global lsenv defined here */
__thread ENV *lsenv;

void latx_lsenv_init(CPUArchState *env)
{
    lsenv = &lsenv_real;
    lsenv->cpu_state = env;
    lsenv->tr_data = &tr_data_real;
    lsenv->fp_data = &fp_data_real;

    env->vregs[4] = (uint64_t)shadow_stack._ssi_current;
    if (option_dump) {
        fprintf(stderr, "[LATX] env init : %p\n", lsenv->cpu_state);
    }
}

void latx_set_tls_ibtc_table(CPUArchState *env)
{
    env->ibtc_table_p = &ibtc_table;
}
