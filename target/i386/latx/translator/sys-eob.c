#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "latx-intb-sys.h"
#include "latx-string-sys.h"
#include "latx-counter-sys.h"
#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#include <string.h>

static int latxs_is_system_eob(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    ADDRX pc_first = td->sys.pc;
    ADDRX pc_next  = ir1_addr_next(ir1) + td->sys.cs_base;

    if (td->sys.tf || (td->sys.flags & HF_INHIBIT_IRQ_MASK)) {
        return 1;
    } else if (td->sys.cflags & CF_USE_ICOUNT) {
        if (((pc_next & TARGET_PAGE_MASK)
                   != ((pc_next + 15)
                       & TARGET_PAGE_MASK)
                   || (pc_next & ~TARGET_PAGE_MASK) == 0)) {
            return 1;
        } else {
            /* string instructions are eob in icount mode */
            if (latxs_ir1_is_string_op(ir1)) {
                /* string in/out are included */
                return 1;
            }
            /* io instructions is eob in icount mode */
            switch (ir1_opcode(ir1)) {
            case X86_INS_RDTSC:
            case X86_INS_RDTSCP:

            case X86_INS_IN:
            case X86_INS_OUT:

                return 1;
            default:
                return 0;
            }
        }
    } else if ((pc_next - pc_first) >= (TARGET_PAGE_SIZE - 32)) {
        return 1;
    }

    return 0;
}

static int latxs_ir1_is_hit_breakpoint(IR1_INST *ir1)
{
    CPUState *cpu = current_cpu;
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (current_cpu->singlestep_enabled) {
        return 1;
    }

    target_ulong pc = (target_ulong)(ir1_addr(ir1) + td->sys.cs_base);

    if (unlikely(!QTAILQ_EMPTY(&cpu->breakpoints))) {
        CPUBreakpoint *bp;
        QTAILQ_FOREACH(bp, &cpu->breakpoints, entry) {
            if (bp->pc == pc) {
                td->sys.bp_hit = 1;
                return 1;
            }
        }
    }

    if (cpu->watchpoint_hit) {
        td->sys.bp_hit = 1;
        return 1;
    }

    return 0;
}

static int latxs_ir1_is_lxx_seg(IR1_INST *ir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    switch (ir1_opcode(ir1)) {
    case X86_INS_LDS:
    case X86_INS_LES:
        if (td->sys.pe && !td->sys.vm86 && td->sys.code32) {
            return 1;
        } else {
            return 0;
        }
    case X86_INS_LSS:
        return 1;
    case X86_INS_LFS:
    case X86_INS_LGS:
    default:
        return 0;
    }
}

static int latxs_ir1_is_pop_seg_eob(IR1_INST *ir1)
{
    if (ir1_opcode(ir1) != X86_INS_POP) {
        return 0;
    }

    IR1_OPND *opnd0 = ir1_get_opnd(ir1, 0);
    if (!ir1_opnd_is_seg(opnd0)) {
        return 0;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    int seg_num = ir1_opnd_base_reg_num(opnd0);

    switch (seg_num) {
    case ds_index:
    case es_index:
        if (td->sys.pe && !td->sys.vm86 && td->sys.code32) {
            return 1;
        } else {
            return 0;
        }
    case ss_index:
        return 1;
    case fs_index:
    case gs_index:
    default:
        return 0;
    }
}

int latxs_ir1_is_eob_in_sys(IR1_INST *ir1)
{
    if (lsenv->tr_data->sys.cflags & CF_USE_ICOUNT) {
        if (latxs_ir1_is_mov_from_cr(ir1)) {
            return true;
        }
    }

    return latxs_ir1_is_mov_to_cr(ir1) ||         /* mov cr, src */
           latxs_ir1_is_mov_to_dr(ir1) ||         /* mov dr, src */
           latxs_ir1_is_pop_eflags(ir1) ||        /* popf */
           latxs_ir1_is_sysenter(ir1) ||          /* sysenter */
           latxs_ir1_is_sysexit(ir1) ||           /* sysexit */
           latxs_ir1_is_syscall(ir1) ||           /* syscall */
           latxs_ir1_is_sysret(ir1) ||            /* sysret */
           latxs_ir1_is_rsm(ir1) ||               /* rsm */
           latxs_ir1_is_sti(ir1) ||               /* sti */
           latxs_ir1_is_mov_to_seg_eob(ir1) ||    /* mov es/cs/ss/ds, src */
           latxs_ir1_is_pop_seg_eob(ir1) ||       /* pop ds/es/ss, src */
           latxs_ir1_is_repz_nop(ir1) ||          /* pause */
           latxs_ir1_is_pause(ir1) ||             /* pause */
           latxs_ir1_is_iret(ir1) ||              /* iret */
           latxs_ir1_is_lmsw(ir1) ||              /* lmsw */
           latxs_ir1_is_retf(ir1) ||              /* ret far */
           latxs_ir1_is_call_far(ir1) ||          /* call far */
           latxs_ir1_is_clts(ir1) ||              /* clts */
           latxs_ir1_is_invlpg(ir1) ||            /* invlpg */
           latxs_ir1_is_lxx_seg(ir1) ||           /* lss, lds/les */
           /* fldenv, frstor, fxrstor, xrstor */
           latxs_ir1_contains_fldenv(ir1) ||
           /* fninit: reset all to zero */
           latxs_ir1_is_fninit(ir1) ||
           /* fnsave: reset all to zero after save */
           latxs_ir1_is_fnsave(ir1) ||
           latxs_ir1_is_xsetbv(ir1) ||            /* xsetbv */
           latxs_ir1_is_mwait(ir1) ||             /* mwait */
           latxs_ir1_is_vmrun(ir1) ||             /* vmrun */
           latxs_ir1_is_stgi(ir1) ||              /* stgi */
#ifndef LATXS_STRING_LOOP_INSIDE
           latxs_ir1_is_string_op(ir1) || /* ins/outs/movs/cmps/stoc/lods/scas */
#endif
           latxs_is_system_eob(ir1) ||
           latxs_ir1_is_hit_breakpoint(ir1);
}

int latxs_ir1_is_mov_to_seg_eob(IR1_INST *ir1)
{
    if (ir1->info->id != X86_INS_MOV) {
        return 0;
    }

    IR1_OPND *opnd0 = ir1_get_opnd(ir1, 0);
    if (!ir1_opnd_is_seg(opnd0)) {
        return 0;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    int seg_index = ir1_opnd_base_reg_num(opnd0);

    if (td->sys.pe && !td->sys.vm86) {
        if (seg_index == ss_index ||
           (td->sys.code32 && seg_index < fs_index)) {
            return 1;
        }
    } else{
        if (seg_index == ss_index) {
            return 1;
        }
    }

    return 0;
}

static void latxs_tr_gen_hflags_set(uint32_t mask)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.flags & mask) {
        return;
    }

    IR2_OPND mask_reg   = latxs_ra_alloc_itemp();
    IR2_OPND hflags_reg = latxs_ra_alloc_itemp();

    latxs_load_imm32_to_ir2(&mask_reg, mask, EXMode_N);

    latxs_append_ir2_opnd2i(LISA_LD_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));
    latxs_append_ir2_opnd3(LISA_OR, &hflags_reg, &hflags_reg, &mask_reg);
    latxs_append_ir2_opnd2i(LISA_ST_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));

    latxs_ra_free_temp(&mask_reg);
    latxs_ra_free_temp(&hflags_reg);
}

static void latxs_tr_gen_hflags_clr(uint32_t mask)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!(td->sys.flags & mask)) {
        return;
    }

    IR2_OPND mask_reg   = latxs_ra_alloc_itemp();
    IR2_OPND hflags_reg = latxs_ra_alloc_itemp();

    latxs_load_imm32_to_ir2(&mask_reg, ~mask, EXMode_N);

    latxs_append_ir2_opnd2i(LISA_LD_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));
    latxs_append_ir2_opnd3(LISA_AND, &hflags_reg, &hflags_reg, &mask_reg);
    latxs_append_ir2_opnd2i(LISA_ST_W, &hflags_reg, &latxs_env_ir2_opnd,
            lsenv_offset_of_hflags(lsenv));

    latxs_ra_free_temp(&mask_reg);
    latxs_ra_free_temp(&hflags_reg);
}

void latxs_tr_gen_eob(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (!td->need_eob) {
        return;
    }

    if (td->inhibit_irq &&
        !(td->sys.flags & HF_INHIBIT_IRQ_MASK)) {
        latxs_tr_gen_hflags_set(HF_INHIBIT_IRQ_MASK);
    } else {
        latxs_tr_gen_hflags_clr(HF_INHIBIT_IRQ_MASK);
    }

    /*
     * > ignore_top_update
     *
     * is used by special system FP instructions that load TOP
     * from memory and save it into env->fpstt in helper
     *
     * Since the env->fpstt already contains the correct TOP,
     * there is no need to update again. If we do update TOP
     * here we will ruin the correct TOP value.
     *
     * > with LSFPU enabled
     *
     * No need to update TOP after every TB's execution
     * 1. in context switch the TOP will be readed
     * 2. TB-Link no more need to do FPU rotate
     *
     * For those special FP instructions, the TOP will be loaded
     * again after helper finished. So the context switch can
     * read the correct TOP value and save it again.
     */
    if (!(td->ignore_top_update) && !option_lsfpu && !option_soft_fpu) {
        /*
         * tr_gen_save_curr_top() is controled by td->force_curr_top_save.
         * Here we should always save TOP.
         */
        latxs_tr_gen_save_curr_top();
    }

    /* reset to 32s for TB-Link */
#ifndef TARGET_X86_64
    latxs_tr_reset_extmb(0xFF);
#endif
}

static void latxs_tr_gen_exit_tb_update_eip(
        int ignore_eip, ADDRX eip, int opnd_size)
{
    if (!ignore_eip) {
        IR2_OPND eip_opnd = latxs_ra_alloc_itemp();
        switch (opnd_size) {
        case 32:
            latxs_load_imm32_to_ir2(&eip_opnd, eip, EXMode_Z);
            break;
        case 16:
            latxs_load_imm32_to_ir2(&eip_opnd, eip & 0xffff, EXMode_Z);
            break;
#ifdef TARGET_X86_64
        case 64:
            latxs_load_imm64_to_ir2(&eip_opnd, eip);
            break;
#endif
        default:
            lsassert(0);
            break;
        }
#ifdef TARGET_X86_64
        latxs_append_ir2_opnd2i(LISA_ST_D, &eip_opnd, &latxs_env_ir2_opnd,
                lsenv_offset_of_eip(lsenv));
#else
        latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd, &latxs_env_ir2_opnd,
                lsenv_offset_of_eip(lsenv));
#endif
        latxs_ra_free_temp(&eip_opnd);
    }

    /* should correctly set env->eip */
    if (lsenv->tr_data->sys.tf) {
        latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_single_step,
                                         default_helper_cfg);
    }
}

/*
 * Generate exit tb for system EOB situation, no TB-Link.
 *
 * Called after all IR1s' translation and
 * translation is not ended with exception.
 * > Normal TB-End: pir1 will be NULL, do nothing
 * > system EOB situation: pir1 will be the last IR1
 *   > special system instruction: pir1 will be that IR1
 *                                 generate exit tb to next
 *                                 x86 instruction
 *   > system eob situation
 *     > If this IR1 is already TB-end instruction, do nothing
 *     > If not, generate exit tb to next x86 instruction
 *   > breakpoint: generate exit tb to next x86 instruction
 */
void latxs_tr_gen_sys_eob(IR1_INST *pir1)
{
    if (!pir1) {
        return;
    }

    if (ir1_is_tb_ending(pir1)) {
        /*
         * If the system EOB instruction is already
         * one normal TB-end instruction,
         * no more exit tb is needed.
         */
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    lsassertm(!tb->tb_too_large_pir1,
            "system EOB should not exist with TB too large.\n");

    /* EOB worker */
    latxs_tr_gen_eob();

    if (td->sys_eob_can_link) {
        latxs_tr_gen_exit_tb_j_tb_link(tb, 0);
    }

    /* t8: This TB's address */
    IR2_OPND tbptr = latxs_ra_alloc_dbt_arg1(); /* a6($10) */
    latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, (ADDR)tb);

    /* t9: next x86 instruction's address */
    ADDRX next_eip = ir1_addr_next(pir1);
#ifdef TARGET_X86_64
    latxs_tr_gen_exit_tb_update_eip(td->ignore_eip_update, next_eip, 64);
#else
    latxs_tr_gen_exit_tb_update_eip(td->ignore_eip_update, next_eip, 32);
#endif

    /* jump to context switch */
    if (td->sys_eob_can_link) {
        latxs_tr_gen_exit_tb_j_context_switch(&tbptr, 1, 0);
    } else {
        latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, 0);
    }
}

void latxs_tr_gen_exit_tb_load_tb_addr(IR2_OPND *tbptr, ADDR tb_addr)
{
    TranslationBlock *tb = lsenv->tr_data->curr_tb;
    tb->fastcs_jmp_glue_checker = 1;

    if (lsenv && lsenv->tr_data && lsenv->tr_data->curr_tb) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;
        ADDR code_ptr = (ADDR)tb->tc.ptr;
        ADDR code_off = (ADDR)(lsenv->tr_data->ir2_asm_nr << 2);
        ADDR ins_pc = code_ptr + code_off;
        /* tbptr = ins_pc + offset => offset = tbptr - ins_pc */
        int offset = (tb_addr - ins_pc) >> 2;
        if (int32_in_int20(offset)) {
            latxs_append_ir2_opnd1i(LISA_PCADDI, tbptr, offset);
        } else {
            latxs_load_imm64_to_ir2(tbptr, tb_addr);
        }
    } else {
        latxs_load_imm64_to_ir2(tbptr, tb_addr);
    }
}

void latxs_tr_gen_exit_tb_j_tb_link(TranslationBlock *tb, int succ_id)
{
    if (latxs_fastcs_is_jmp_glue_direct()) {
        /* fastcs BL for jmp glue with return */
        latxs_append_ir2_opnd0_(lisa_nop);
    }

    /* 1. set a label for native code linkage */
    IR2_OPND goto_label_opnd = latxs_ir2_opnd_new_label();

    /* remember where the j instruction position */
    /* reuse jmp_reset_offset[], they will be rewritten in label_dispose */
    /* at that time we can calculate exact offset for this position */
    latxs_append_ir2_opnd1(LISA_LABEL, &goto_label_opnd);
    tb->jmp_reset_offset[succ_id] = goto_label_opnd.val;

    /* point to current j insn addr plus 8 by default, will resolve in */
    /* label_dispose, two instructions for pcaddu18i and jirl patch */
    /* patch may be pcaddu18i and jirl, this b should b to next insn of nop other than nop  */
    /* if nop was interrupt, and may just exec jirl without pcaddu18i  */
    latxs_append_ir2_opnda(LISA_B, 2);
    latxs_append_ir2_opnd0_(lisa_nop);
}

void latxs_tr_gen_exit_tb_j_context_switch(IR2_OPND *tbptr,
        int can_link, int succ_id)
{
    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    if (can_link) {
        latxs_append_ir2_opnd2i(LISA_ORI, ret0, tbptr, succ_id);
    } else {
        latxs_append_ir2_opnd2i(LISA_ORI, ret0, zero, succ_id);
    }
    /* jump to context_switch_native_to_bt */
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;
    int rid = td->region_id;
    lsassert(rid == tb->region_id);
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    ADDR code_buf = (ADDR)tb->tc.ptr;
    int offset = td->ir2_asm_nr << 2;

    int64_t ins_offset = (GET_SC_TABLE(rid, cs_native_to_bt) - code_buf - offset) >> 2;
    latxs_append_ir2_jmp_far(ins_offset, 0);
}

#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_INDIRBR)
static void latxs_counter_patch_inbr(CPUState *cpu, IR1_INST *pir1)
{
    CPUX86State *env = cpu->env_ptr;
    int cpl = env->hflags & 0x3;

    IR2_OPND tmp1 = latxs_ra_alloc_itemp();
    IR2_OPND tmp2 = latxs_ra_alloc_itemp();

    IR1_OPCODE opcode = ir1_opcode(pir1);
    switch (opcode) {
    case X86_INS_RET:
        if (cpl == 0) latxs_counter_gen_inbr_cpl0_ret(cpu, &tmp1, &tmp2);
        if (cpl == 3) latxs_counter_gen_inbr_cpl3_ret(cpu, &tmp1, &tmp2);
        break;
    case X86_INS_CALL:
        if (cpl == 0) latxs_counter_gen_inbr_cpl0_call(cpu, &tmp1, &tmp2);
        if (cpl == 3) latxs_counter_gen_inbr_cpl3_call(cpu, &tmp1, &tmp2);
        break;
    case X86_INS_JMP:
        if (cpl == 0) latxs_counter_gen_inbr_cpl0_jmp(cpu, &tmp1, &tmp2);
        if (cpl == 3) latxs_counter_gen_inbr_cpl3_jmp(cpu, &tmp1, &tmp2);
        break;
    default:
        break;
    }

    latxs_ra_free_temp(&tmp1);
    latxs_ra_free_temp(&tmp2);
}
#endif

/* Should always use TB-Link. */
void latxs_tr_gen_eob_if_tb_too_large(IR1_INST *pir1)
{
    if (!pir1) {
        return;
    }

    if (ir1_is_tb_ending(pir1)) {
        return;
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    lsassertm(!tb->sys_eob_pir1,
            "TB too large should not exist with system EOB.\n");

    int can_link = option_tb_link;

    latxs_tr_gen_eob();
    CPUState *cpu = env_cpu(lsenv->cpu_state);
    if (td->sys.bp_hit || cpu->singlestep_enabled) {
        current_cpu->exception_index = EXCP_DEBUG;
        can_link = 0;
    }

    IR2_OPND tbptr = latxs_ra_alloc_dbt_arg1(); /* a6($10) */
    int succ_id = 1;

    /* t8: This TB's address */
    latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, (ADDR)tb);

    if (can_link) {
        latxs_tr_gen_exit_tb_j_tb_link(tb, succ_id);
    }

    /* t9: next x86 instruction's address */
    ADDRX next_eip = ir1_addr_next(pir1);
#ifdef TARGET_X86_64
    latxs_tr_gen_exit_tb_update_eip(td->ignore_eip_update, next_eip, 64);
#else
    latxs_tr_gen_exit_tb_update_eip(td->ignore_eip_update, next_eip, 32);
#endif

    /* jump to context switch */
    latxs_tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
}

static int cpu_hit_breakpoint(void)
{
    CPUState *cpu = env_cpu(lsenv->cpu_state);
    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->sys.bp_hit || cpu->singlestep_enabled) {
        lsassert(cpu == current_cpu);
        current_cpu->exception_index = EXCP_DEBUG;
        return 1;
    }
    return 0;
}

static int branch_is_cross_page(IR1_INST *branch, int n)
{
    ADDRX curr_eip = ir1_addr(branch);
    ADDRX next_eip = 0;

    if (ir1_is_call(branch) && !ir1_is_indirect_call(branch)) {
        next_eip = ir1_target_addr(branch);
        return (next_eip >> TARGET_PAGE_BITS) != (curr_eip >> TARGET_PAGE_BITS);
    }

    if (ir1_is_jump(branch) && !ir1_is_indirect_jmp(branch)) {
        next_eip = ir1_target_addr(branch);
        return (next_eip >> TARGET_PAGE_BITS) != (curr_eip >> TARGET_PAGE_BITS);
    }

    if (ir1_is_branch(branch)) {
        if (n == 0) next_eip = ir1_addr_next(branch);
        if (n == 1) next_eip = ir1_target_addr(branch);
        return (next_eip >> TARGET_PAGE_BITS) != (curr_eip >> TARGET_PAGE_BITS);
    }

    return 0;
}

static void tr_gen_branch_save_next_eip(IR1_INST *branch, int n)
{
    int next_eip_size = 32;
#ifdef TARGET_X86_64
    next_eip_size = latxs_ir1_addr_size(branch) << 3;
#endif

    ADDRX next_eip = 0;

    TRANSLATION_DATA *td = lsenv->tr_data;
    int doit = 0;
    int ignore_eip = 0;

    if (ir1_is_call(branch) && !ir1_is_indirect_call(branch)) {
        next_eip = ir1_target_addr(branch);
        if (latxs_ir1_addr_size(branch) == 2 ||
            ir1_opnd_size(ir1_get_opnd(branch, 0)) == 16) {
            next_eip_size = 16;
        }
        doit = 1;
    }

    if (ir1_is_jump(branch) && !ir1_is_indirect_jmp(branch)) {
        next_eip = ir1_target_addr(branch);
        next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 0));
        ignore_eip = td->ignore_eip_update;
        doit = 1;
    }

    if (ir1_is_branch(branch)) {
        if (n == 0) next_eip = ir1_addr_next(branch);
        if (n == 1) next_eip = ir1_target_addr(branch);
        next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 0));
        doit = 1;
    }

    if (doit) {
        latxs_tr_gen_exit_tb_update_eip(ignore_eip, next_eip, next_eip_size);
    }
}

/*
 * Details of exit-tb
 * <A> EOB: End-Of-TB worker  latxs_tr_gen_eob()
 * <B> Load TB addr   $t8     latxs_tr_gen_exit_tb_load_tb_addr()
 * <C> Save next EIP          latxs_tr_gen_exit_tb_update_eip()
 * <D> TB-Link j 0            latxs_tr_gen_exit_tb_j_tb_link()
 * <E> Context Switch         latxs_tr_gen_exit_tb_j_context_switch()
 */
void latxs_tr_generate_exit_tb(IR1_INST *branch, int succ_id)
{
    int      next_eip_size = 32;
#ifdef TARGET_X86_64
    next_eip_size = latxs_ir1_addr_size(branch) << 3;
#endif
    ADDRX    next_eip;

    IR1_OPCODE opcode = ir1_opcode(branch);
    int can_link = option_tb_link;

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;
    int rid = td->region_id;
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    /* debug support : when hit break point, do NOT TB-link */
    if (cpu_hit_breakpoint()) {
        can_link = 0;
    }

    /*
     * <A> EOB end-of-tb worker
     *
     * for branch instructions, EOB will be called before branch
     * since it contains two exit ports.
     */
    if (!ir1_is_branch(branch)) {
        latxs_tr_gen_eob();
    }

    /*
     * <B> load tb address
     *
     *        |       | need TB addr   |
     * FastCS | LSFPU | for jmp glue ? | Reason
     * -----------------------------------------------------
     *    X   |   X   |     YES        | FPU rotate
     *    X   |   O   |     NO         |
     *    O   |   X   |     YES        | TB's fast cs mask
     *    O   |   O   |     YES        | TB's fast cs mask
     */

    IR2_OPND tbptr = latxs_ra_alloc_dbt_arg1(); /* t8($24) */
    ADDR tb_addr = (ADDR)tb;

    int load_tb_addr_before_tblink  = 0; /* <B> load tb address */
    int save_next_eip_before_tblink = 0; /* <C> save next pc */
    
    load_tb_addr_before_tblink = (option_lsfpu || option_soft_fpu) ? 0 : 1;

    if (option_lsfpu && latxs_fastcs_is_jmp_glue()) {
        load_tb_addr_before_tblink = 1;
    }

    if (option_cross_page_check && branch_is_cross_page(branch, succ_id)) {
        tb->next_tb_cross_page[succ_id] = 1;
        load_tb_addr_before_tblink = 1;
        tr_gen_branch_save_next_eip(branch, succ_id);
        save_next_eip_before_tblink = 1;
    }

    /* <B> load tb address before tb link */
    if (load_tb_addr_before_tblink) {
        latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
    }

    /* <D> generate 'j 0' for TB-Link */
    if (can_link &&
            !ir1_is_indirect_jmp(branch) &&
            !ir1_is_indirect_call(branch) &&
            opcode != X86_INS_RET) {
        latxs_tr_gen_exit_tb_j_tb_link(tb, succ_id);
    }

    /* <B> load tb address after tb link : for context switch */
    if (!load_tb_addr_before_tblink) {
        latxs_tr_gen_exit_tb_load_tb_addr(&tbptr, tb_addr);
    }

    /*
     * <E> context swtich
     * 1. prepare Current TranslationBlock's address <B>
     *    as the return value (a0/v0) if tb link is allowed
     * 2. update PC (eip) in cpu state <C>
     * 3. jump to context switch native to bt
     */

#define CS_ACTION_UNDEF     0
#define CS_ACTION_DEFAULT   1
#define CS_ACTION_LJMP      2
#define CS_ACTION_INDIR     3

    int cs_action = CS_ACTION_UNDEF;

    if (ir1_is_branch(branch))  cs_action = CS_ACTION_DEFAULT;
    if (opcode == X86_INS_RET)  cs_action = CS_ACTION_INDIR;
    if (opcode == X86_INS_LJMP) cs_action = CS_ACTION_LJMP;
    if (ir1_is_call(branch)) {
        if (ir1_is_indirect_call(branch)) {
            cs_action = CS_ACTION_INDIR;
        } else {
            cs_action = CS_ACTION_DEFAULT;
        }
    }
    if (ir1_is_jump(branch)) {
        if (ir1_is_indirect_jmp(branch)) {
            cs_action = CS_ACTION_INDIR;
        } else {
            cs_action = CS_ACTION_DEFAULT;
        }
    }

    switch (cs_action) {
    case CS_ACTION_DEFAULT:
        if (!save_next_eip_before_tblink) {
            tr_gen_branch_save_next_eip(branch, succ_id);
        }
        if (tb->next_tb_cross_page[succ_id]) {
            can_link &= option_cross_page_jmp_link;
        }
        latxs_tr_gen_exit_tb_j_context_switch(&tbptr, can_link, succ_id);
        break;
    case CS_ACTION_LJMP:
        /* only ptr16:16/ptr16:32 is supported now */
        if (td->sys.pe && !td->sys.vm86) {
            lsassert(td->ignore_eip_update == 1);
            latxs_tr_gen_exit_tb_update_eip(1, 0, 0);
        } else if (!ir1_opnd_is_mem(ir1_get_opnd(branch, 0))) {
            next_eip = ir1_opnd_uimm(ir1_get_opnd(branch, 1));
            next_eip_size = ir1_opnd_size(ir1_get_opnd(branch, 1));
            latxs_tr_gen_exit_tb_update_eip(0, next_eip, next_eip_size);
        }

        /* Always no TB-link for jmp far */
        latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
        break;
    case CS_ACTION_INDIR:
#if defined(BG_COUNTER_ENABLE) && defined(BG_COUNTER_GROUP_INDIRBR)
        latxs_counter_patch_inbr(cpu, branch);
#endif
        if (sigint_enabled() == 1) {
#ifdef LATXS_INTB_LINK_ENABLE
            if (can_link && intb_link_enable()) {
                ADDR code_buf = (ADDR)tb->tc.ptr;
                int offset = td->ir2_asm_nr << 2;

                IR2_OPND sigint_label = latxs_ir2_opnd_new_label();
                latxs_append_ir2_opnd1(LISA_LABEL, &sigint_label);

                int64_t ins_offset =
                    (GET_SC_TABLE(rid, intb_lookup) - code_buf - offset) >> 2;
                latxs_append_ir2_jmp_far(ins_offset, 0);

                /* will be resolved in label_dispose() */
                tb->jmp_reset_offset[0] =
                    latxs_ir2_opnd_label_id(&sigint_label);
                latxs_append_ir2_opnd0_(lisa_nop);
                tb->is_indir_tb = 1;
            }
#endif
            latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
        } else {
            if (can_link && intb_link_enable()) {
#ifdef LATXS_INTB_LINK_ENABLE
                ADDR code_buf = (ADDR)tb->tc.ptr;
                int offset = td->ir2_asm_nr << 2;
                int64_t ins_offset =
                    (GET_SC_TABLE(rid, intb_lookup) - code_buf - offset) >> 2;
                latxs_append_ir2_jmp_far(ins_offset, 0);
#endif
            } else {
                latxs_tr_gen_exit_tb_j_context_switch(NULL, 0, succ_id);
            }
        }
        break;
    default:
        lsassertm(0, "not implement.\n");
        break;
    }
}
