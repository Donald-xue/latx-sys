#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

static int is_ldst_realized_by_softmmu(IR2_OPCODE op)
{
    switch (op) {
    case LISA_LD_B:
    case LISA_LD_BU:
    case LISA_LD_H:
    case LISA_LD_HU:
    case LISA_LD_W:
    case LISA_LD_WU:
    case LISA_LD_D:
    case LISA_ST_B:
    case LISA_ST_H:
    case LISA_ST_W:
    case LISA_ST_D:
        return 1;
    default:
        return 0;
    }
}

static int convert_to_tcgmemop(IR2_OPCODE op)
{
    switch (op) {
    case LISA_LD_B:
        return MO_SB;
    case LISA_LD_BU:
    case LISA_ST_B:
        return MO_UB;
    case LISA_LD_H:
        return MO_LESW;
    case LISA_LD_HU:
    case LISA_ST_H:
        return MO_LEUW;
    case LISA_LD_W:
        return MO_LESL;
    case LISA_LD_WU:
    case LISA_ST_W:
        return MO_LEUL;
    case LISA_LD_D:
    case LISA_ST_D:
        return MO_LEQ;
    default:
        lsassertm(0, "not support IR2 %d in convert_to_tcgmemop.\n", op);
        return -1;
    }
}

static int get_ldst_align_bits(IR2_OPCODE opc)
{
    switch (opc) {
    case LISA_LD_B:
    case LISA_LD_BU:
    case LISA_ST_B:
        return 0; /* MemOp & MO_SIZE == MO_8  == 0 */
    case LISA_LD_H:
    case LISA_LD_HU:
    case LISA_ST_H:
        return 1; /* MemOp & MO_SIZE == MO_16 == 1 */
    case LISA_LD_W:
    case LISA_LD_WU:
    case LISA_ST_W:
        return 2; /* MemOp & MO_SIZE == MO_32 == 2 */
    case LISA_LD_D:
    case LISA_ST_D:
        return 3; /* MemOp & MO_SIZE == MO_64 == 3 */
    default:
        return -1; /* no support for unaligned access */
    }
}

/* return label point to slow path */
static void tr_gen_lookup_qemu_tlb(
        IR2_OPCODE op,
        IR2_OPND *gpr_opnd,
        IR2_OPND *mem_opnd,
        int mmu_index,
        bool is_load,
        IR2_OPND label_slow_path)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    /* offset from ENV */
    int fast_off  = TLB_MASK_TABLE_OFS(mmu_index);
    int mask_off  = fast_off + offsetof(CPUTLBDescFast, mask);  /* 64-bit */
    int table_off = fast_off + offsetof(CPUTLBDescFast, table); /* 64-bit */
    /* offset from tlb entry */
    int add_off   = offsetof(CPUTLBEntry, addend); /* 64-bit */
    int tag_off   = (is_load ? offsetof(CPUTLBEntry, addr_read)
                   : offsetof(CPUTLBEntry, addr_write)); /* 32-bit */

    /* 0. format the mem opnd */
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(mem_opnd,
            &mem_no_offset_new_tmp);
    IR2_OPND mem = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
    if (mem_no_offset_new_tmp) {
        latxs_append_ir2_opnd2_(lisa_mov32z, &mem, &mem);
    }

    /* 1. load f[mmu].mask */
    IR2_OPND mask_opnd = latxs_ra_alloc_itemp();
    if (int32_in_int12(mask_off)) {
        latxs_append_ir2_opnd2i(LISA_LD_D, &mask_opnd, env, mask_off);
    } else {
        latxs_load_imm32_to_ir2(&mask_opnd, mask_off, EXMode_S);
        latxs_append_ir2_opnd3(LISA_ADD_D, &mask_opnd, env, &mask_opnd);
        latxs_append_ir2_opnd2i(LISA_LD_D, &mask_opnd, &mask_opnd, 0);
    }

    /* 2. extract tlb index */
    IR2_OPND index_opnd = latxs_ra_alloc_itemp();
    /* 2.1 index = address >> shift */
    int shift = TARGET_PAGE_BITS - CPU_TLB_ENTRY_BITS;
    latxs_append_ir2_opnd2i(LISA_SRLI_W, &index_opnd, &mem, shift);

    /* 2.2 index = index & mask */
    latxs_append_ir2_opnd3(LISA_AND, &index_opnd, &index_opnd, &mask_opnd);
    latxs_ra_free_temp(&mask_opnd);

    /* 3. load f[mmu].table */
    IR2_OPND table_opnd = latxs_ra_alloc_itemp();
    if (int32_in_int12(table_off)) {
        latxs_append_ir2_opnd2i(LISA_LD_D, &table_opnd, env, table_off);
    } else {
        latxs_load_imm32_to_ir2(&table_opnd, table_off, EXMode_S);
        latxs_append_ir2_opnd3(LISA_ADD_D, &table_opnd, env, &table_opnd);
        latxs_append_ir2_opnd2i(LISA_LD_D, &table_opnd, &table_opnd, 0);
    }

    /* 4. tlb entry = table + index */
    latxs_append_ir2_opnd3(LISA_ADD_D, &table_opnd, &table_opnd, &index_opnd);
    IR2_OPND tlb_opnd = table_opnd;
    latxs_ra_free_temp(&index_opnd);

    /* 5. load compare part from tlb entry */
    IR2_OPND tag_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_WU, &tag_opnd, &tlb_opnd, tag_off);

    /* 6. cmp = vaddr & mask */
    /*
     *                      12 11 8 7  4 3  0
     * vaddr 0x 1111 .... 1111 0000 0000 0000
     *                       ^              \- align_bits = 0 ; B  8-bits
     *                       |                              1 ; H 16-bits
     *          i386.TARGET_PAGE_BITS = 12                  2 ; W 32-bits
     *                                                      3 ; D 64-bits
     */
    int align_bits = get_ldst_align_bits(op);
    IR2_OPND cmp_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &cmp_opnd, &mem);
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_D, &cmp_opnd, zero,
            TARGET_PAGE_BITS - 1, align_bits);

    /* 7. compare cmp and tag */
    if (!option_smmu_slow) {
        latxs_append_ir2_opnd3(LISA_BNE,
                &cmp_opnd, &tag_opnd, &label_slow_path);
    } else {
        /* Always jump to Softmmu Slow Path : mainly for debug */
        latxs_append_ir2_opnd3(LISA_BEQ,
                zero, zero, &label_slow_path);
    }
    latxs_ra_free_temp(&tag_opnd);
    latxs_ra_free_temp(&cmp_opnd);

    /* 8. load addend from tlb entry */
    IR2_OPND add_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_D, &add_opnd, &tlb_opnd, add_off);
    latxs_ra_free_temp(&tlb_opnd);

    /* 9. get hvaddr if not branch */
    switch (op) {
    case LISA_LD_B:
        latxs_append_ir2_opnd3(LISA_LDX_B, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_LD_H:
        latxs_append_ir2_opnd3(LISA_LDX_H, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_LD_W:
        latxs_append_ir2_opnd3(LISA_LDX_W, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_LD_D:
        latxs_append_ir2_opnd3(LISA_LDX_D, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_LD_BU:
        latxs_append_ir2_opnd3(LISA_LDX_BU, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_LD_HU:
        latxs_append_ir2_opnd3(LISA_LDX_HU, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_LD_WU:
        latxs_append_ir2_opnd3(LISA_LDX_WU, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_ST_B:
        latxs_append_ir2_opnd3(LISA_STX_B, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_ST_H:
        latxs_append_ir2_opnd3(LISA_STX_H, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_ST_W:
        latxs_append_ir2_opnd3(LISA_STX_W, gpr_opnd, &mem, &add_opnd);
        break;
    case LISA_ST_D:
        latxs_append_ir2_opnd3(LISA_STX_D, gpr_opnd, &mem, &add_opnd);
        break;
    default:
        lsassertm(0, "wrong in softmmu\n");
        break;
    }
    latxs_ra_free_temp(&add_opnd);

    if (mem_no_offset_new_tmp) {
        latxs_ra_free_temp(&mem_no_offset);
    }
}

static void td_rcd_softmmu_slow_path(
        IR2_OPCODE op,
        IR2_OPND *gpr_ir2_opnd,
        IR2_OPND *mem_ir2_opnd,
        IR2_OPND *label_slow_path,
        IR2_OPND *label_exit,
        int mmu_index,
        int save_tmp,
        int is_load)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    int sp_nr  = td->slow_path_rcd_nr;
    int sp_max = td->slow_path_rcd_max;

    /* get Slow Path Record entry */

    softmmu_sp_rcd_t *sp = NULL;

    if (sp_nr == sp_max) {
        td->slow_path_rcd_max = sp_max << 1;
        sp_max = sp_max << 1;
        sp = mm_realloc(td->slow_path_rcd,
                sp_max * sizeof(softmmu_sp_rcd_t));
        td->slow_path_rcd = sp;
        sp = &sp[sp_nr];
    } else {
        softmmu_sp_rcd_t *sp_array = td->slow_path_rcd;
        sp = (void *)(&sp_array[sp_nr]);
    }

    td->slow_path_rcd_nr += 1;

    /* save information about slow path */
    sp->op = op;
    sp->gpr_ir2_opnd = *gpr_ir2_opnd;
    sp->mem_ir2_opnd = *mem_ir2_opnd;
    sp->label_slow_path = *label_slow_path;
    sp->label_exit = *label_exit;
    sp->mmu_index = mmu_index;
    sp->tmp_need_save = save_tmp;
    sp->is_load = is_load;

    sp->retaddr = (ADDR)(tb->tc.ptr) +
                  (ADDR)((td->real_ir2_inst_num - 2) << 2);

    if (!option_lsfpu) {
        sp->fpu_top = latxs_td_fpu_get_top();
    }

    sp->tmp_mask = td->itemp_mask;

    if (option_by_hand) {
        int i = 0;
        for (i = 0; i < CPU_NB_REGS; ++i) {
            sp->reg_exmode[i] = td->reg_exmode[i];
            sp->reg_exbits[i] = td->reg_exbits[i];
        }
    }
}

static void tr_gen_ldst_slow_path(
        IR2_OPCODE op,
        IR2_OPND  *gpr_opnd, /* temp(t0-t7) or mapping(s1-s8) */
        IR2_OPND  *mem_opnd,
        IR2_OPND  *label_slow_path,
        IR2_OPND  *label_exit,
        int        mmu_index,
        int        is_load,
        int        save_temp)
{
    td_rcd_softmmu_slow_path(
            op, gpr_opnd, mem_opnd,
            label_slow_path, label_exit,
            mmu_index, save_temp, is_load);
}



static void __gen_ldst_softmmu_helper_native(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    int mmu_index = td->sys.mem_index;
    bool is_load  = latxs_ir2_opcode_is_load(op);

    /*
     * 1. lookup QEMU TLB in native code
     *    and finish memory access if TLB hit
     *    temp register is free to use inside.
     */
    IR2_OPND label_slow_path = latxs_ir2_opnd_new_label();
    tr_gen_lookup_qemu_tlb(op, opnd_gpr, opnd_mem, mmu_index,
                           is_load, label_slow_path);

    /* 2. memory access finish. jump slow path. */
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();

    /*
     * 3. slow path : call QEMU's helper
     *    Here we just record the data to generate slow path
     *    The real slow path will be generated at the end of TB
     */
    tr_gen_ldst_slow_path(op, opnd_gpr, opnd_mem,
            &label_slow_path, &label_exit,
            mmu_index, is_load, save_temp);

    /* 4. exit from fast path or return from slow path */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
}

static void __tr_gen_softmmu_sp_rcd(softmmu_sp_rcd_t *sp)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
    IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
    IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;
    IR2_OPND *arg4 = &latxs_arg4_ir2_opnd;

    latxs_append_ir2_opnd1(LISA_LABEL, &sp->label_slow_path);

    TRANSLATION_DATA *td = lsenv->tr_data;
    int tmp_mask_bak = td->itemp_mask;
    td->itemp_mask = sp->tmp_mask;

    int top_bak = 0;
    if (!option_lsfpu) {
        top_bak = latxs_td_fpu_get_top();
        latxs_td_fpu_set_top(sp->fpu_top);
    }

    if (sp->tmp_need_save) {
        latxs_tr_save_temp_register_mask(sp->tmp_mask);
    }

    EXMode gpr_em_bak[CPU_NB_REGS];
    EXBits gpr_eb_bak[CPU_NB_REGS];

    int i = 0;

    /* 1. save native context */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper_prologue_cfg(cfg);

    /* 2. prepare arguments for softmmu helper */

    /*
     * build arguemnts for the helper
     *               LOAD     |  STORE
     * ---------------------------------------
     * helper arg0 : env      |  env
     * helper arg1 : x86vaddr |  x86vaddr
     * helper arg2 : memop    |  data
     * helper arg3 : retaddr  |  memop
     * helper arg4 :          |  retaddr
     */

    /* 2.1 get mem address */
    int mem_no_offset_new_tmp = 0;
    IR2_OPND mem_no_offset = latxs_convert_mem_ir2_opnd_no_offset(
            &sp->mem_ir2_opnd, &mem_no_offset_new_tmp);
    IR2_OPND mem = latxs_ir2_opnd_mem_get_base(&mem_no_offset);
    /* 2.2 convert memop */
    MemOp memop = convert_to_tcgmemop(sp->op);
    TCGMemOpIdx memopidx = (memop << 4) | sp->mmu_index;

    /* 3.1 arg1: mem address */
    latxs_append_ir2_opnd2_(lisa_mov, arg1, &mem);
    if (mem_no_offset_new_tmp) {
        latxs_ra_free_temp(&mem_no_offset);
    }
    /* 3.2 arg0 : env */
    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
    /* 3.3 arg2 : memop(LOAD) data(STORE) */
    if (sp->is_load) {
        latxs_append_ir2_opnd2i(LISA_ORI, arg2, zero, memopidx);
    } else {
        latxs_append_ir2_opnd2_(lisa_mov, arg2, &sp->gpr_ir2_opnd);
    }
    /* 3.4 arg3 : retaddr(LOAD) memop(STORE) */
    if (sp->is_load) {
        latxs_load_imm64_to_ir2(arg3, sp->retaddr);
    } else {
        latxs_append_ir2_opnd2i(LISA_ORI, arg2, zero, memopidx);
        /* 3.5 arg4 : retaddr(STORE) */
        latxs_load_imm64_to_ir2(arg4, sp->retaddr);
    }

    /* 4. call the helper */
    switch (sp->op) {
    case LISA_ST_B:
        latxs_tr_gen_call_to_helper((ADDR)helper_ret_stb_mmu);
        break;
    case LISA_ST_H:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_stw_mmu);
        break;
    case LISA_ST_W:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_stl_mmu);
        break;
    case LISA_ST_D:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_stq_mmu);
        break;
    case LISA_LD_B:
        latxs_tr_gen_call_to_helper((ADDR)helper_ret_ldsb_mmu);
        break;
    case LISA_LD_BU:
        latxs_tr_gen_call_to_helper((ADDR)helper_ret_ldub_mmu);
        break;
    case LISA_LD_H:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldsw_mmu);
        break;
    case LISA_LD_HU:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_lduw_mmu);
        break;
    case LISA_LD_W:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldsl_mmu);
        break;
    case LISA_LD_WU:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldul_mmu);
        break;
    case LISA_LD_D:
        latxs_tr_gen_call_to_helper((ADDR)helper_le_ldq_mmu);
        break;
    default:
        lsassert(0);
        break;
    }

    /* 6. restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(cfg);

    if (option_by_hand) {
        for (i = 0; i < CPU_NB_REGS; ++i) {
            gpr_em_bak[i] = td->reg_exmode[i];
            gpr_eb_bak[i] = td->reg_exbits[i];
            td->reg_exmode[i] = sp->reg_exmode[i];
            td->reg_exbits[i] = sp->reg_exbits[i];
        }
        /*
         * Since slow path is generated at the end of TB,
         * it does not need to manage GPR's EM like translation.
         *
         * Instead, it need to keep the GPR's EM equal to
         * the original EM stored from fast path.
         */
        latxs_tr_setto_extmb_after_cs(0xFF);
        /*
         * The purpose of save/restore the EM in TRANSLATION_DATA
         * is NOT to manage the GPR's EM. It is mean to make the
         * slow path generation function does not have side effects.
         *
         * This should be good for other modification in the future.
         */
        for (i = 0; i < CPU_NB_REGS; ++i) {
            td->reg_exmode[i] = gpr_em_bak[i];
            td->reg_exbits[i] = gpr_eb_bak[i];
        }
    }

    if (sp->tmp_need_save) {
        latxs_tr_restore_temp_register_mask(sp->tmp_mask);
    }

    if (sp->is_load) {
        latxs_append_ir2_opnd2_(lisa_mov, &sp->gpr_ir2_opnd,
                &latxs_ret0_ir2_opnd);
    }

    td->itemp_mask = tmp_mask_bak;

    if (!option_lsfpu) {
        latxs_td_fpu_set_top(top_bak);
    }

    latxs_append_ir2_opnd1(LISA_B, &sp->label_exit);
}

void tr_gen_softmmu_slow_path(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    td->in_gen_slow_path = 1;

    int i = 0;
    int sp_nr = td->slow_path_rcd_nr;

    softmmu_sp_rcd_t *sp = NULL;
    softmmu_sp_rcd_t *sp_array = td->slow_path_rcd;

    for (i = 0; i < sp_nr; ++i) {
        sp = &sp_array[i];
        __tr_gen_softmmu_sp_rcd(sp);
    }

    td->in_gen_slow_path = 0;
}

void gen_ldst_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_gpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    if (is_ldst_realized_by_softmmu(op)) {
        __gen_ldst_softmmu_helper_native(op, opnd_gpr, opnd_mem, save_temp);
        return;
    }

    lsassertm(0, "Softmmu not support non-load/store instruction.");
}

void gen_ldst_c1_softmmu_helper(
        IR2_OPCODE op,
        IR2_OPND *opnd_fpr,
        IR2_OPND *opnd_mem,
        int save_temp)
{
    IR2_OPCODE ldst_op = LISA_INVALID;
    IR2_OPCODE mfmt_op = LISA_INVALID;

    switch (op) {
    case LISA_FLD_S:
        ldst_op = LISA_LD_WU;
        mfmt_op = LISA_MOVGR2FR_W;
        break;
    case LISA_FLD_D:
        ldst_op = LISA_LD_D;
        mfmt_op = LISA_MOVGR2FR_D;
        break;
    case LISA_FST_S:
        ldst_op = LISA_ST_W;
        mfmt_op = LISA_MOVFR2GR_S;
        break;
    case LISA_FST_D:
        ldst_op = LISA_ST_D;
        mfmt_op = LISA_MOVFR2GR_D;
        break;
    default:
        lsassert(0);
        break;
    }

    IR2_OPND tmp = ra_alloc_itemp();
    if (latxs_ir2_opcode_is_load(op)) {
        /* 1. load to GPR from memory */
        gen_ldst_softmmu_helper(ldst_op, &tmp, opnd_mem, save_temp);
        /* 2. move GPR to FPR */
        latxs_append_ir2_opnd2(mfmt_op, opnd_fpr, &tmp);
    } else {
        /* 1. move FPR to GPR */
        latxs_append_ir2_opnd2(mfmt_op, &tmp, opnd_fpr);
        /* 2. store GPR to to memory */
        gen_ldst_softmmu_helper(ldst_op, &tmp, opnd_mem, save_temp);
    }
}
