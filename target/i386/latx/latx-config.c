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

#include "latx-hamt.h"
#include "info.h"

#include "latx-config.h"
#include "latx-tracecc-sys.h"
#include "latx-counter-sys.h"
#include "latx-bpc-sys.h"
#include "latx-np-sys.h"
#include "latx-tb-trace-sys.h"
#include "latx-sys-flag-rdtn.h"
#include "latx-sys-inst-ptn.h"
#include "latx-sigint-sys.h"
#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"
#include "latx-callret-func.h"

int target_latx_host(CPUArchState *env, struct TranslationBlock *tb)
{
    counter_tb_tr += 1;

    trace_xtm_tr_tb((void *)tb, (void *)tb->tc.ptr,
                    (void *)(unsigned long long)tb->pc);

    if (option_dump && qemu_log_in_addr_range(tb->pc)) {
        fprintf(stderr, "=====================================\n");
        fprintf(stderr, "|| TB translation : %14p ||\n", tb);
        fprintf(stderr, "=====================================\n");
#ifndef CONFIG_SOFTMMU
        fprintf(stderr, "Guest Base = 0x%lx\n", (unsigned long)guest_base);
        fprintf(stderr, "=====================================\n");
#endif
    }

    /* target => IR1
     * IR1 stored in lsenv->tr_data
     */
    tr_disasm(tb);

#ifdef CONFIG_LATX_FLAG_REDUCTION
    if (option_flag_reduction)
        tb_flag_reduction((void *)tb);
    else
#endif
        tb_flag(tb);

    if (!option_lsfpu) {
        etb_check_top_in(tb, env->fpstt);
    }

    /* IR1 => IR2 => host
     * IR2 stored in lsenv->tr_data
     * host write into TB
     */
    return tr_translate_tb(tb);
}

#ifdef CONFIG_LATX_DEBUG
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


    IR1_INST *ir1_list = tb->_ir1_instructions;
    IR1_INST *pir1 = NULL;
    int ir1_nr = tb->icount;

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
#endif

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

#ifdef CONFIG_LATX_DEBUG
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

#endif

void latx_before_exec_rotate_fpu(CPUArchState *env, struct TranslationBlock *tb)
{
    if (!option_lsfpu) {
        lsassert(lsenv_get_top_bias(lsenv) == 0);
        if (env->fpstt != etb_get_top_in(tb)) {
            rotate_fpu_to_top(etb_get_top_in(tb));
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
        if (last_executed_tb) {
            env->fpstt = last_executed_tb->_top_out;
        } else {
            env->fpstt = tb->_top_out;
        }
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
#ifdef CONFIG_SOFTMMU
    if (cs_open(CS_ARCH_X86, CS_MODE_16, &handle16) != CS_ERR_OK) {
        fprintf(stderr, "%s %s %d error : cs_open\n",
                __FILE__, __func__, __LINE__);
        exit(-1);
    }
    cs_option(handle16, CS_OPT_DETAIL, CS_OPT_ON);
#ifdef TARGET_X86_64
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle64) != CS_ERR_OK) {
        fprintf(stderr, "%s %s %d error : cs_open\n",
                __FILE__, __func__, __LINE__);
        exit(-1);
    }
    cs_option(handle64, CS_OPT_DETAIL, CS_OPT_ON);
#endif
#endif
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

static __thread ENV lsenv_real;
static __thread TRANSLATION_DATA tr_data_real;
#ifdef CONFIG_LATX_FLAG_PATTERN
static __thread FLAG_PATTERN_DATA fp_data_real;
#endif

/* global lsenv defined here */
__thread ENV *lsenv;

void latx_lsenv_init(CPUArchState *env)
{
    lsenv = &lsenv_real;
    lsenv->cpu_state = env;
    lsenv->tr_data = &tr_data_real;
#ifndef CONFIG_SOFTMMU
#ifdef CONFIG_LATX_FLAG_PATTERN
    lsenv->fp_data = &fp_data_real;
#endif

    env->vregs[4] = (uint64_t)shadow_stack._ssi_current;
    if (option_dump) {
        fprintf(stderr, "[LATX] env init : %p\n", lsenv->cpu_state);
    }
#else
    env->latxs_fpu = LATXS_FPU_RESET_VALUE;
    lsenv->sigint_data.tb_unlinked = NULL;

    latxs_fastcs_env_init(env);
    latxs_np_env_init(env);

    lsenv->after_exec_tb_fixed = 1;

    if (!lsenv->tr_data->slow_path_rcd) {
        lsenv->tr_data->slow_path_rcd_max = 4;
        lsenv->tr_data->slow_path_rcd_nr  = 0;
        lsenv->tr_data->slow_path_rcd =
            mm_calloc(4, sizeof(softmmu_sp_rcd_t));
    }
#endif
}

void latx_set_tls_ibtc_table(CPUArchState *env)
{
    env->ibtc_table_p = &ibtc_table;
}

#ifdef CONFIG_SOFTMMU

void latxs_lsenv_switch(CPUArchState *env)
{
    lsenv->cpu_state = env;

    env->latxs_fpu = LATXS_FPU_RESET_VALUE;
    lsassert(lsenv->sigint_data.tb_unlinked == NULL);

    latxs_fastcs_env_init(env);
    latxs_np_env_init(env);

    lsassert(lsenv->after_exec_tb_fixed == 1);
    lsassert(lsenv->tr_data->slow_path_rcd != NULL);
}

int target_latxs_host(CPUState *cpu, TranslationBlock *tb,
        int max_insns, void *code_highwater, int region_id,
        int *search_size)
{
    CPUArchState *env = cpu->env_ptr;

    latx_multi_region_switch(region_id);

    option_config_dump(tb->flags & 0x3);

    latxs_counter_tb_tr(cpu);

    latxs_break_point(env, tb);

    latxs_tr_sys_init(tb, max_insns, code_highwater);

    latxs_tr_disasm(tb);

    /* perform flag reduction OR just set flag */
    latxs_flag_reduction(tb);
    /* perform instruction pattern */
    latxs_inst_pattern(tb);

    if (!option_lsfpu && !option_soft_fpu) {
        tb->_top_in = env->fpstt & 0x7;
    }

    return latxs_tr_translate_tb(tb, search_size);
}

void latxs_before_exec_tb(CPUState *cpu, TranslationBlock *tb)
{
    CPUX86State *env = cpu->env_ptr;

    lsassertm(env->is_fcsr_simd == 0, "fcsr simd %s\n", __func__);
    env->is_fcsr_simd = 0;
    lsassertm(env->sigint_hamt_flag == 0, "sigint hamt wrong.\n");

    if (sigint_enabled() == 2) {
        env->latxs_int_tb = tb;
    }

    latx_multi_region_prepare_exec(tb->region_id);
    latxs_tracecc_before_exec_tb(env, tb);
    latxs_tb_trace(env, tb);
    latxs_break_point(env, tb);
    latxs_np_tb_print(env);
    latxs_counter_wake(cpu);

    latxs_clear_scr();

    if (!option_lsfpu && !option_soft_fpu) {
        lsassert(lsenv_get_top_bias(lsenv) == 0);
        latxs_fpu_fix_before_exec_tb(env, tb);
    }

    if (latxs_fastcs_enabled()) {
        env->fastcs_ctx = tb->fastcs_ctx;
    }

    lsenv->after_exec_tb_fixed = 0;
}

void latxs_after_exec_tb(CPUState *cpu, TranslationBlock *tb)
{
    CPUX86State *env = cpu->env_ptr;

    if (!option_lsfpu && !option_soft_fpu) {
        /*
         * if tb linked to other tbs, last executed tb might not be current tb
         * if last_executed_tb is null, it is not linked indirect jmps
         */
        /* TranslationBlock *last_tb = */
            /* (TranslationBlock *)(lsenv_get_last_executed_tb(lsenv)); */

        /* if(last_tb) { */
            /* last_tb = (TranslationBlock*)((uintptr_t)last_executed_tb |\ */
                    /* ((uintptr_t)tb & 0xffffffff00000000));  */
        /* } */

        /* latxs_fpu_fix_after_exec_tb(env, tb, last_tb); */
        latxs_fpu_fix_after_exec_tb(env, NULL);
    }

    lsenv->after_exec_tb_fixed = 1;

    if (sigint_enabled() == 1) {
        TranslationBlock *utb = lsenv->sigint_data.tb_unlinked;
        latxs_tb_relink(utb);
        lsenv->sigint_data.tb_unlinked = NULL;
    }

    if (sigint_enabled() == 2) {
        env->latxs_int_tb = NULL;
    }
}

/*
 * Because of exception or interrupt, latxs_after_exec_tb() will no
 * be executed.
 *
 * Here we use this function to finish some necessary job.
 *
 * Current implemented instructions need this fix :
 * ========================================================================
 *   > X86_INS_INT :
 *     helper_raise_interrupt() -> raise_interrupt()
 *     -> raise_interrupt2() -> cpu_loop_exit_restore() --> cpu_loop_exit()
 *   > All Exception Check :
 *     helper_raise_exception() -> raise_exception()
 *     -> raise_interrupt2() -> cpu_loop_exit_restore() --> cpu_loop_exit()
 *   > X86_INS_PAUSE : helper_pause() -> do_pause() ------> cpu_loop_exit()
 *   > X86_INS_HLT : helper_hlt() -> do_hlt() ------------> cpu_loop_exit()
 *   > ...
 * ========================================================================
 *
 * Other helpers might need this fix too :
 * ========================================================================
 *   > X86_INS_INTO : helper_into()
 *     -> raise_interrupt() -> raise_interrupt2() --------> cpu_loop_exit()
 *   > helpers in misc/seg_helper.c might call
 *     raise_exception_err() -> raise_interrupt2() -------> cpu_loop_exit()
 *   > helpers in bpt/excp/misc/seg_helper.c might call
 *     raise_exception_err_ra() -> raise_interrupt2() ----> cpu_loop_exit()
 *   > helpers in bpt/excp/svm_helper.c might call
 *     raise_exception() -> raise_interrupt2() -----------> cpu_loop_exit()
 *   > helpers in cc/fpu/int/mem/misc/mpx_helper.c might call
 *     raise_exception_ra() -> raise_interrupt2() --------> cpu_loop_exit()
 *   > helpers in svm/seg/misc_helper.c might call -------> cpu_loop_exit()
 *   > cpu_handle_exception() -> do_interrupt_all()
 *                            -> do_interrupt_protected/real()
 *                            -> raise_exception_err() ---> cpu_loop_exit()
 *   > all of the raise_interrupt2()
 *                -> cpu_svm_check_interrupt_param()
 *                -> cpu_vmexit() ------------------------> cpu_loop_exit()
 *   > all of the raise_interrupt2()
 *                -> check_exception() -> qemu_system_reset_request()
 *                   -> cpu_stop_current()
 *                   -> cpu_exit() : cpu->exit_request = 1
 *                -> cpu_loop_exit_restore() -------------> cpu_loop_exit()
 *   > helper_svm_check_interrupt_param() ----------------> cpu_loop_exit()
 *   > helper_svm_check_io() -> cpu_vmexit() -------------> cpu_loop_exit()
 * ========================================================================
 *
 * The execution might be :
 * ========================================================================
 * <1> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // no interrupt
 *     -> next TB
 * ========================================================================
 * <2> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // have interrupt
 *     -> x86_cpu_exec_interrupt()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <3> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // have hard interrupt
 *     -> x86_cpu_exec_interrupt()
 *     -> do_interrupt_x86_hardirq()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <4> native code -> context switch ---------> x86_to_mips_after_exec_tb()
 *     -> cpu_handle_interrupt()    // have hard interrupt
 *     -> x86_cpu_exec_interrupt()
 *     -> do_interrupt_x86_hardirq()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> raise_exception_err() --------------------------> cpu_loop_exit()
 *     -> cpu_hadnle_exception()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <5> native code -> helper ->
 *     raise_exception/interrupt() -----------------------> cpu_loop_exit()
 *     -> cpu_handle_exception()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> EIP updated -> new TB
 * ========================================================================
 * <6> native code -> helper ->
 *     raise_exception/interrupt() -----------------------> cpu_loop_exit()
 *     -> cpu_handle_exception()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> raise_exception_err()
 *     -> raise_interrupt2() -----------------------------> cpu_loop_exit()
 *     -> cpu_handle_exception()
 *     -> do_interrupt_all()
 *     -> do_interrupt_protected/real()
 *     -> EIP updated -> new TB
 * ========================================================================
 *
 * Plan 1: always do this in cpu_loop_exit()
 * Plan 2: do this fix in each possible execution
 *
 * Advantage of Plan 1:
 *
 *      Only need to modify one function cpu_loop_exit() to cover all the
 *      cases that need to be fix.
 *
 * Advantage of Plan 2:
 *
 *      Easy to control only exec it when necessary.
 *
 * Disadvantage of plan 1: Too many ways to call cpu_loop_exit()!
 *
 *      It is hard to tell when this fix is really needed and when it
 *      should not be executed such as exception happends and the CPU
 *      has to restore.
 *
 * Disadvantage of plan 2: Need to modity too many places.
 *
 * We choose plan 1:
 *      Use the flag 'after_exec_tb_fixed' in lsenv to control not to execute
 *      it more than once.
 */


void latxs_fix_after_excp_or_int(void)
{
    CPUX86State *env = lsenv->cpu_state;
    env->sigint_flag = 1;
    env->sigint_hamt_flag = 0;
    lsassertm(env->is_fcsr_simd == 0, "fcsr simd %s\n", __func__);
    env->is_fcsr_simd = 0;

    if (lsenv->after_exec_tb_fixed) {
        return;
    }

    if (!option_lsfpu && !option_soft_fpu) {
        latxs_fpu_fix_cpu_loop_exit();
    }

    if (sigint_enabled() == 1) {
        TranslationBlock *utb = lsenv->sigint_data.tb_unlinked;
        latxs_tb_relink(utb);
        lsenv->sigint_data.tb_unlinked = NULL;
    }

    lsenv->after_exec_tb_fixed = 1;
}

/* This is used in target/i386/fpu_helper.c */
int latxs_get_top_bias_from_env(CPUX86State *env)
{
    return (int)env->vregs[3];
}

void latxs_hamt_dump_context(void)
{
    CPUX86State *env = lsenv->cpu_state;

    pr_info(" ------- LATX-SYS");
    pr_info("         ENV 0x%lx", env);
    pr_info("         CPL 0x%lx", env->hflags & 0x3);
#ifdef TCG_USE_MULTI_REGION
    pr_info("Region 0 st  0x%lx", tcg_ctx->mregion[0].code_gen_buffer);
    pr_info("Region 1 st  0x%lx", tcg_ctx->mregion[1].code_gen_buffer);
#else
    pr_info("Region   st  0x%lx", tcg_ctx->code_gen_buffer);
#endif
}

#endif
