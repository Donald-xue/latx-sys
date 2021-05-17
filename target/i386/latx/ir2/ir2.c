#include "common.h"
#include "ir2.h"
#include "ir1.h"
#include "env.h"
#include "reg-alloc.h"
#include <string.h>

#include "ir2-la-name.h"
IR2_OPND sp_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 3,  ._reg_num = 3,  ._addr = 0};
IR2_OPND fp_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 22, ._reg_num = 22, ._addr = 0};
IR2_OPND zero_ir2_opnd = {._type = IR2_OPND_GPR, .val = 0,  ._reg_num = 0,  ._addr = 0};
IR2_OPND env_ir2_opnd = {._type = IR2_OPND_GPR,  .val = 25, ._reg_num = 25, ._addr = 0};
IR2_OPND n1_ir2_opnd = {._type = IR2_OPND_IREG, .val = 23, ._reg_num = 23, ._addr = 0};
IR2_OPND a0_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 4,  ._reg_num = 4,  ._addr = 0};
IR2_OPND t5_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 17,  ._reg_num = 17,  ._addr = 0};
IR2_OPND ra_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 1,  ._reg_num = 1,  ._addr = 0};
/* TODO: interesting */
#warning FIXME:Is f32_ir2_opnd Num 8 ???
IR2_OPND f32_ir2_opnd = {._type = IR2_OPND_FPR,  .val = 8, ._reg_num = 8,  ._addr = 0};

/*
 * FIXME: For LA, IR2_OPND didn't need reg_num and addr, val is enough for LA
 */
IR2_OPND fcsr_ir2_opnd = {._type = IR2_OPND_CREG, ._reg_num = 0, ._addr = 0};
IR2_OPND fcc0_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 0, ._addr = 0};
IR2_OPND fcc1_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 1, ._addr = 0};
IR2_OPND fcc2_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 2, ._addr = 0};
IR2_OPND fcc3_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 3, ._addr = 0};
IR2_OPND fcc4_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 4, ._addr = 0};
IR2_OPND fcc5_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 5, ._addr = 0};
IR2_OPND fcc6_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 6, ._addr = 0};
IR2_OPND fcc7_ir2_opnd = {._type = IR2_OPND_CC, ._reg_num = 7, ._addr = 0};

void ir2_opnd_build_none(IR2_OPND *opnd)
{
    opnd->_type = IR2_OPND_NONE;
    opnd->_reg_num = 0;
    opnd->_addr = 0;
}

IR2_OPND ir2_opnd_new_none(void)
{
    IR2_OPND opnd;
    ir2_opnd_build_none(&opnd);
    return opnd;
}

void ir2_opnd_build_type(IR2_OPND *opnd, IR2_OPND_TYPE t)
{
    lsassert(t == IR2_OPND_LABEL);
    opnd->_type = t;
    opnd->_label_id = 0;
    opnd->_addr = ++(lsenv->tr_data->label_num);
}

IR2_OPND ir2_opnd_new_type(IR2_OPND_TYPE t)
{
    IR2_OPND opnd;
    ir2_opnd_build_type(&opnd, t);
    return opnd;
}

void ir2_opnd_build(IR2_OPND *opnd, IR2_OPND_TYPE t, int value)
{
    if (t == IR2_OPND_IREG || t == IR2_OPND_FREG || t == IR2_OPND_CREG) {
        opnd->_type = t;
        opnd->_reg_num = value;
        opnd->_addr = 0;
    } else if (t == IR2_OPND_IMM) {
        opnd->_type = t;
        opnd->_reg_num = 0;
        opnd->_addr = 0;
        opnd->_imm16 = value;
    } else if (t == IR2_OPND_ADDR) {
        opnd->_type = t;
        opnd->_reg_num = 0;
        opnd->_addr = value;
    } else {
        printf("[LATX] [error] not implemented in %s : %d", __func__,
               __LINE__);
        exit(-1);
    }
    /*
     * To make LA backend happy.
     */
    opnd->val = value;
}

IR2_OPND ir2_opnd_new(IR2_OPND_TYPE type, int value)
{
    IR2_OPND opnd;

    ir2_opnd_build(&opnd, type, value);

    return opnd;
}

void ir2_opnd_build2(IR2_OPND *opnd, IR2_OPND_TYPE type, int base, int16 offset)
{
    lsassert(type == IR2_OPND_MEM);
    opnd->_type = type;
    opnd->val = base;
    opnd->_reg_num = base;
    opnd->_imm16 = offset;
}

IR2_OPND ir2_opnd_new2(IR2_OPND_TYPE type, int base, int16 offset)
{
    IR2_OPND opnd;

    ir2_opnd_build2(&opnd, type, base, offset);

    return opnd;
}

int16 ir2_opnd_imm(IR2_OPND *opnd) { return opnd->_imm16; }

int ir2_opnd_is_ireg(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_IREG; }

int ir2_opnd_is_freg(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_FREG; }

int ir2_opnd_is_creg(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_CREG; }

int32 ir2_opnd_label_id(IR2_OPND *opnd) { return opnd->_label_id; }

int ir2_opnd_is_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_ireg(opnd) && ir2_opnd_base_reg_num(opnd) > 31;
}

int ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_freg(opnd) && ir2_opnd_base_reg_num(opnd) > 31;
}

int ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_mem(opnd) && ir2_opnd_base_reg_num(opnd) > 31;
}

int ir2_opnd_is_mem(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_MEM; }

int ir2_opnd_is_imm(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_IMM; }

int ir2_opnd_is_label(IR2_OPND *opnd) { return opnd->_type == IR2_OPND_LABEL; }

int ir2_opnd_base_reg_num(IR2_OPND *opnd) { return opnd->val; }

IR2_OPND_TYPE ir2_opnd_type(IR2_OPND *opnd)
{
    return (IR2_OPND_TYPE)opnd->_type;
}

int ir2_opnd_cmp(IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    return opnd1->_type == opnd2->_type && opnd1->val == opnd2->val;
}

void ir2_opnd_convert_label_to_imm(IR2_OPND *opnd, int imm)
{
    lsassert(ir2_opnd_is_label(opnd));
    opnd->_type = IR2_OPND_IMM;
    opnd->_imm16 = imm;
    opnd->val = imm;
}

int ir2_opnd_to_string(IR2_OPND *opnd, char *str, bool hex)
{
    int base_reg_num = ir2_opnd_base_reg_num(opnd);

    switch (ir2_opnd_type(opnd)) {
    case IR2_OPND_INV:
        return 0;
    case IR2_OPND_GPR: {
        if (ir2_opnd_is_itemp(opnd)) {
            return sprintf(str, "\033[3%dmitmp%d\033[m", base_reg_num % 6 + 1,
                           base_reg_num);
        } else {
            strcpy(str, ir2_name(base_reg_num));
            return strlen(str);
        }
    }
    case IR2_OPND_FPR: {
        if (ir2_opnd_is_ftemp(opnd)) {
            return sprintf(str, "\033[3%dmftmp%d\033[m", base_reg_num % 6 + 1,
                           base_reg_num);
        } else {
            strcpy(str, ir2_name(40 + base_reg_num));
            return strlen(str);
        }
    }
    case IR2_OPND_FCSR: {
        return sprintf(str, "$c%d", base_reg_num);
    }
    case IR2_OPND_CC: {
        return sprintf(str, "$c%d", base_reg_num);
    }
    //case IR2_OPND_MEM: {
    //    if (ir2_opnd_imm(opnd) > -1024 && ir2_opnd_imm(opnd) < 1024) {
    //        if (ir2_opnd_is_mem_base_itemp(opnd)) {
    //            return sprintf(str, "%d(\033[3%dmitmp%d\033[m)",
    //                           ir2_opnd_imm(opnd), base_reg_num % 6 + 1,
    //                           base_reg_num);
    //        } else {
    //            return sprintf(str, "%d(%s)", ir2_opnd_imm(opnd),
    //                           ir2_name(base_reg_num));
    //        }
    //    } else {
    //        if (ir2_opnd_is_mem_base_itemp(opnd)) {
    //            return sprintf(str, "0x%x(\033[3%dmitmp%d\033[m)",
    //                           ir2_opnd_imm(opnd), base_reg_num % 6 + 1,
    //                           base_reg_num);
    //        } else {
    //            return sprintf(str, "0x%x(%s)", ir2_opnd_imm(opnd),
    //                           ir2_name(base_reg_num));
    //        }
    //    }
    //}
    case IR2_OPND_IMMD: {
            return sprintf(str, "%d", ir2_opnd_imm(opnd));
    }
    case IR2_OPND_IMMH: {
        if(hex)
            return sprintf(str, "0x%x", (uint16)ir2_opnd_imm(opnd));
        else
            return sprintf(str, "%d", ir2_opnd_imm(opnd));
    }
    //case IR2_OPND_ADDR:
    //    return sprintf(str, "0x%x", ir2_opnd_addr(opnd));
    case IR2_OPND_LABEL:
        return sprintf(str, "LABEL %d", ir2_opnd_imm(opnd));
    default:
        lsassertm(0, "type = %d\n", ir2_opnd_type(opnd));
        return 0;
    }
}

bool ir2_opcode_is_load(IR2_OPCODE opcode)
{
    if (opcode >= mips_ld && opcode <= mips_lld) {
        return true;
    }
    if (opcode >= mips_lb && opcode <= mips_lwu && opcode != mips_lui) {
        return true;
    }
    if (opcode == mips_ldc1 || opcode == mips_lwc1) {
        return true;
    }
    if (opcode >= mips_gs_lq && opcode <= mips_gs_ldrc1) {
        return true;
    }
    if (opcode >= mips_ldb && opcode <=mips_ldd){
        return true;
    }
    if (opcode == mips_pref || opcode == mips_prefx) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_load_rl(IR2_OPCODE opcode)
{
    if (opcode == mips_lb || opcode == mips_lbu) {
        return true;
    }
    if (opcode == mips_ldl || opcode == mips_ldr) {
        return true;
    }
    if (opcode == mips_lwl || opcode == mips_lwr) {
        return true;
    }
    if (opcode == mips_gs_ldlc1 || opcode == mips_gs_ldrc1) {
        return true;
    }
    if (opcode == mips_gs_lwlc1 || opcode == mips_gs_lwrc1) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_load_not_rl(IR2_OPCODE opcode)
{
    return ir2_opcode_is_load(opcode) && !ir2_opcode_is_load_rl(opcode);
}

bool ir2_opcode_is_store(IR2_OPCODE opcode)
{
    if (opcode >= mips_sdi && opcode <= mips_sdr) {
        return true;
    }
    if (opcode >= mips_sw && opcode <= mips_swr) {
        return true;
    }
    if (opcode == mips_sb || opcode == mips_sh || opcode == mips_sc) {
        return true;
    }
    if (opcode == mips_sdc1 || opcode == mips_swc1) {
        return true;
    }
    if (opcode >= mips_gs_sq && opcode <= mips_gs_sdrc1) {
        return true;
    }
    if (opcode >= mips_stb && opcode <=mips_std){
        return true;
    }
    return false;
}

bool ir2_opcode_is_store_rl(IR2_OPCODE opcode)
{
    if (opcode == mips_sb) {
        return true;
    }
    if (opcode == mips_sdl || opcode == mips_sdr) {
        return true;
    }
    if (opcode == mips_swl || opcode == mips_swr) {
        return true;
    }
    if (opcode == mips_gs_sdlc1 || opcode == mips_gs_sdrc1) {
        return true;
    }
    if (opcode == mips_gs_swlc1 || opcode == mips_gs_swrc1) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_store_not_rl(IR2_OPCODE opcode)
{
    return ir2_opcode_is_store(opcode) && !ir2_opcode_is_store_rl(opcode);
}

bool ir2_opcode_is_branch(IR2_INS_TYPE opcode)
{
    return (opcode >= LISA_BEQZ && opcode <= LISA_BCNEZ) ||
           (opcode >= LISA_B && opcode <= LISA_BGEU);
}

bool ir2_opcode_is_branch_with_3opnds(IR2_INS_TYPE opcode)
{
    if (opcode >= LISA_BEQ && opcode <= LISA_BGEU) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_f_branch(IR2_INS_TYPE opcode)
{
    return (opcode == LISA_BCEQZ || opcode == LISA_BCNEZ);
}

bool ir2_opcode_is_rri(IR2_OPCODE opcode)
{
    if (ir2_opcode_is_rri_arith(opcode) || ir2_opcode_is_rri_logic(opcode) ||
        ir2_opcode_is_rri_shift(opcode)) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_rri_arith(IR2_OPCODE opcode)
{
    if (opcode == mips_addi || opcode == mips_addiu || opcode == mips_daddi ||
        opcode == mips_daddiu || opcode == mips_subiu ||
        opcode == mips_dsubiu || opcode == mips_slti || opcode == mips_sltiu) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_rri_logic(IR2_OPCODE opcode)
{
    if (opcode == mips_andi || opcode == mips_ori || opcode == mips_xori) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_rri_shift(IR2_OPCODE opcode)
{
    if (opcode == mips_sll || opcode == mips_srl || opcode == mips_sra ||
        opcode == mips_dsll || opcode == mips_dsrl || opcode == mips_dsra ||
        opcode == mips_dsll32 || opcode == mips_dsrl32 ||
        opcode == mips_dsra32) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_convert(IR2_INS_TYPE opcode)
{
    switch (opcode) {
    case LISA_FCVT_D_S:
    case LISA_FCVT_S_D:
    case LISA_FFINT_D_W:
    case LISA_FFINT_D_L:
    case LISA_FFINT_S_W:
    case LISA_FFINT_S_L:
    case LISA_FTINT_L_D:
    case LISA_FTINT_L_S:
    case LISA_FTINT_W_D:
    case LISA_FTINT_W_S:
        return true;
    default:
        return false;
    }
}

bool ir2_opcode_is_fcmp(IR2_INS_TYPE opcode)
{
    return (opcode == LISA_FCMP_COND_S || opcode == LISA_FCMP_COND_D);
}

static IR2_INS_TYPE ir2_opcode_rrr_to_rri(IR2_INS_TYPE opcode)
{
    switch (opcode) {
    case LISA_ADDI_ADDR:
    case LISA_ADD_ADDR:
        return LISA_ADDI_ADDR;
    case LISA_ADDI_ADDRX:
    case LISA_ADD_ADDRX:
        return LISA_ADDI_ADDRX;
    case LISA_SUBI_ADDR:
    case LISA_SUB_ADDR:
        return LISA_SUBI_ADDR;
    case LISA_SUBI_ADDRX:
    case LISA_SUB_ADDRX:
        return LISA_SUBI_ADDRX;
    case LISA_ADDI_W:
    case LISA_ADD_W:
        return LISA_ADDI_W;
    case LISA_ADDI_D:
    case LISA_ADD_D:
        return LISA_ADDI_D;
    case LISA_SUBIU:
    case LISA_SUB_W:
        return LISA_SUBIU;
    case LISA_DSUBIU:
    case LISA_SUB_D:
        return LISA_DSUBIU;
    case LISA_SLTI:
    case LISA_SLT:
        return LISA_SLTI;
    case LISA_SLTUI:
    case LISA_SLTU:
        return LISA_SLTUI;
    case LISA_ANDI:
    case LISA_AND:
        return LISA_ANDI;
    case LISA_ORI:
    case LISA_OR:
        return LISA_ORI;
    case LISA_XORI:
    case LISA_XOR:
        return LISA_XORI;
    case LISA_SLL_W:
    case LISA_SLLI_W:
        return LISA_SLLI_W;
    case LISA_SRL_W:
    case LISA_SRLI_W:
        return LISA_SRLI_W;
    case LISA_SRA_W:
    case LISA_SRAI_W:
        return LISA_SRAI_W;
    default:
        lsassertm(0, "illegal immediate operand used for %s\n",
                  ir2_name(opcode));
        return LISA_INVALID;
    }
}

void ir2_set_id(IR2_INST *ir2, int id) { ir2->_id = id; }

int ir2_get_id(IR2_INST *ir2) { return ir2->_id; }

IR2_INS_TYPE ir2_opcode(IR2_INST *ir2) { return (IR2_INS_TYPE)(ir2->_opcode); }

void ir2_set_opcode(IR2_INST *ir2, IR2_INS_TYPE type) { 
    ir2->_opcode = type;
}

ADDR ir2_addr(IR2_INST *ir2) { return ir2->_addr; }

void ir2_set_addr(IR2_INST *ir2, ADDR a) { ir2->_addr = a; }

uint32 ir2_opnd_addr(IR2_OPND *ir2) { return ir2->_addr; }

IR2_INST *ir2_prev(IR2_INST *ir2)
{
    if (ir2->_prev == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_prev;
    }
}

IR2_INST *ir2_next(IR2_INST *ir2)
{
    if (ir2->_next == -1) {
        return NULL;
    } else {
        return lsenv->tr_data->ir2_inst_array + ir2->_next;
    }
}

int ir2_to_string(IR2_INST *ir2, char *str)
{
    int length = 0;
    int i = 0;
    bool print_hex_imm = false;

    length = sprintf(str, "%-8s  ", ir2_name(ir2_opcode(ir2)));

    if (ir2_opcode(ir2) == LISA_ANDI || ir2_opcode(ir2) == LISA_ORI ||
        ir2_opcode(ir2) == LISA_XORI || ir2_opcode(ir2) == LISA_LU12I_W) {
        print_hex_imm = true;
    }

    for (i = 0; i < ir2->op_count; ++i) {
        if (ir2_opnd_type(&ir2->_opnd[i]) == IR2_OPND_NONE) {
            return length;
        } else {
            if (i > 0) {
                strcat(str, ",");
                length += 1;
            }
            /*length += _opnd[i].to_string(str+length, print_hex_imm);*/
            length +=
                ir2_opnd_to_string(&ir2->_opnd[i], str + length, print_hex_imm);
        }
    }

    return length;
}

int ir2_dump(IR2_INST *ir2)
{
    char str[64];
    int size = 0;

    if (ir2_opcode(ir2) ==
        0) { /* an empty IR2_INST was inserted into the ir2 */
             /* list, but not assigned yet. */
        return 0;
    }

    size = ir2_to_string(ir2, str);

    fprintf(stderr, "%s\n", str);

    return size;
}

void ir2_build(IR2_INST *ir2, IR2_OPCODE opcode, IR2_OPND opnd0, IR2_OPND opnd1,
               IR2_OPND opnd2)
{
    ir2->_opcode = opcode;
    ir2->_opnd[0] = opnd0;
    ir2->_opnd[1] = opnd1;
    ir2->_opnd[2] = opnd2;
    ir2->_opnd[3] = ir2_opnd_new_none();
}

void ir2_append(IR2_INST *ir2)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *former_last = t->last_ir2;

    if (former_last != NULL) {
        lsassert(t->first_ir2 != NULL);
        ir2->_prev = ir2_get_id(former_last);
        ir2->_next = -1;
        t->last_ir2 = ir2;
        former_last->_next = ir2_get_id(ir2);
    } else {
        lsassert(t->first_ir2 == NULL);
        ir2->_prev = -1;
        ir2->_next = -1;
        t->last_ir2 = ir2;
        t->first_ir2 = ir2;
    }

    if(ir2->_opcode >= LISA_GR2SCR)
        t->real_ir2_inst_num++;
}

void ir2_remove(IR2_INST *ir2)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    IR2_INST *next = ir2_next(ir2);
    IR2_INST *prev = ir2_prev(ir2);

    if (t->first_ir2 == ir2) {
        if (t->last_ir2 == ir2) { /* head and tail */
            t->first_ir2 = NULL;
            t->last_ir2 = NULL;
        } else { /* head but not tail */
            t->first_ir2 = next;
            next->_prev = -1;
        }
    } else if (t->last_ir2 == ir2) { /* tail but not head */
        t->last_ir2 = prev;
        prev->_next = -1;
    } else {
        prev->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(prev);
    }

    ir2->_prev = -1;
    ir2->_next = -1;
}

void ir2_insert_before(IR2_INST *ir2, IR2_INST *next)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (t->first_ir2 == next) {
        t->first_ir2 = ir2;
        ir2->_prev = -1;
        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);
    } else {
        IR2_INST *prev = ir2_prev(next);

        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);

        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);
    }
}

void ir2_insert_after(IR2_INST *ir2, IR2_INST *prev)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    if (t->last_ir2 == prev) {
        t->last_ir2 = ir2;
        ir2->_next = -1;
        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);
    } else {
        IR2_INST *next = ir2_next(prev);

        ir2->_next = ir2_get_id(next);
        next->_prev = ir2_get_id(ir2);

        ir2->_prev = ir2_get_id(prev);
        prev->_next = ir2_get_id(ir2);
    }
}

static IR2_INST *ir2_allocate(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    /* 1. make sure we have enough space */
    if (t->ir2_inst_num_current == t->ir2_inst_num_max) {
        /* printf("[LATX] [error] not implemented in %s : %d", __func__, */
        /* __LINE__); exit(-1); 1.1. current array size in bytes */
        int bytes = sizeof(IR2_INST) * t->ir2_inst_num_max;

        /* 1.2. double the array */
        t->ir2_inst_num_max *= 2;
        IR2_INST *back_ir2_inst_array = t->ir2_inst_array;
        t->ir2_inst_array =
            (IR2_INST *)mm_realloc(t->ir2_inst_array, bytes << 1);
        t->first_ir2 =
            (IR2_INST *)((ADDR)t->first_ir2 - (ADDR)back_ir2_inst_array +
                         (ADDR)t->ir2_inst_array);
        t->last_ir2 =
            (IR2_INST *)((ADDR)t->last_ir2 - (ADDR)back_ir2_inst_array +
                         (ADDR)t->ir2_inst_array);
    }

    /* 2. allocate one */
    IR2_INST *p = t->ir2_inst_array + t->ir2_inst_num_current;
    ir2_set_id(p, t->ir2_inst_num_current);
    t->ir2_inst_num_current++;

    /* 3. return it */
    return p;
}

static inline EXTENSION_MODE int16_em(int16 imm)
{
    if (imm >= 0) {
        return ZERO_EXTENSION;
    } else {
        return SIGN_EXTENSION;
    }
}

static inline int int16_eb(int16 imm)
{
    int i = 0;
    if (imm >= 0) { /* where is the first "1"? */
        for (i = 14; i >= 0; --i) {
            if (BITS_ARE_SET(imm, 1 << i)) {
                return i + 1;
            }
        }
        return 0; /* no "1", the value is zero */
    } else {
        for (i = 14; i >= 0; --i) { /* where is the first "0"? */
            if (BITS_ARE_CLEAR(imm, 1 << i)) {
                return i + 2;
            }
        }
        return 1; /* no "0", the value is -1 */
    }
}

static inline int uint16_eb(uint16 imm)
{
    int i = 0;
    for (i = 15; i >= 0; --i) {
        if (BITS_ARE_SET(imm, 1 << i)) {
            return i + 1;
        }
    }
    return 0;
}

IR2_INST *append_ir2_opnd3(IR2_OPCODE opcode, IR2_OPND opnd0, IR2_OPND opnd1,
                           IR2_OPND opnd2)
{

    if (ir2_opnd_is_imm(&opnd2)) {
        IR2_OPCODE new_opcode = ir2_opcode_rrr_to_rri(opcode);
        return append_ir2_opnd2i(new_opcode, opnd0, opnd1,
                                 ir2_opnd_imm(&opnd2));
    }

    IR2_INST *p = ir2_allocate();
    ir2_append(p); /* insert into tr_data's ir2 array */

    switch (opcode) {
    case mips_add_addr:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
#ifdef N64 /* validate address */
        if (ir2_opnd_is_mips_address(&opnd1)) {
            lsassert(ir2_opnd_is_sx(&opnd2, 32) &&
                     !ir2_opnd_is_mips_address(&opnd2));
            ir2_build(p, mips_daddu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_mips_address(&opnd2)) {
            lsassert(ir2_opnd_is_sx(&opnd1, 32) &&
                     !ir2_opnd_is_mips_address(&opnd1));
            ir2_build(p, mips_daddu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32)) {
            ir2_build(p, mips_addu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
            if (!ir2_opnd_is_sx(&opnd0, 32)) {
                ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_build(p, mips_daddu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
#else
        lsassert(ir2_opnd_is_sx(&opnd1) && opnd2.is_sx());
        ir2_build(p, mips_addu, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32))
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_add_addrx:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
#ifdef N64 /* validate address */
        if (ir2_opnd_is_x86_address(&opnd1)) {
            lsassert(!ir2_opnd_is_x86_address(&opnd2));
            ir2_build(p, mips_daddu, opnd0, opnd1, opnd2);
            if (ir2_opnd_is_sx(&opnd2, 32)) {
                ir2_opnd_set_em(&opnd0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
            }
        } else if (ir2_opnd_is_x86_address(&opnd2)) {
            lsassert(!ir2_opnd_is_x86_address(&opnd1));
            ir2_build(p, mips_daddu, opnd0, opnd1, opnd2);
            if (ir2_opnd_is_sx(&opnd1, 32)) {
                ir2_opnd_set_em(&opnd0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
            }
        } else if (ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32)) {
            ir2_build(p, mips_addu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
            if (!ir2_opnd_is_sx(&opnd0, 32)) {
                ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_build(p, mips_addu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
#else
        ir2_build(p, mips_addu, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32))
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_sub_addr:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
#ifdef N64 /* validate address */
        if (ir2_opnd_is_mips_address(&opnd1) &&
            ir2_opnd_is_mips_address(&opnd2)) {
            ir2_build(p, mips_dsubu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        } else if (ir2_opnd_is_mips_address(&opnd1) &&
                   ir2_opnd_is_sx(&opnd2, 32)) {
            ir2_build(p, mips_dsubu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32)) {
            ir2_build(p, mips_subu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em_sub2(&opnd0, &opnd1, &opnd2);
            if (!ir2_opnd_is_sx(&opnd0, 32)) {
                ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_build(p, mips_dsubu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
#else
        lsassert(ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32));
        ir2_build(p, mips_subu, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_sub2(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32))
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_sub_addrx:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
#ifdef N64 /* validate address */
        if (ir2_opnd_is_x86_address(&opnd1) &&
            ir2_opnd_is_x86_address(&opnd2)) {
            ir2_build(p, mips_dsubu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        } else if (ir2_opnd_is_x86_address(&opnd1) &&
                   ir2_opnd_is_sx(&opnd2, 32)) {
            ir2_build(p, mips_dsubu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32)) {
            ir2_build(p, mips_subu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em_sub2(&opnd0, &opnd1, &opnd2);
            if (!ir2_opnd_is_sx(&opnd0, 32)) {
                ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_build(p, mips_dsubu, opnd0, opnd1, opnd2);
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
#else
        lsassert(ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32));
        ir2_build(p, mips_subu, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_sub2(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32))
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_daddu:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
        break;
    case mips_dsubu:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_sub2(&opnd0, &opnd1, &opnd2);
        break;
    case mips_addu:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        lsassert(ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_add2(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32)) {
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        }
        break;
    case mips_subu:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        lsassert(ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_sub2(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32)) {
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        }
        break;
    case mips_and:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        if (ir2_opnd_is_x86_address(&opnd1) ||
            ir2_opnd_is_x86_address(&opnd2)) {
            ir2_opnd_set_em(&opnd0, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_mips_address(&opnd1) ||
                   ir2_opnd_is_mips_address(&opnd2)) {
            ir2_opnd_set_em(&opnd0, EM_MIPS_ADDRESS, 32);
        } else {
            ir2_opnd_set_em_and2(&opnd0, &opnd1, &opnd2);
        }
        break;
    case mips_or:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        if (ir2_opnd_is_ax(&opnd1, 32) || ir2_opnd_is_ax(&opnd2, 32)) {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
        if (ir2_opnd_is_mips_address(&opnd1) ||
            ir2_opnd_is_mips_address(&opnd2)) {
            /*if (opnd1.is_mips_address() && opnd2 == zero_ir2_opnd)*/
            if (ir2_opnd_is_mips_address(&opnd1) &&
                ir2_opnd_cmp(&opnd2, &zero_ir2_opnd)) {
                ir2_opnd_set_em_mov(&opnd0, &opnd1);
            } else {
                lsassertm(0, "not implemented yet\n");
            }
        } else if (ir2_opnd_is_x86_address(&opnd1)) {
            lsassert(!ir2_opnd_is_address(&opnd2));
            if (ir2_opnd_is_zx(&opnd2, 32)) {
                ir2_opnd_set_em(&opnd0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
            }
        } else if (ir2_opnd_is_x86_address(&opnd2)) {
            lsassert(!ir2_opnd_is_address(&opnd1));
            if (ir2_opnd_is_zx(&opnd1, 32)) {
                ir2_opnd_set_em(&opnd0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_opnd_set_em_or2(&opnd0, &opnd1, &opnd2);
        }
        break;
    case mips_nor:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        if (ir2_opnd_is_ax(&opnd1, 32) || ir2_opnd_is_ax(&opnd2, 32)) {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&opnd1) || ir2_opnd_is_address(&opnd2)) {
            lsassertm(0, "not implemented yet\n");
        } else {
            ir2_opnd_set_em_nor2(&opnd0, &opnd1, &opnd2);
        }
        break;
    case mips_xor:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        if (ir2_opnd_is_ax(&opnd1, 32) || ir2_opnd_is_ax(&opnd2, 32)) {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_xor2(&opnd0, &opnd1, &opnd2);
        }
        break;
    case mips_sllv:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        break;
    case mips_dsllv:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        break;
    case mips_srlv:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        lsassert(ir2_opnd_is_sx(&opnd1, 32));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        break;
    case mips_dsrlv:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        if (ir2_opnd_is_ax(&opnd1, 32)) {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&opnd1)) {
            lsassertm(0, "not implemented yet\n");
        } else if (ir2_opnd_is_zx(&opnd1, 63)) {
            ir2_opnd_set_em_mov(&opnd0, &opnd1);
        } else {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case mips_srav:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        lsassert(ir2_opnd_is_sx(&opnd1, 32));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        break;
    case mips_dsrav:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        if (ir2_opnd_is_ax(&opnd1, 32)) {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&opnd1)) {
            lsassertm(0, "not implemented yet\n");
        } else if (ir2_opnd_is_sx(&opnd1, 63)) {
            ir2_opnd_set_em_mov(&opnd0, &opnd1);
        } else {
            ir2_opnd_set_em(&opnd0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case mips_sltu:
    case mips_slt:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em(&opnd0, ZERO_EXTENSION, 1);
        break;
    case mips_dmult_g:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_dmult_g(&opnd0, &opnd1, &opnd2);
        break;
    case mips_dmultu_g:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_dmultu_g(&opnd0, &opnd1, &opnd2);
        break;
    case mips_mult_g:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_dmult_g(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32)) {
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        }
        break;
    case mips_multu_g:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        lsassert(ir2_opnd_is_sx(&opnd1, 32) && ir2_opnd_is_sx(&opnd2, 32));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        ir2_opnd_set_em_dmultu_g(&opnd0, &opnd1, &opnd2);
        if (!ir2_opnd_is_sx(&opnd0, 32)) {
            ir2_opnd_set_em(&opnd0, SIGN_EXTENSION, 32);
        }
        break;
    case mips_movz:
    case mips_movn:
        lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                 ir2_opnd_is_ireg(&opnd2));
        ir2_build(p, opcode, opnd0, opnd1, opnd2);
        // ir2_opnd_set_em_movcc(&opnd0, &opnd1, &opnd2);
        // arg error
        ir2_opnd_set_em_movcc(&opnd0, &opnd1, &opnd0);
        break;
    default:
        if (ir2_opcode_is_branch(opcode)) {
            lsassert(ir2_opnd_is_ireg(&opnd0) && ir2_opnd_is_ireg(&opnd1) &&
                     ir2_opnd_is_label(&opnd2));
            ir2_build(p, opcode, opnd0, opnd1, opnd2);
            p = append_ir2_opnd0(mips_nop);
        } else if (ir2_opnd_is_freg(&opnd0)) {
            lsassert(ir2_opnd_is_freg(&opnd1) && ir2_opnd_is_freg(&opnd2));
            ir2_build(p, opcode, opnd0, opnd1, opnd2);
        } else {
            lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                      ir2_name(opcode));
        }
        break;
    }

    return p;
}

IR2_INST *append_ir2_opnd2i(IR2_OPCODE opcode, IR2_OPND dest, IR2_OPND src,
                            int32 imm)
{

    int base_ireg_num = ir2_opnd_base_reg_num(&src);

#ifdef N64 /* validate address */
    if (ir2_opcode_is_load(opcode) || ir2_opcode_is_store(opcode)) {
        base_ireg_num = em_validate_address(base_ireg_num);
    }
#endif

    IR2_INST *p = ir2_allocate();
    ir2_append(p);

    IR2_OPND ir2_opnd_tmp;

    switch (opcode) {
    case mips_load_addr:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
#ifdef N64 /* validate address */
        ir2_build(p, mips_ld, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
#else
        ir2_build(p, mips_lw, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_load_addrx:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
#ifdef N64 /* validate address */
        ir2_build(p, mips_lwu, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
#else
        ir2_build(p, mips_lw, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_store_addr:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
#ifdef N64 /* validate address */
        ir2_build(p, mips_sdi, dest, ir2_opnd_tmp, ir2_opnd_new_none());
#else
        ir2_build(p, mips_sw, dest, ir2_opnd_tmp, ir2_opnd_new_none());
#endif
        break;
    case mips_store_addrx:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, mips_sw, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        break;
    case mips_subi_addr:
        opcode = mips_addi_addr;
        imm = -imm;
    case mips_addi_addr:
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
#ifdef N64 /* validate address */
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (ir2_opnd_is_mips_address(&src)) {
            ir2_build(p, mips_daddiu, dest, src, ir2_opnd_tmp);
            ir2_opnd_set_em(&dest, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&src, 32)) {
            ir2_build(p, mips_addiu, dest, src, ir2_opnd_tmp);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
            if (!ir2_opnd_is_sx(&dest, 32)) {
                ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_build(p, mips_daddiu, dest, src, ir2_opnd_tmp);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        }
#else
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&src));
        ir2_build(p, mips_addiu, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        if (!ir2_opnd_is_sx(dest, 32))
            ir2_opnd_set_em(dest, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_subi_addrx:
        opcode = mips_addi_addrx;
        imm = -imm;
    case mips_addi_addrx:
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
#ifdef N64 /* validate address */
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        if (ir2_opnd_is_x86_address(&src)) {
            ir2_build(p, mips_daddiu, dest, src, ir2_opnd_tmp);
            ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&src, 32)) {
            ir2_build(p, mips_addiu, dest, src, ir2_opnd_tmp);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
            if (!ir2_opnd_is_sx(&dest, 32)) {
                ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_build(p, mips_daddiu, dest, src, ir2_opnd_tmp);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        }
#else
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&src));
        ir2_build(p, mips_addiu, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        if (!ir2_opnd_is_sx(&dest, 32))
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
#endif
        break;
    case mips_ld:
    case mips_ldr:
    case mips_ldl:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case mips_lw:
    case mips_lwr:
    case mips_lwl:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case mips_lh:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 16);
        break;
    case mips_lb:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 8);
        break;
    case mips_lwu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 32);
        break;
    case mips_lhu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 16);
        break;
    case mips_lbu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 8);
        break;
    case mips_ldc1:
    case mips_lwc1:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        break;
    case mips_sdi:
    case mips_sdr:
    case mips_sdl:
    case mips_sw:
    case mips_swr:
    case mips_swl:
    case mips_sh:
    case mips_sb:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        break;
    case mips_sdc1:
    case mips_swc1:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_opnd_build2(&ir2_opnd_tmp, IR2_OPND_MEM, base_ireg_num, imm);
        ir2_build(p, opcode, dest, ir2_opnd_tmp, ir2_opnd_new_none());
        break;
    case mips_subiu:
        opcode = mips_addiu;
        imm = -imm;
    case mips_addiu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&src, 32));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case mips_dsubiu:
        opcode = mips_daddiu;
        imm = -imm;
    case mips_daddiu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        break;
    case mips_daddi:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        break;
    case mips_slti:
    case mips_sltiu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 1);
        break;
    case mips_andi:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_and(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case mips_ori:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_or(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case mips_xori:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_xor(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case mips_sll:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_dsll(&dest, &src, imm);
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case mips_sra:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&src, 32));
        lsassert((uint32)imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        if (!ir2_opnd_is_sx(&src, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_mov(&dest, &src);
        }
        ir2_opnd_set_em_dsra(&dest, &dest, imm); /* may be a problem */
        lsassert(ir2_opnd_is_sx(&dest, 32));
        break;
    case mips_srl:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&src, 32));
        lsassert((uint32)imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        if (!ir2_opnd_is_zx(&src, 32)) {
            ir2_opnd_set_em(&dest, ZERO_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_mov(&dest, &src);
        }
        ir2_opnd_set_em_dsrl(&dest, &dest, imm);
        lsassert(ir2_opnd_is_sx(&dest, 32));
        break;
    case mips_dsll:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_dsll(&dest, &src, imm);
        break;
    case mips_dsll32:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_build(p, opcode, dest, src, ir2_opnd_new(IR2_OPND_IMM, imm));
        ir2_opnd_set_em_dsll(&dest, &src, 32 + imm);
        break;
    case mips_dsra:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_dsra(&dest, &src, imm);
        break;
    case mips_dsra32:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_build(p, opcode, dest, src, ir2_opnd_new(IR2_OPND_IMM, imm));
        ir2_opnd_set_em_dsra(&dest, &src, 32 + imm);
        break;
    case mips_dsrl:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        ir2_opnd_set_em_dsrl(&dest, &src, imm);
        break;
    case mips_dsrl32:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_build(p, opcode, dest, src, ir2_opnd_new(IR2_OPND_IMM, imm));
        ir2_opnd_set_em_dsrl(&dest, &src, 32 + imm);
        break;
    case mips_ldb:
    case mips_ldh:
    case mips_ldw:
    case mips_ldd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -512 && imm <= 511);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_stb:
    case mips_sth:
    case mips_stw:
    case mips_std:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(imm >= -512 && imm <= 511);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_vclrstriv:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(imm >= 0 && imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_shfb:
    case mips_shfh:
    case mips_shfw:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xffffff00) == 0);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insveb:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=15);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insveh:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=7);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insvew:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=3);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insved:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=1);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insertb:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(0 <= imm && imm <=15);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_inserth:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(0 <= imm && imm <=7);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insertw:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(0 <= imm && imm <=3);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_insertd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(0 <= imm && imm <=1);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_sb:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=15);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_sh:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=7);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_sw:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=3);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_sd:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=1);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_ub:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=15);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_uh:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=7);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_copy_uw:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=3);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_shfd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xffffff00) == 0);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_sldib:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xfffffff0) == 0);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_vshufirh:
    case mips_vshufilh:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xffffff00) == 0);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_andib:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xffffff00) == 0);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_vsrains_sb:
    case mips_vsrains_sh:
    case mips_vsrains_ub:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_sllib:
    case mips_srlib:
    case mips_sraib:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(imm >=0 && imm <= 7);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_sllih:
    case mips_srlih:
    case mips_sraih:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(imm >=0 && imm <= 15);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_slliw:
    case mips_srliw:
    case mips_sraiw:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(imm >=0 && imm <= 31);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_sllid:
    case mips_srlid:
    case mips_sraid:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(imm >=0 && imm <= 63);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_splatib:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=15);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_splatih:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=7);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_splatiw:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=3);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case mips_splatid:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert(0 <= imm && imm <=1);
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    default:
        printf("[LATX] [error] not implemented in %s : %d", __func__,
               __LINE__);
        exit(-1);
        break;
    }

#ifdef N64
    if (base_ireg_num != ir2_opnd_base_reg_num(&src)) {
        ra_free_itemp(base_ireg_num);
    }
#endif

    return p;
}

IR2_INST *append_ir2_opnd2(IR2_OPCODE opcode, IR2_OPND dest, IR2_OPND src)
{

    IR2_OPND ir2_opnd_tmp;

    if (ir2_opcode_is_load(opcode) || ir2_opcode_is_store(opcode)) {
        ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IREG,
                       ir2_opnd_base_reg_num(&src));
        return append_ir2_opnd2i(opcode, dest, ir2_opnd_tmp,
                                 ir2_opnd_imm(&src));
    } else if (opcode == mips_mov64) {
        if (ir2_opnd_cmp(&dest, &src)) {
            return NULL;
        } else {
            IR2_INST *p = append_ir2_opnd3(mips_or, dest, src, zero_ir2_opnd);
            ir2_opnd_set_em_mov(&dest, &src);
            return p;
        }
    } else if (opcode == mips_mov32_sx) {
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_sx(&src, 32)) {
            return NULL;
        } else {
            return append_ir2_opnd2i(mips_sll, dest, src, 0);
        }
    } else if (opcode == mips_mov32_zx) {
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_zx(&src, 32)) {
            return NULL;
        } else {
            return append_ir2_opnd3(mips_and, dest, src, n1_ir2_opnd);
        }
    } else if (opcode == mips_mov_addrx) {
#ifdef N64
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_zx(&src, 32)) {
            if (!(ir2_opnd_cmp(&dest, &zero_ir2_opnd))) {
                ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
            }
            return NULL;
        } else {
            IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
            if (pir1 != NULL) {
                BITS_SET(pir1->flags, FI_ZX_ADDR_USED);
            }
            IR2_INST *p = append_ir2_opnd3(mips_and, dest, src, n1_ir2_opnd);
            ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
            return p;
        }
#else
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_sx(&src)) {
            return NULL;
        } else {
            return append_ir2_opnd2i(mips_sll, dest, src, 0);
        }
#endif
    }

    IR2_INST *p = ir2_allocate();
    ir2_append(p);

    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    switch (opcode) {
    case mips_cfc1:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_creg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case mips_ctc1:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_creg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_mfc1:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case mips_dmfc1:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case mips_mtc1:
    case mips_dmtc1:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_mult:
    case mips_multu:
    case mips_div:
    case mips_divu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&dest, 32) && ir2_opnd_is_sx(&src, 32));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        lsenv->tr_data->hi_em = SIGN_EXTENSION;
        lsenv->tr_data->lo_em = SIGN_EXTENSION;
        break;
    case mips_dmult:
    case mips_dmultu:
    case mips_ddiv:
    case mips_ddivu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        lsenv->tr_data->hi_em = UNKNOWN_EXTENSION;
        lsenv->tr_data->lo_em = UNKNOWN_EXTENSION;
        break;
    case mips_teq:
    case mips_tne:
    case mips_tge:
    case mips_tlt:

    case mips_x86adcb:
    case mips_x86adch:
    case mips_x86adcw:
    case mips_x86adcd:

    case mips_x86addb:
    case mips_x86addh:
    case mips_x86addw:
    case mips_x86addd:

    case mips_x86sbcb:
    case mips_x86sbch:
    case mips_x86sbcw:
    case mips_x86sbcd:

    case mips_x86subb:
    case mips_x86subh:
    case mips_x86subw:
    case mips_x86subd:

    case mips_x86xorb:
    case mips_x86xorh:
    case mips_x86xorw:
    case mips_x86xord:

    case mips_x86andb:
    case mips_x86andh:
    case mips_x86andw:
    case mips_x86andd:

    case mips_x86orb:
    case mips_x86orh:
    case mips_x86orw:
    case mips_x86ord:

    case mips_x86sllvb:
    case mips_x86sllvh:
    case mips_x86sllvw:
    case mips_x86sllvd:

    case mips_x86srlvb:
    case mips_x86srlvh:
    case mips_x86srlvw:
    case mips_x86srlvd:

    case mips_x86sravb:
    case mips_x86sravh:
    case mips_x86sravw:
    case mips_x86sravd:

    case mips_x86rclvb:
    case mips_x86rclvh:
    case mips_x86rclvw:
    case mips_x86rclvd:

    case mips_x86rcrvb:
    case mips_x86rcrvh:
    case mips_x86rcrvw:
    case mips_x86rcrvd:

    case mips_x86rotlvb:
    case mips_x86rotlvh:
    case mips_x86rotlvw:
    case mips_x86rotlvd:

    case mips_x86rotrvb:
    case mips_x86rotrvh:
    case mips_x86rotrvw:
    case mips_x86rotrvd:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_sqrt_d:
    case mips_sqrt_s:
    case mips_rsqrt_s:
    case mips_rsqrt_d:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_c_un_d:
    case mips_c_un_s:
    case mips_c_lt_d:
    case mips_c_lt_s:
    case mips_cvt_l_d:
    case mips_cvt_w_d:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_biadd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_pmovmskb:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_floor_l_d:
    case mips_floor_l_s:
    case mips_ceil_l_d:
    case mips_trunc_l_d:
    case mips_round_l_d:
    case mips_round_l_s:
    case mips_floor_w_d:
    case mips_ceil_w_d:
    case mips_trunc_w_d:
    case mips_round_w_d:
    case mips_floor_w_s:
    case mips_ceil_w_s:
    case mips_trunc_w_s:
    case mips_round_w_s:
    case mips_movev:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vsignfillb:
    case mips_vsignfillh:
    case mips_vsignfillw:
    case mips_vsignfilld:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vgetsignb:
    case mips_vgetsignh:
    case mips_vgetsignw:
    case mips_vgetsignd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_frcpw:
    case mips_frcpd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_fsqrtw:
    case mips_fsqrtd:
    case mips_frsqrtw:
    case mips_frsqrtd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vacc8b_ud:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_insveb:
    case mips_insveh:
    case mips_insvew:
    case mips_insved:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    default:
        if (opcode == mips_mov_d || ir2_opcode_is_convert(opcode) ||
            ir2_opcode_is_fcmp(opcode)) {
            lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
            ir2_build(p, opcode, dest, src, ir2_opnd_none);
        } else if (ir2_opcode_is_branch(opcode) &&
                   !ir2_opcode_is_branch_with_3opnds(opcode) &&
                   opcode != mips_b) {
            lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_label(&src));
            ir2_build(p, opcode, dest, src, ir2_opnd_none);
            p = append_ir2_opnd0(mips_nop);
        } else {
            lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                      ir2_name(opcode));
        }
        break;
    }

    return p;
}

IR2_INST *append_ir2_opnd1i(IR2_OPCODE opcode, IR2_OPND dest, int32 imm)
{

    IR2_INST *p = ir2_allocate();
    IR2_OPND ir2_opnd_imm;

    ir2_append(p);

    ir2_opnd_build(&ir2_opnd_imm, IR2_OPND_IMM, imm);

    if (opcode == mips_lui) {
        // lsassert(ir2_opnd_is_ireg(&dest) && imm >= -32768 && imm <= 32767);
        lsassert(ir2_opnd_is_ireg(&dest) && imm >= -32768 && imm <= 65535);
        ir2_build(p, opcode, dest, ir2_opnd_imm, ir2_opnd_new_none());
        if (imm == 0) {
            ir2_opnd_set_em(&dest, ZERO_EXTENSION, 0);
        } else {
            ir2_opnd_set_em(&dest, int16_em(imm), int16_eb(imm) + 16);
        }
    } else if (opcode == mips_teqi || opcode == mips_tgei ||
               opcode == mips_tgeiu || opcode == mips_tlti ||
               opcode == mips_tltiu || opcode == mips_tnei) {
        ir2_build(p, opcode, dest, ir2_opnd_imm, ir2_opnd_new_none());
    } else if (opcode == mips_mfflag || opcode == mips_mtflag) {
        ir2_build(p, opcode, dest, ir2_opnd_imm, ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 0);
    } else if ((opcode >= mips_x86rotlb && opcode <= mips_x86rotl32d) || 
               (opcode >= mips_x86rotrb && opcode <= mips_x86rotr32d)) {
        ir2_build(p, opcode, dest, ir2_opnd_imm, ir2_opnd_new_none());
    } else if (opcode == mips_ldib || opcode == mips_ldid){
        lsassert(imm >= -512 && imm <=511);
        lsassert(ir2_opnd_is_freg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_imm, ir2_opnd_new_none());
    } else if (opcode == mips_pref){
        lsassert(imm >= 0 && imm <=31);
        ir2_build(p, opcode, ir2_opnd_imm, dest, ir2_opnd_new_none());
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnd1(IR2_OPCODE opcode, IR2_OPND dest)
{

    IR2_INST *p = ir2_allocate();
    ir2_append(p);

    if (opcode == mips_jr || opcode == mips_jalr) {
        lsassert(ir2_opnd_is_ireg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
        p = append_ir2_opnd0(mips_nop);
    } else if (opcode == mips_label) {
        lsassert(ir2_opnd_is_label(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
    } else if (opcode == mips_j || opcode == mips_b ||
               ir2_opcode_is_f_branch(opcode)) {
        lsassert(ir2_opnd_is_label(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
        p = append_ir2_opnd0(mips_nop);
    } else if (opcode == mips_mfhi) {
        lsassert(ir2_opnd_is_ireg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, lsenv->tr_data->hi_em, 32);
    } else if (opcode == mips_mflo) {
        lsassert(ir2_opnd_is_ireg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, lsenv->tr_data->lo_em, 32);
    } else if ((opcode >= mips_x86incb && opcode <= mips_x86incd) || 
             (opcode >= mips_x86decb && opcode <= mips_x86decd)) {
        lsassert(ir2_opnd_is_ireg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
    } else if (opcode == mips_mftop) {
        lsassert(ir2_opnd_is_ireg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_new_none(), ir2_opnd_new_none());
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 3);
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnda(IR2_OPCODE opcode, ADDR addr)
{

    IR2_INST *p = ir2_allocate();
    IR2_OPND ir2_opnd_addr;
    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    ir2_opnd_build(&ir2_opnd_addr, IR2_OPND_ADDR, addr);
    ir2_append(p);

    if (opcode == mips_j || opcode == mips_jal) {
        ir2_build(p, opcode, ir2_opnd_addr, ir2_opnd_none, ir2_opnd_none);

        p = append_ir2_opnd0(mips_nop);
    } else if (opcode == mips_x86_inst) {
        ir2_build(p, opcode, ir2_opnd_addr, ir2_opnd_none, ir2_opnd_none);
    } else if (opcode == mips_dup) {
        ir2_build(p, opcode, ir2_opnd_addr, ir2_opnd_none, ir2_opnd_none);
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opndi(IR2_OPCODE opcode, int32 imm)
{

    IR2_INST *p = ir2_allocate();
    IR2_OPND ir2_opnd_imm;

    ir2_append(p);
    ir2_opnd_build(&ir2_opnd_imm, IR2_OPND_IMM, imm);

    if (opcode == mips_mttop) {
        lsassert(imm >= -32768 && imm <= 32767);
        ir2_build(p, opcode, ir2_opnd_imm, ir2_opnd_new_none(), ir2_opnd_new_none());
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnd0(IR2_OPCODE opcode)
{

    IR2_INST *p = ir2_allocate();
    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    ir2_append(p);

    if (opcode == mips_nop || opcode == mips_break || opcode == mips_setop ||
        opcode == mips_clrtop || opcode == mips_inctop || opcode == mips_dectop) {
        ir2_build(p, opcode, ir2_opnd_none, ir2_opnd_none, ir2_opnd_none);
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnd3_not_nop(IR2_OPCODE opcode, IR2_OPND src1,
                                   IR2_OPND src2, IR2_OPND target_label)
{

    IR2_INST *p = ir2_allocate();
    ir2_append(p); /* insert into tr_data's ir2 array */
    if (ir2_opcode_is_branch(opcode)) {
        lsassert(ir2_opnd_is_ireg(&src1) && ir2_opnd_is_ireg(&src2) &&
                 ir2_opnd_is_label(&target_label));
        ir2_build(p, opcode, src1, src2, target_label);
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnda_not_nop(IR2_OPCODE opcode, ADDR addr)
{

    IR2_INST *p = ir2_allocate();
    ir2_append(p); /* insert into tr_data's ir2 array */
    IR2_OPND ir2_opnd_addr;
    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();
    ir2_opnd_build(&ir2_opnd_addr, IR2_OPND_ADDR, addr);
    if (opcode == mips_j || opcode == mips_jal) {
        ir2_build(p, opcode, ir2_opnd_addr, ir2_opnd_none, ir2_opnd_none);
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnd1_not_nop(IR2_OPCODE opcode, IR2_OPND dest)
{

    IR2_INST *p = ir2_allocate();
    ir2_append(p);
    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();
    if (opcode == mips_jr || opcode == mips_jalr) {
        lsassert(ir2_opnd_is_ireg(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_none, ir2_opnd_none);
    } else if (opcode == mips_b || ir2_opcode_is_f_branch(opcode)) {
        lsassert(ir2_opnd_is_label(&dest));
        ir2_build(p, opcode, dest, ir2_opnd_none, ir2_opnd_none);
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

IR2_INST *append_ir2_opnd2ii(IR2_OPCODE opcode, IR2_OPND opnd0, IR2_OPND opnd1, int32 opnd2, int32 opnd3){

    IR2_INST *p = ir2_allocate();
    ir2_append(p);
    if (opcode == mips_vextrinsd) {
        lsassert(ir2_opnd_is_freg(&opnd0));
        lsassert(ir2_opnd_is_freg(&opnd1));
        p->_opcode = opcode;
        p->_opnd[0] = opnd0;
        p->_opnd[1] = opnd1;
        lsassert(0 <= opnd2 && opnd2 <=1);
        lsassert(0 <= opnd3 && opnd3 <=1);
        IR2_OPND ir2_opnd_imm1;
        IR2_OPND ir2_opnd_imm2;
        ir2_opnd_build(&ir2_opnd_imm1, IR2_OPND_IMM, opnd2);
        ir2_opnd_build(&ir2_opnd_imm2, IR2_OPND_IMM, opnd3);
        p->_opnd[2] = ir2_opnd_imm1;
        p->_opnd[3] = ir2_opnd_imm2;
    } else {
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
    }

    return p;
}

/********************************************
 *                                          *
 *     LA IR2 implementation.               *
 *                                          *
 ********************************************/
bool la_ir2_opcode_is_load(IR2_INS_TYPE opcode)
{
    if (opcode >= LISA_LD_B && opcode <=  LISA_LD_D) {
        return true;
    }
    if (opcode >=  LISA_LD_BU && opcode <=  LISA_LD_WU) {
        return true;
    }
    if (opcode == LISA_LL_W || opcode == LISA_LL_D) {
        return true;
    }
    if (opcode == LISA_LDPTR_W || opcode == LISA_LDPTR_D) {
        return true;
    }
    if (opcode == LISA_FLD_S || opcode == LISA_FLD_D) {
        return true;
    }
    if (opcode == LISA_VLD || opcode == LISA_XVLD){
        return true;
    }
    if (opcode >= LISA_LDL_W && opcode <= LISA_LDR_D){
        return true;
    }
    if (opcode >= LISA_VLDREPL_D && opcode <= LISA_VLDREPL_B){
        return true;
    }
    if (opcode >= LISA_XVLDREPL_D && opcode <= LISA_XVLDREPL_B){
        return true;
    }
    if (opcode == LISA_PRELD) {
        return true;
    }
    return false;
}

bool la_ir2_opcode_is_store(IR2_INS_TYPE opcode)
{
    if (opcode >= LISA_ST_B && opcode <= LISA_ST_D) {
        return true;
    }
    if (opcode >= LISA_STL_W && opcode <= LISA_STR_D) {
        return true;
    }
    if (opcode == LISA_SC_D || opcode == LISA_SC_W) {
        return true;
    }
    if (opcode == LISA_STPTR_W || opcode == LISA_STPTR_D) {
        return true;
    }
    if (opcode == LISA_FST_S || opcode == LISA_FST_D) {
        return true;
    }
    if (opcode == LISA_XVST || opcode == LISA_VST){
        return true;
    }
    return false;
}

IR2_OPND create_ir2_opnd(IR2_OPND_TYPE type, int val) {
    IR2_OPND res;
    switch (type) {
    case IR2_OPND_GPR:
        res._reg_num = val;
        break;
    case IR2_OPND_IMMH:
        res._imm16 = val;
        break;
    default:
        break;
    }
    res._type = type;
    res.val = val;
    return res;
}

IR2_INST *la_append_ir2_opnd0(IR2_INS_TYPE type) {

    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 1;
    pir2->_opnd[0] = ir2_opnd_none;
    ir2_append(pir2);
    return pir2;
}

IR2_INST *la_append_ir2_opnd1(IR2_INS_TYPE type, IR2_OPND op0) {
    /*
     * FIXME: Might be handle mfhi/mflo later.
     */
    //if (type == LISA_X86MFTOP) {
    //    lsassertm(0, "cannot handle %s. Need to set EM",
    //                ir2_name(type));
    //}
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 1;
    pir2->_opnd[0] = op0;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd2(IR2_INS_TYPE type, IR2_OPND op0, IR2_OPND op1) {
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 2;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd3(IR2_INS_TYPE type, IR2_OPND op0, 
                      IR2_OPND op1, IR2_OPND op2) {
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 3;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    pir2->_opnd[2] = op2;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd4(IR2_INS_TYPE type, IR2_OPND op0, IR2_OPND op1, 
                      IR2_OPND op2, IR2_OPND op3) {
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 4;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    pir2->_opnd[2] = op2;
    pir2->_opnd[3] = op3;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opndi(IR2_INS_TYPE type, int imm) {
    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd1(type, opnd);
}

IR2_INST *la_append_ir2_opnd1i(IR2_INS_TYPE type, IR2_OPND op0, int imm) {
    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd2(type, op0, opnd);
}

IR2_INST *la_append_ir2_opnd2i(IR2_INS_TYPE type, IR2_OPND op0, IR2_OPND op1, int imm) {
    if(type == LISA_ANDI || type == LISA_ORI || type == LISA_XORI || type == LISA_LU52I_D)
        lsassert((unsigned int)(imm) <= 0xfff);
    else
        lsassert(imm >= -2048 && imm <= 2047);
    /*
     * This code is used for add AND xx, n1.
     */
#ifdef N64 /* validate address */
    if (la_ir2_opcode_is_load(type) || la_ir2_opcode_is_store(type)) {
        int base_ireg_num = ir2_opnd_base_reg_num(&op1);
        base_ireg_num = em_validate_address(base_ireg_num);
    }
#endif

    if (type == LISA_SC_W || type == LISA_LL_W) {
        lsassertm(((imm % 4) == 0), "ll/sc imm %d error.\n", imm);
        imm = imm >> 2;
    }

    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd3(type, op0, op1, opnd);
}

IR2_INST *la_append_ir2_opnd3i(IR2_INS_TYPE type, IR2_OPND op0, IR2_OPND op1,
        IR2_OPND op2, int imm0) {
    IR2_OPND imm0_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm0);
    return la_append_ir2_opnd4(type, op0, op1, op2, imm0_opnd);
}

IR2_INST *la_append_ir2_opnd2ii(IR2_INS_TYPE type, IR2_OPND op0, IR2_OPND op1,
        int imm0, int imm1) {
    IR2_OPND imm0_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm0);
    IR2_OPND imm1_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm1);
    return la_append_ir2_opnd4(type, op0, op1, imm0_opnd, imm1_opnd);
}

//ir2 with extension mode handling
IR2_INST *la_append_ir2_opnd3_em(IR2_INS_TYPE opcode, IR2_OPND op0, 
                      IR2_OPND op1, IR2_OPND op2) {
    if (ir2_opnd_is_imm(&op2)) {
        IR2_INS_TYPE new_opcode = ir2_opcode_rrr_to_rri(opcode);
        return la_append_ir2_opnd2i_em(new_opcode, op0, op1,
                                 ir2_opnd_imm(&op2));
    }

    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = opcode;
    pir2->op_count = 3;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    pir2->_opnd[2] = op2;

    lsassert(ir2_opnd_is_ireg(&op0) && ir2_opnd_is_ireg(&op1) &&
             ir2_opnd_is_ireg(&op2));
    switch (opcode) {
    case LISA_ADD_ADDR:
        if (ir2_opnd_is_mips_address(&op1)) {
            lsassert(ir2_opnd_is_sx(&op2, 32) &&
                     !ir2_opnd_is_mips_address(&op2));
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_mips_address(&op2)) {
            lsassert(ir2_opnd_is_sx(&op1, 32) &&
                     !ir2_opnd_is_mips_address(&op1));
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_ADD_W);
            ir2_opnd_set_em_add2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_ADD_ADDRX:
        if (ir2_opnd_is_x86_address(&op1)) {
            lsassert(!ir2_opnd_is_x86_address(&op2));
            ir2_set_opcode(pir2, LISA_ADD_D);
            if (ir2_opnd_is_sx(&op2, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em_add2(&op0, &op1, &op2);
            }
        } else if (ir2_opnd_is_x86_address(&op2)) {
            lsassert(!ir2_opnd_is_x86_address(&op1));
            ir2_set_opcode(pir2, LISA_ADD_D);
            if (ir2_opnd_is_sx(&op1, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em_add2(&op0, &op1, &op2);
            }
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_ADD_W);
            ir2_opnd_set_em_add2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADD_W);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SUB_ADDR:
        if (ir2_opnd_is_mips_address(&op1) &&
            ir2_opnd_is_mips_address(&op2)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        } else if (ir2_opnd_is_mips_address(&op1) &&
                   ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_W);
            ir2_opnd_set_em_sub2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADD_D);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SUB_ADDRX:
        if (ir2_opnd_is_x86_address(&op1) &&
            ir2_opnd_is_x86_address(&op2)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        } else if (ir2_opnd_is_x86_address(&op1) &&
                   ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32)) {
            ir2_set_opcode(pir2, LISA_SUB_W);
            ir2_opnd_set_em_sub2(&op0, &op1, &op2);
            if (!ir2_opnd_is_sx(&op0, 32)) {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_SUB_D);
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_ADD_D:
        ir2_opnd_set_em_add2(&op0, &op1, &op2);
        break;
    case LISA_SUB_D:
        ir2_opnd_set_em_sub2(&op0, &op1, &op2);
        break;
    case LISA_ADD_W:
        lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
        ir2_opnd_set_em_add2(&op0, &op1, &op2);
        if (!ir2_opnd_is_sx(&op0, 32)) {
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_SUB_W:
        //lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
        ir2_opnd_set_em_sub2(&op0, &op1, &op2);
        if (!ir2_opnd_is_sx(&op0, 32)) {
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_AND:
        if (ir2_opnd_is_x86_address(&op1) ||
            ir2_opnd_is_x86_address(&op2)) {
            ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_mips_address(&op1) ||
                   ir2_opnd_is_mips_address(&op2)) {
            ir2_opnd_set_em(&op0, EM_MIPS_ADDRESS, 32);
        } else {
            ir2_opnd_set_em_and2(&op0, &op1, &op2);
        }
        break;
    case LISA_OR:
        if (ir2_opnd_is_ax(&op1, 32) || ir2_opnd_is_ax(&op2, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        if (ir2_opnd_is_mips_address(&op1) ||
            ir2_opnd_is_mips_address(&op2)) {
            /*if (op1.is_mips_address() && op2 == zero_ir2_opnd)*/
            if (ir2_opnd_is_mips_address(&op1) &&
                ir2_opnd_cmp(&op2, &zero_ir2_opnd)) {
                ir2_opnd_set_em_mov(&op0, &op1);
            } else {
                lsassertm(0, "not implemented yet\n");
            }
        } else if (ir2_opnd_is_x86_address(&op1)) {
            lsassert(!ir2_opnd_is_address(&op2));
            if (ir2_opnd_is_zx(&op2, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else if (ir2_opnd_is_x86_address(&op2)) {
            lsassert(!ir2_opnd_is_address(&op1));
            if (ir2_opnd_is_zx(&op1, 32)) {
                ir2_opnd_set_em(&op0, EM_X86_ADDRESS, 32);
            } else {
                ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_opnd_set_em_or2(&op0, &op1, &op2);
        }
        break;
    case LISA_NOR:
        if (ir2_opnd_is_ax(&op1, 32) || ir2_opnd_is_ax(&op2, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&op1) || ir2_opnd_is_address(&op2)) {
            lsassertm(0, "not implemented yet\n");
        } else {
            ir2_opnd_set_em_nor2(&op0, &op1, &op2);
        }
        break;
    case LISA_XOR:
        if (ir2_opnd_is_ax(&op1, 32) || ir2_opnd_is_ax(&op2, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_xor2(&op0, &op1, &op2);
        }
        break;
    case LISA_SLL_W:
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_SLL_D:
        ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_SRL_W:
        //lsassert(ir2_opnd_is_sx(&op1, 32));
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_SRL_D:
        if (ir2_opnd_is_ax(&op1, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&op1)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_zx(&op1, 63)) {
            ir2_opnd_set_em_mov(&op0, &op1);
        } else {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SRA_W:
        lsassert(ir2_opnd_is_sx(&op1, 32));
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_SRA_D:
        if (ir2_opnd_is_ax(&op1, 32)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_address(&op1)) {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        } else if (ir2_opnd_is_sx(&op1, 63)) {
            ir2_opnd_set_em_mov(&op0, &op1);
        } else {
            ir2_opnd_set_em(&op0, UNKNOWN_EXTENSION, 32);
        }
        break;
    case LISA_SLTU:
    case LISA_SLT:
        ir2_opnd_set_em(&op0, ZERO_EXTENSION, 1);
        break;
    //case mips_dmult_g:
    //    ir2_opnd_set_em_dmult_g(&op0, &op1, &op2);
    //    break;
    case LISA_MUL_D:
        ir2_opnd_set_em_dmultu_g(&op0, &op1, &op2);
        break;
    //case mips_mult_g:
    //    ir2_opnd_set_em_dmult_g(&op0, &op1, &op2);
    //    if (!ir2_opnd_is_sx(&op0, 32)) {
    //        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
    //    }
    //    break;
    case LISA_MUL_W:
        lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
        ir2_opnd_set_em_dmultu_g(&op0, &op1, &op2);
        if (!ir2_opnd_is_sx(&op0, 32)) {
            ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        }
        break;
    //case mips_movz: //TODO
    //case mips_movn:
    //    // ir2_opnd_set_em_movcc(&op0, &op1, &op2);
    //    // arg error
    //    ir2_opnd_set_em_movcc(&op0, &op1, &op0);
    //    break;
    default:
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                  ir2_name(opcode));
        break;
    }
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd2i_em(IR2_INS_TYPE opcode, IR2_OPND dest, IR2_OPND src, int imm)
{
    if(opcode == LISA_ANDI || opcode == LISA_ORI || 
			opcode == LISA_XORI || opcode == LISA_LU52I_D)
        lsassert((unsigned int)(imm) <= 0xfff);
    else
        lsassert(imm >= -2048 && imm <= 2047);

    /*
     * This code is used for add AND xx, n1.
     */
#ifdef N64 /* validate address */
    if (la_ir2_opcode_is_load(opcode) || la_ir2_opcode_is_store(opcode)) {
        int base_ireg_num = ir2_opnd_base_reg_num(&src);
        base_ireg_num = em_validate_address(base_ireg_num);
    }
#endif

    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = opcode;
    pir2->op_count = 3;
    pir2->_opnd[0] = dest;
    pir2->_opnd[1] = src;

    switch (opcode) {
    case LISA_LOAD_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_LD_D);
        ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        break;
    case LISA_LOAD_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_LD_WU);
        ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        break;
    case LISA_STORE_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_ST_D);
        break;
    case LISA_STORE_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_set_opcode(pir2, LISA_ST_W);
        break;
    case LISA_SUBI_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        opcode = LISA_ADDI_ADDR;
        imm = -imm;
    case LISA_ADDI_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (ir2_opnd_is_mips_address(&src)) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em(&dest, EM_MIPS_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&src, 32)) {
            ir2_set_opcode(pir2, LISA_ADDI_W);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
            if (!ir2_opnd_is_sx(&dest, 32)) {
                ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        }
        break;
    case LISA_SUBI_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        opcode = LISA_ADDI_ADDRX;
        imm = -imm;
    case LISA_ADDI_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (ir2_opnd_is_x86_address(&src)) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
        } else if (ir2_opnd_is_sx(&src, 32)) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
            if (!ir2_opnd_is_sx(&dest, 32)) {
                ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
            }
        } else {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        }
        break;
    case LISA_LL_D:
    case LISA_LD_D:
    case LISA_LDR_D:
    case LISA_LDL_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_LL_W:
        lsassertm(((imm % 4) == 0), "ll_w imm %d error.\n", imm);
        imm = imm >> 2;
    case LISA_LD_W:
    case LISA_LDR_W:
    case LISA_LDL_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_LD_H:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 16);
        break;
    case LISA_LD_B:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 8);
        break;
    case LISA_LD_WU:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 32);
        break;
    case LISA_LD_HU:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 16);
        break;
    case LISA_LD_BU:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 8);
        break;
    case LISA_SUBIU:
        ir2_set_opcode(pir2, LISA_ADDI_W);
        imm = -imm;
    case LISA_ADDI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert(ir2_opnd_is_sx(&src, 32));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_DSUBIU:
        ir2_set_opcode(pir2, LISA_ADDI_D);
        imm = -imm;
    case LISA_ADDI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        break;
    //case LISA_daddi:
    //    ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
    //    break;
    case LISA_SLTI:
    case LISA_SLTUI: //TODO imm sign extend, but compared as unsigned
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, ZERO_EXTENSION, 1);
        break;
    case LISA_ANDI:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em_and(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case LISA_ORI:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em_or(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case LISA_XORI:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em_xor(&dest, &src, ZERO_EXTENSION, uint16_eb(imm));
        break;
    case LISA_SLLI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 31);
        ir2_opnd_set_em_dsll(&dest, &src, imm);
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_SRAI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        //lsassert(ir2_opnd_is_sx(&src, 32));
        lsassert((uint32)imm <= 31);
        if (!ir2_opnd_is_sx(&src, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_mov(&dest, &src);
        }
        ir2_opnd_set_em_dsra(&dest, &dest, imm); /* may be a problem */
        lsassert(ir2_opnd_is_sx(&dest, 32));
        break;
    case LISA_SRLI_W:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        //lsassert(ir2_opnd_is_zx(&src, 32));
        lsassert((uint32)imm <= 31);
        if (!ir2_opnd_is_zx(&src, 32)) {
            ir2_opnd_set_em(&dest, ZERO_EXTENSION, 32);
        } else {
            ir2_opnd_set_em_mov(&dest, &src);
        }
        ir2_opnd_set_em_dsrl(&dest, &dest, imm);
        lsassert(ir2_opnd_is_sx(&dest, 32));
        break;
    case LISA_SLLI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 63);
        ir2_opnd_set_em_dsll(&dest, &src, imm);
        break;
    case LISA_SRAI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 63);
        ir2_opnd_set_em_dsra(&dest, &src, imm);
        break;
    case LISA_SRLI_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        lsassert((uint32)imm <= 63);
        ir2_opnd_set_em_dsrl(&dest, &src, imm);
        break;
    case LISA_VSHUF4I_B:
    case LISA_VSHUF4I_H:
    case LISA_VSHUF4I_W:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        lsassert((imm & 0xffffff00) == 0);
        //ir2_opnd_build(&ir2_opnd_tmp, IR2_OPND_IMM, imm);
        //ir2_build(p, opcode, dest, src, ir2_opnd_tmp);
        break;
    case LISA_VLD:
        /*
         * VLD si12
         */
         lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_ireg(&src));
        break;
    default:
        printf("[LATX] [%s] not implemented in %s : %d\n", ir2_name(opcode),
				__func__, __LINE__);
        exit(-1);
        break;
    }

    IR2_OPND op2 = create_ir2_opnd(IR2_OPND_IMMH, imm);
    pir2->_opnd[2] = op2;
    ir2_append(pir2);
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnd2_em(IR2_INS_TYPE opcode, IR2_OPND dest, IR2_OPND src) {
    if (opcode == LISA_MOV64) {
        if (ir2_opnd_cmp(&dest, &src)) {
            return NULL;
        } else {
            IR2_INST *p = la_append_ir2_opnd3_em(LISA_OR, dest, src, zero_ir2_opnd);
            ir2_opnd_set_em_mov(&dest, &src);
            return p;
        }
    } else if (opcode == LISA_MOV32_SX) {
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_sx(&src, 32)) {
            return NULL;
        } else {
            return la_append_ir2_opnd2i_em(LISA_SLLI_W, dest, src, 0);
        }
    } else if (opcode == LISA_MOV32_ZX) {
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_zx(&src, 32)) {
            return NULL;
        } else {
            return la_append_ir2_opnd3_em(LISA_AND, dest, src, n1_ir2_opnd);
        }
    } else if (opcode == LISA_MOV_ADDRX) {
#ifdef N64
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_zx(&src, 32)) {
            if (!(ir2_opnd_cmp(&dest, &zero_ir2_opnd))) {
                ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
            }
            return NULL;
        } else {
            IR1_INST *pir1 = lsenv->tr_data->curr_ir1_inst;
            if (pir1 != NULL) {
                BITS_SET(pir1->flags, FI_ZX_ADDR_USED);
            }
            IR2_INST *p = la_append_ir2_opnd3_em(LISA_AND, dest, src, n1_ir2_opnd);
            ir2_opnd_set_em(&dest, EM_X86_ADDRESS, 32);
            return p;
        }
#else
        if (ir2_opnd_cmp(&dest, &src) && ir2_opnd_is_sx(&src)) {
            return NULL;
        } else {
            return la_append_ir2_opnd2i_em(LISA_SLLI_W, dest, src, 0);
        }
#endif
    }

    IR2_INST *p = ir2_allocate();

    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    switch (opcode) {
    case LISA_MOVFCSR2GR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_creg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_MOVGR2FCSR:
        lsassert(ir2_opnd_is_ireg(&src) && ir2_opnd_is_creg(&dest));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case LISA_MOVFR2GR_S:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
    case LISA_MOVFR2GR_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_MOVGR2FR_W:
    case LISA_MOVGR2FR_D:
        lsassert(ir2_opnd_is_ireg(&src) && ir2_opnd_is_freg(&dest));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
#if 0
    case mips_mult:
    case mips_multu:
    case mips_div:
    case mips_divu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src) &&
                 ir2_opnd_is_sx(&dest, 32) && ir2_opnd_is_sx(&src, 32));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        lsenv->tr_data->hi_em = SIGN_EXTENSION;
        lsenv->tr_data->lo_em = SIGN_EXTENSION;
        break;
    case mips_dmult:
    case mips_dmultu:
    case mips_ddiv:
    case mips_ddivu:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        lsenv->tr_data->hi_em = UNKNOWN_EXTENSION;
        lsenv->tr_data->lo_em = UNKNOWN_EXTENSION;
        break;
    case mips_teq:
    case mips_tne:
    case mips_tge:
    case mips_tlt:
#endif
    case LISA_X86ADC_B:
    case LISA_X86ADC_H:
    case LISA_X86ADC_W:
    case LISA_X86ADC_D:

    case LISA_X86ADD_B:
    case LISA_X86ADD_H:
    case LISA_X86ADD_W:
    case LISA_X86ADD_D:

    case LISA_X86SBC_B:
    case LISA_X86SBC_H:
    case LISA_X86SBC_W:
    case LISA_X86SBC_D:

    case LISA_X86SUB_B:
    case LISA_X86SUB_H:
    case LISA_X86SUB_W:
    case LISA_X86SUB_D:

    case LISA_X86XOR_B:
    case LISA_X86XOR_H:
    case LISA_X86XOR_W:
    case LISA_X86XOR_D:

    case LISA_X86AND_B:
    case LISA_X86AND_H:
    case LISA_X86AND_W:
    case LISA_X86AND_D:

    case LISA_X86OR_B:
    case LISA_X86OR_H:
    case LISA_X86OR_W:
    case LISA_X86OR_D:

    case LISA_X86SLL_B:
    case LISA_X86SLL_H:
    case LISA_X86SLL_W:
    case LISA_X86SLL_D:

    case LISA_X86SRL_B:
    case LISA_X86SRL_H:
    case LISA_X86SRL_W:
    case LISA_X86SRL_D:

    case LISA_X86SRA_B:
    case LISA_X86SRA_H:
    case LISA_X86SRA_W:
    case LISA_X86SRA_D:

    case LISA_X86ROTL_B:
    case LISA_X86ROTL_H:
    case LISA_X86ROTL_W:
    case LISA_X86ROTL_D:

    case LISA_X86RCR_B:
    case LISA_X86RCR_H:
    case LISA_X86RCR_W:
    case LISA_X86RCR_D:

    case LISA_X86RCL_B:
    case LISA_X86RCL_H:
    case LISA_X86RCL_W:
    case LISA_X86RCL_D:

    case LISA_X86ROTR_B:
    case LISA_X86ROTR_H:
    case LISA_X86ROTR_W:
    case LISA_X86ROTR_D:

    case LISA_X86MUL_B:
    case LISA_X86MUL_H:
    case LISA_X86MUL_W:
    case LISA_X86MUL_D:

    case LISA_X86MUL_BU:
    case LISA_X86MUL_HU:
    case LISA_X86MUL_WU:
    case LISA_X86MUL_DU:
        /*
         * FIXME: for LISA_X86MUL_*, we need add new assert here.
         */
        //lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
#if 0
    case mips_sqrt_d:
    case mips_sqrt_s:
    case mips_rsqrt_s:
    case mips_rsqrt_d:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_c_un_d:
    case mips_c_un_s:
    case mips_c_lt_d:
    case mips_c_lt_s:
    case mips_cvt_l_d:
    case mips_cvt_w_d:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_biadd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_pmovmskb:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_floor_l_d:
    case mips_floor_l_s:
    case mips_ceil_l_d:
    case mips_trunc_l_d:
    case mips_round_l_d:
    case mips_round_l_s:
    case mips_floor_w_d:
    case mips_ceil_w_d:
    case mips_trunc_w_d:
    case mips_round_w_d:
    case mips_floor_w_s:
    case mips_ceil_w_s:
    case mips_trunc_w_s:
    case mips_round_w_s:
    case mips_movev:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vsignfillb:
    case mips_vsignfillh:
    case mips_vsignfillw:
    case mips_vsignfilld:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vgetsignb:
    case mips_vgetsignh:
    case mips_vgetsignw:
    case mips_vgetsignd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_frcpw:
    case mips_frcpd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_fsqrtw:
    case mips_fsqrtd:
    case mips_frsqrtw:
    case mips_frsqrtd:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_vacc8b_ud:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
    case mips_insveb:
    case mips_insveh:
    case mips_insvew:
    case mips_insved:
        lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
        ir2_build(p, opcode, dest, src, ir2_opnd_none);
        break;
#endif
    default:
        if (opcode == LISA_FMOV_D || ir2_opcode_is_convert(opcode) ||
            ir2_opcode_is_fcmp(opcode)) {
            lsassert(ir2_opnd_is_freg(&dest) && ir2_opnd_is_freg(&src));
            ir2_build(p, opcode, dest, src, ir2_opnd_none);
        } else if (ir2_opcode_is_branch(opcode) &&
                   !ir2_opcode_is_branch_with_3opnds(opcode) &&
                   opcode != LISA_B) {
            lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_label(&src));
            ir2_build(p, opcode, dest, src, ir2_opnd_none);
            //p = append_ir2_opnd0(mips_nop);
        } else {
            lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                      ir2_name(opcode));
        }
        break;
    }

    ir2_append(p);
    if(!ir2_op_check(p))
        lsassertm(0, "Maybe you should check the type of operand");
    return p;
}

IR2_INST *la_append_ir2_opnd1i_em(IR2_INS_TYPE opcode, IR2_OPND op0, int imm) {
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND op1= create_ir2_opnd(IR2_OPND_IMMH, imm);
    pir2->_opcode = opcode;
    pir2->op_count = 2;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    ir2_append(pir2);
    if (opcode == LISA_X86MFFLAG || opcode == LISA_X86MTFLAG) {
        ir2_opnd_set_em(&op0, ZERO_EXTENSION, 0);
    }else if(opcode == LISA_LU12I_W || opcode == LISA_LU32I_D || opcode == LISA_LU52I_D){
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 0);
    }
    if(!ir2_op_check(pir2))
        lsassertm(0, "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnda(IR2_INS_TYPE opcode, ADDR addr)
{
    IR2_INST *p = ir2_allocate();
    IR2_OPND ir2_opnd_addr;
    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();

    ir2_opnd_build(&ir2_opnd_addr, IR2_OPND_ADDR, addr);
    switch (opcode) {
        case LISA_B:
        case LISA_BL:
        case LISA_X86_INST:
        case LISA_DUP:
            ir2_build(p, opcode, ir2_opnd_addr, ir2_opnd_none, ir2_opnd_none);
            break;
    default:
            lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                      ir2_name(opcode));
            break;
    }

    ir2_append(p);
    ir2_op_check(p);
    return p;
}
