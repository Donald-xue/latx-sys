#include "common.h"
#include "latx-config.h"
#include "diStorm/distorm.h"
#include "ir1.h"
#include "ir2.h"
#include "env.h"
#include "etb.h"
#include "reg_alloc.h"
#include "latx-options.h"
#include "etb.h"
#include "shadow_stack.h"
#include "profile.h"

#include "trace.h"

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

int lsenv_offset_of_mips_regs(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->mips_regs[i]) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_gpr(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->regs[i]) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_eflags(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->eflags) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_ibtc_table(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->ibtc_table_p) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_tb_jmp_cache_ptr(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->tb_jmp_cache_ptr) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_eip(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->eip) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_top(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->fpstt) - (ADDR)lsenv->cpu_state);
}

int lsenv_get_top(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return cpu->fpstt;
}

void lsenv_set_top(ENV *lsenv, int new_fpstt)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->fpstt = new_fpstt;
}

int lsenv_offset_of_status_word(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->fpus) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_control_word(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->fpuc) - (ADDR)lsenv->cpu_state);
}

int lsenv_get_fpu_control_word(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return cpu->fpuc;
}

int lsenv_offset_of_tag_word(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->fptags[0]) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_fpr(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR) & (cpu->fpregs[i].d) - (ADDR)lsenv->cpu_state);
}

void lsenv_set_fpregs(ENV *lsenv, int i, FPReg new_value)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->fpregs[i] = new_value;
}

FPReg lsenv_get_fpregs(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return cpu->fpregs[i];
}

int lsenv_offset_of_mmx(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->fpregs[i]) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_mxcsr(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->mxcsr) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_seg_base(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->segs[i].base) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_seg_selector(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->segs[i].selector) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_seg_limit(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->segs[i].limit) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_seg_flags(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->segs[i].flags) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_gdt_base(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->gdt.base) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_gdt_limit(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->gdt.limit) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_exception_index(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    CPUState *cpu_state = env_cpu(cpu);
    return (int)((ADDR)(&cpu_state->exception_index) - (ADDR)lsenv->cpu_state);
}

/* virtual registers */
int lsenv_offset_of_vreg(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->vregs[i]) - (ADDR)lsenv->cpu_state);
}

int lsenv_offset_of_guest_base(ENV *lsenv)
{
    return lsenv_offset_of_vreg(lsenv, 0);
}

int lsenv_offset_of_last_executed_tb(ENV *lsenv)
{
    return lsenv_offset_of_vreg(lsenv, 1);
}

int lsenv_offset_of_next_eip(ENV *lsenv)
{
    return lsenv_offset_of_vreg(lsenv, 2);
}

int lsenv_offset_of_top_bias(ENV *lsenv)
{
    return lsenv_offset_of_vreg(lsenv, 3);
}

int lsenv_offset_of_ss(ENV *lsenv)
{
    return lsenv_offset_of_vreg(lsenv, 4);
}


/* Instead save intno in helper_raise_int, we save intno in translate_int. */
void helper_raise_int(void)
{
    set_CPUX86State_error_code(lsenv, 0);
    set_CPUX86State_exception_is_int(lsenv, 1);
    set_CPUState_can_do_io(lsenv, 1);
    siglongjmp_cpu_jmp_env();
}
void set_CPUX86State_error_code(ENV *lsenv, int error_code)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->error_code = error_code;
}
void set_CPUX86State_exception_is_int(ENV *lsenv, int exception_is_int)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->exception_is_int = exception_is_int;
}
int lsenv_offset_exception_next_eip(ENV *lsenv)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->exception_next_eip) - (ADDR)lsenv->cpu_state);
}
void set_CPUState_can_do_io(ENV *lsenv, int can_do_io)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    CPUState *cs = env_cpu(cpu);
    cs->can_do_io = can_do_io;
}
void siglongjmp_cpu_jmp_env(void)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;

    /* siglongjmp will skip the execution of latx_after_exec_tb
     * which is expected to reset top_bias/top
     */
    TranslationBlock *last_tb =
        (TranslationBlock *)lsenv_get_last_executed_tb(lsenv);
    latx_after_exec_tb(cpu, last_tb);

    CPUState *cpu_state = env_cpu(cpu);
    siglongjmp(cpu_state->jmp_env, 1);
}

int lsenv_offset_of_xmm(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (int)((ADDR)(&cpu->xmm_regs[i]) - (ADDR)lsenv->cpu_state);
}

ADDR lsenv_get_vreg(ENV *lsenv, int i)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    return (ADDR)cpu->vregs[i];
}

void lsenv_set_vreg(ENV *lsenv, int i, ADDR val)
{
    CPUX86State *cpu = (CPUX86State *)lsenv->cpu_state;
    cpu->vregs[i] = (uint64_t)val;
}

ADDR lsenv_get_guest_base(ENV *lsenv) { return (ADDR)lsenv_get_vreg(lsenv, 0); }

void lsenv_set_guest_base(ENV *lsenv, ADDR gbase)
{
    lsenv_set_vreg(lsenv, 0, gbase);
}

ADDR lsenv_get_last_executed_tb(ENV *lsenv)
{
    return (ADDR)lsenv_get_vreg(lsenv, 1);
}

void lsenv_set_last_executed_tb(ENV *lsenv, ADDR tb)
{
    lsenv_set_vreg(lsenv, 1, tb);
}

ADDRX lsenv_get_next_eip(ENV *lsenv) { return (ADDRX)lsenv_get_vreg(lsenv, 2); }

void lsenv_set_next_eip(ENV *lsenv, ADDRX eip)
{
    lsenv_set_vreg(lsenv, 2, eip);
}

int lsenv_get_top_bias(ENV *lsenv) { return (int)lsenv_get_vreg(lsenv, 3); }

void lsenv_set_top_bias(ENV *lsenv, int top_bias)
{
    lsenv_set_vreg(lsenv, 3, top_bias);
}

int lsenv_offset_of_tr_data(ENV *lsenv)
{
    return (ADDR)(&lsenv->tr_data) - (ADDR)lsenv;
}

int lsenv_get_last_executed_tb_top_out(ENV *lsenv)
{
    ETB *etb =
        &((TranslationBlock *)(lsenv_get_last_executed_tb(lsenv)))->extra_tb;
    return etb->_top_out;
}

uint8_t cpu_read_code_via_qemu(void *cpu, ADDRX pc)
{
    return cpu_ldub_code((CPUX86State *)cpu, (target_ulong)pc);
}

ADDRX qm_tb_get_pc(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return (ADDRX)ptb->pc;
}

ETB *qm_tb_get_extra_tb(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return &ptb->extra_tb;
}

void *qm_tb_get_code_cache(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return ptb->tc.ptr;
}

void *qm_tb_get_jmp_target_arg(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return &(ptb->jmp_target_arg[0]);
}

void *qm_tb_get_jmp_reset_offset(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return &(ptb->jmp_reset_offset[0]);
}

ADDR cpu_get_guest_base(void) { return guest_base; }

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

void latx_before_exec_tb(CPUArchState *env, struct TranslationBlock *tb)
{
    if (option_trace_tb)
        fprintf(stderr,
                "[LATX] before executing TB {PC = %p, code at %p}.\n",
                (void *)(unsigned long)tb->pc, (void *)tb->tc.ptr);
    counter_tb_exec += 1;

    if (!option_lsfpu) {
        lsassert(lsenv_get_top_bias(lsenv) == 0);
        if (env->fpstt != etb_get_top_in(&tb->extra_tb)) {
            rotate_fpu_to_top(etb_get_top_in(&tb->extra_tb));
        }
    }
}

void latx_after_exec_tb(CPUArchState *env, struct TranslationBlock *tb)
{
    if (option_profile)
        context_switch_time++;
    if (option_trace_tb)
        fprintf(stderr,
                "[LATX] after  executing TB {PC = %p, code at %p}.\n",
                (void *)(unsigned long)tb->pc, (void *)tb->tc.ptr);

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
