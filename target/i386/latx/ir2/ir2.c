#include "common.h"
#include "ir1.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "translate.h"
#include <string.h>

#include "ir2.h"

static const char *ir2_name(int value)
{
    /*
     * Todo: Add below defination:
     */
    #if 0
    static const char *ir2_scr_name[] = {
        "$scr0" , "$scr1" , "$scr2" , "$scr3",
    };

    static const char *ir2_cc_name[] = {
        "$cc0" , "$cc1" , "$cc2" , "$cc3" ,
        "$cc4" , "$cc5" , "$cc6" , "$cc7" ,
    };
    #endif
    const char *g_ir2_names[] = {
        "$zero" , "$ra" , "$tp" , "$sp" , "$a0" , "$a1" , "$a2" , "$a3" ,
        "$a4"   , "$a5" , "$a6" , "$a7" , "$t0" , "$t1" , "$t2" , "$t3" ,
        "$t4"   , "$t5" , "$t6" , "$t7" , "$t8" , "$x"  , "$fp" , "$s0" ,
        "$s1"   , "$s2" , "$s3" , "$s4" , "$s5" , "$s6" , "$s7" , "$s8" ,
        "", "", "", "", "", "", "", "", /*  32-39 */
        "$fa0"  , "$fa1"  , "$fa2"  , "$fa3"  ,
        "$fa4"  , "$fa5"  , "$fa6"  , "$fa7"  ,
        "$ft0"  , "$ft1"  , "$ft2"  , "$ft3"  ,
        "$ft4"  , "$ft5"  , "$ft6"  , "$ft7"  ,
        "$ft8"  , "$ft9"  , "$ft10" , "$ft11" ,
        "$ft12" , "$ft13" , "$ft14" , "$ft15" ,
        "$fs0"  , "$fs1"  , "$fs2"  , "$fs3"  ,
        "$fs4"  , "$fs5"  , "$fs6"  , "$s7"   ,
        "", "", "", "", "", "", "", "", /* 72-79 */
        "NONE", "GPR", "SCR", "FPR", "FCSR", "IMMD", "IMMH", "LABEL",/* 80-87 */
        "", "", "", "", "", "", "", "", /* 88-95 */
        "ax", "sx", "zx", "bx", "AD", "AX", "", "",  /* 96-103 */
        "", "", "", "", "", "", "", "", /* 104-111 */
        "", "", "", "", "", "", "", "", /* 112-119 */
        "", "", "", "", "", "", "", "", /* 120-127 */

#include "ir2-name.h"

    };
    lsassert(value <= LISA_XVFCMP_COND_Q);
    return g_ir2_names[value];
}

IR2_OPND sp_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 3,  ._reg_num = 3,  ._addr = 0};
IR2_OPND fp_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 22, ._reg_num = 22, ._addr = 0};
IR2_OPND zero_ir2_opnd = {._type = IR2_OPND_GPR, .val = 0,  ._reg_num = 0,  ._addr = 0};
IR2_OPND env_ir2_opnd = {._type = IR2_OPND_GPR,  .val = 25, ._reg_num = 25, ._addr = 0};
IR2_OPND n1_ir2_opnd = {._type = IR2_OPND_IREG, .val = 23, ._reg_num = 23, ._addr = 0};
IR2_OPND a0_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 4,  ._reg_num = 4,  ._addr = 0};
IR2_OPND t5_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 17,  ._reg_num = 17,  ._addr = 0};
IR2_OPND ra_ir2_opnd = {._type = IR2_OPND_GPR,   .val = 1,  ._reg_num = 1,  ._addr = 0};

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
    return ir2_opnd_is_ireg(opnd) &&
           (ir2_opnd_base_reg_num(opnd) >= ITEMP0_NUM) &&
           (ir2_opnd_base_reg_num(opnd) <= ITEMP9_NUM);
}

int ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_freg(opnd) &&
           (ir2_opnd_base_reg_num(opnd) >= FTEMP0_NUM) &&
           (ir2_opnd_base_reg_num(opnd) <= FTEMP6_NUM);
}

int ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return ir2_opnd_is_mem(opnd) &&
           (ir2_opnd_base_reg_num(opnd) > 3) &&
           (ir2_opnd_base_reg_num(opnd) < 14);
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

bool ir2_opcode_is_branch(IR2_OPCODE opcode)
{
    return (opcode >= LISA_BEQZ && opcode <= LISA_BCNEZ) ||
           (opcode >= LISA_B && opcode <= LISA_BGEU);
}

bool ir2_opcode_is_branch_with_3opnds(IR2_OPCODE opcode)
{
    if (opcode >= LISA_BEQ && opcode <= LISA_BGEU) {
        return true;
    }
    return false;
}

bool ir2_opcode_is_f_branch(IR2_OPCODE opcode)
{
    return (opcode == LISA_BCEQZ || opcode == LISA_BCNEZ);
}

bool ir2_opcode_is_convert(IR2_OPCODE opcode)
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

bool ir2_opcode_is_fcmp(IR2_OPCODE opcode)
{
    return (opcode == LISA_FCMP_COND_S || opcode == LISA_FCMP_COND_D);
}

static IR2_OPCODE ir2_opcode_rrr_to_rri(IR2_OPCODE opcode)
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

IR2_INST *ir2_get(int id)
{
    lsassert(id >= 0 && id < lsenv->tr_data->ir2_cur_nr);
    return lsenv->tr_data->ir2_array + id;
}

uint32 ir2_opnd_addr(IR2_OPND *ir2) { return ir2->_addr; }

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

static int ir1_id;

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
    if (ir2->_id == 0) {
        ir1_id = 0;
        fprintf(stderr, "[%d, %d] %s\n", ir2->_id, ir1_id, str);
    } else if (str[0] == '-') {
        ir1_id ++;
        fprintf(stderr, "[%d, %d] %s\n", ir2->_id, ir1_id, str);
    } else {
        fprintf(stderr, "%s\n", str);
    }


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
    if(ir2->_opcode >= LISA_GR2SCR) {
        lsenv->tr_data->ir2_asm_nr++;
    }
}

static IR2_INST *ir2_allocate(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;

    /* 1. make sure we have enough space */
    if (t->ir2_cur_nr == t->ir2_max_nr) {
        int bytes = sizeof(IR2_INST) * t->ir2_max_nr;
        /* 1.2. double the array */
        t->ir2_max_nr *= 2;
        t->ir2_array = (IR2_INST *)mm_realloc(t->ir2_array, bytes << 1);
    }

    /* 2. allocate one */
    IR2_INST *p = t->ir2_array + t->ir2_cur_nr;
    ir2_set_id(p, t->ir2_cur_nr);
    t->ir2_cur_nr++;

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

/********************************************
 *                                          *
 *     LA IR2 implementation.               *
 *                                          *
 ********************************************/
bool la_ir2_opcode_is_load(IR2_OPCODE opcode)
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

bool la_ir2_opcode_is_store(IR2_OPCODE opcode)
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

IR2_INST *la_append_ir2_opnd0(IR2_OPCODE type) {

    IR2_OPND ir2_opnd_none = ir2_opnd_new_none();
    IR2_INST *pir2 = ir2_allocate();
    pir2->_opcode = type;
    pir2->op_count = 1;
    pir2->_opnd[0] = ir2_opnd_none;
    ir2_append(pir2);
    return pir2;
}

IR2_INST *la_append_ir2_opnd1(IR2_OPCODE type, IR2_OPND op0) {
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

IR2_INST *la_append_ir2_opnd2(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1) {
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

IR2_INST *la_append_ir2_opnd3(IR2_OPCODE type, IR2_OPND op0,
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

IR2_INST *la_append_ir2_opnd4(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1,
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

IR2_INST *la_append_ir2_opndi(IR2_OPCODE type, int imm) {
    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd1(type, opnd);
}

IR2_INST *la_append_ir2_opnd1i(IR2_OPCODE type, IR2_OPND op0, int imm) {
    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd2(type, op0, opnd);
}

IR2_INST *la_append_ir2_opnd2i(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, int imm) {
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
    } else if (type == LISA_SC_D || type == LISA_LL_D) {
        lsassertm(((imm % 4) == 0), "ll/sc imm %d error.\n", imm);
        imm = imm >> 2;
    }


    IR2_OPND opnd = create_ir2_opnd(IR2_OPND_IMMH, imm);
    return la_append_ir2_opnd3(type, op0, op1, opnd);
}

IR2_INST *la_append_ir2_opnd3i(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1,
        IR2_OPND op2, int imm0) {
    IR2_OPND imm0_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm0);
    return la_append_ir2_opnd4(type, op0, op1, op2, imm0_opnd);
}

IR2_INST *la_append_ir2_opnd2ii(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1,
        int imm0, int imm1) {
    IR2_OPND imm0_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm0);
    IR2_OPND imm1_opnd = create_ir2_opnd(IR2_OPND_IMMH, imm1);
    return la_append_ir2_opnd4(type, op0, op1, imm0_opnd, imm1_opnd);
}

//ir2 with extension mode handling
IR2_INST *la_append_ir2_opnd3_em(IR2_OPCODE opcode, IR2_OPND op0,
                      IR2_OPND op1, IR2_OPND op2) {
    if (ir2_opnd_is_imm(&op2)) {
        IR2_OPCODE new_opcode = ir2_opcode_rrr_to_rri(opcode);
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
        //lsassert(ir2_opnd_is_sx(&op1, 32) && ir2_opnd_is_sx(&op2, 32));
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
    case LISA_ANDN:
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

IR2_INST *la_append_ir2_opnd2i_em(IR2_OPCODE opcode, IR2_OPND dest, IR2_OPND src, int imm)
{
    if(opcode == LISA_ANDI || opcode == LISA_ORI || 
       opcode == LISA_XORI || opcode == LISA_LU52I_D) {
        lsassert((unsigned int)(imm) <= 0xfff);
    } else if (opcode == LISA_SUBIU || opcode == LISA_DSUBIU) {
        lsassert(imm > -2048 && imm <= 2048);
    } else {
        lsassert(imm >= -2048 && imm < 2048);
    }

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
    case LISA_ADDI_ADDR:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (opcode == LISA_SUBI_ADDR) {
            imm = -imm;
        }
        opcode = LISA_ADDI_ADDR;
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
    case LISA_ADDI_ADDRX:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        if (opcode == LISA_SUBI_ADDRX) {
            imm = -imm;
        }
        opcode = LISA_ADDI_ADDRX;
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
        lsassertm(((imm % 4) == 0), "ll_d imm %d error.\n", imm);
        imm = imm >> 2;
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_LD_D:
    case LISA_LDR_D:
    case LISA_LDL_D:
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, UNKNOWN_EXTENSION, 32);
        break;
    case LISA_LL_W:
        lsassertm(((imm % 4) == 0), "ll_w imm %d error.\n", imm);
        imm = imm >> 2;
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        break;
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
    case LISA_ADDI_W:
        if (opcode == LISA_SUBIU) {
            ir2_set_opcode(pir2, LISA_ADDI_W);
            imm = -imm;
        }
        lsassert(ir2_opnd_is_ireg(&dest) && ir2_opnd_is_ireg(&src));
        //lsassert(ir2_opnd_is_sx(&src, 32));
        lsassertm(!ir2_opnd_is_address(&src),
                  "should use addi_addr or addi_addrx\n");
        ir2_opnd_set_em_add(&dest, &src, int16_em(imm), int16_eb(imm));
        if (!ir2_opnd_is_sx(&dest, 32)) {
            ir2_opnd_set_em(&dest, SIGN_EXTENSION, 32);
        }
        break;
    case LISA_DSUBIU:
    case LISA_ADDI_D:
        if (opcode == LISA_DSUBIU) {
            ir2_set_opcode(pir2, LISA_ADDI_D);
            imm = -imm;
        }
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

IR2_INST *la_append_ir2_opnd2_em(IR2_OPCODE opcode, IR2_OPND dest, IR2_OPND src) {
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

IR2_INST *la_append_ir2_opnd1i_em(IR2_OPCODE opcode, IR2_OPND op0, int imm) {
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND op1= create_ir2_opnd(IR2_OPND_IMMH, imm);
    pir2->_opcode = opcode;
    pir2->op_count = 2;
    pir2->_opnd[0] = op0;
    pir2->_opnd[1] = op1;
    ir2_append(pir2);

    switch (opcode) {
    case LISA_X86MFFLAG:
        ir2_opnd_set_em(&op0, ZERO_EXTENSION, 12);
        break;
    case LISA_LU12I_W:
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 32);
        break;
    case LISA_LU32I_D:
        ir2_opnd_set_em(&op0, SIGN_EXTENSION, 52);
        break;
    default:
        lsassertm(0, "cannot handle %s. add more cases if you are sure.",
                    ir2_name(opcode));
        break;
    }

    lsassertm(ir2_op_check(pir2), "Maybe you should check the type of operand");
    return pir2;
}

IR2_INST *la_append_ir2_opnda(IR2_OPCODE opcode, ADDR addr)
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

#ifdef CONFIG_SOFTMMU

#define LATXS_REG_ALLOC_DEFINE_IR2(ir2name, ir2type, ir2val) \
IR2_OPND ir2name = {._type = ir2type, .val = ir2val}

#define LATXS_REG_ALLOC_DEFINE_IR2_GPR(ir2name, ir2val) \
LATXS_REG_ALLOC_DEFINE_IR2(ir2name, IR2_OPND_GPR, ir2val);

/* Integer Register */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_zero_ir2_opnd,    0); /* ZERO */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_sp_ir2_opnd,      3); /* SP */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_fp_ir2_opnd,     22); /* FP */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_env_ir2_opnd,    23); /* S0 */

/* Floating Point  Register */
LATXS_REG_ALLOC_DEFINE_IR2(latxs_f32_ir2_opnd, IR2_OPND_FPR, 8);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr_ir2_opnd, IR2_OPND_FCSR, 0);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr1_ir2_opnd, IR2_OPND_FCSR, 1);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr2_ir2_opnd, IR2_OPND_FCSR, 2);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcsr3_ir2_opnd, IR2_OPND_FCSR, 3);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc0_ir2_opnd, IR2_OPND_CC, 0);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc1_ir2_opnd, IR2_OPND_CC, 1);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc2_ir2_opnd, IR2_OPND_CC, 2);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc3_ir2_opnd, IR2_OPND_CC, 3);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc4_ir2_opnd, IR2_OPND_CC, 4);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc5_ir2_opnd, IR2_OPND_CC, 5);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc6_ir2_opnd, IR2_OPND_CC, 6);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_fcc7_ir2_opnd, IR2_OPND_CC, 7);

/* Helper Arguments Register */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg0_ir2_opnd,  4); /* a0 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg1_ir2_opnd,  5); /* a1 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg2_ir2_opnd,  6); /* a2 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg3_ir2_opnd,  7); /* a3 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg4_ir2_opnd,  8); /* a4/stmp1      */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg5_ir2_opnd,  9); /* a5/stmp2      */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg6_ir2_opnd, 10); /* a6/TB address */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_arg7_ir2_opnd, 11); /* a7/eip        */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_ret0_ir2_opnd,  4); /* v0 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_ret1_ir2_opnd,  5); /* v1 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_ra_ir2_opnd,    1); /* ra */

/* Static Temp Registers */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_stmp1_ir2_opnd, 8); /* a4 */
LATXS_REG_ALLOC_DEFINE_IR2_GPR(latxs_stmp2_ir2_opnd, 9); /* a5 */

/* Consist value */
LATXS_REG_ALLOC_DEFINE_IR2(latxs_invalid_ir2_opnd, IR2_OPND_INV, 0);

LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr0_ir2_opnd, IR2_OPND_SCR, 0);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr1_ir2_opnd, IR2_OPND_SCR, 1);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr2_ir2_opnd, IR2_OPND_SCR, 2);
LATXS_REG_ALLOC_DEFINE_IR2(latxs_scr3_ir2_opnd, IR2_OPND_SCR, 3);

/* Functions to build IR2_OPND */
IR2_OPND latxs_ir2_opnd_new(IR2_OPND_TYPE type, int value)
{
    IR2_OPND opnd;
    latxs_ir2_opnd_build(&opnd, type, value);
    return opnd;
}

IR2_OPND latxs_ir2_opnd_new_inv(void)
{
    return latxs_invalid_ir2_opnd;
}

IR2_OPND latxs_ir2_opnd_new_label(void)
{
    IR2_OPND opnd;
    opnd._type = IR2_OPND_LABEL;
    opnd.val   = ++(lsenv->tr_data->label_num);
    return opnd;
}

void latxs_ir2_opnd_build(IR2_OPND *opnd, IR2_OPND_TYPE t, int value)
{
    opnd->_type = t;
    opnd->val   = value;
}

void latxs_ir2_opnd_build_mem(IR2_OPND *opnd, int base, int offset)
{
    opnd->_type = IR2_OPND_MEMY;
    opnd->val = base;
    opnd->imm = offset;
}

/* Functions to access IR2_OPND's fields */
IR2_OPND_TYPE latxs_ir2_opnd_type(IR2_OPND *opnd) { return opnd->_type; }
int latxs_ir2_opnd_reg(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_imm(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_label_id(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_addr(IR2_OPND *opnd) { return opnd->val; }
int latxs_ir2_opnd_offset(IR2_OPND *opnd)
{
    lsassert(latxs_ir2_opnd_is_mem(opnd));
    return opnd->imm;
}

/* Functions to modify IR2_OPND_MEMY */
IR2_OPND latxs_ir2_opnd_mem_get_base(IR2_OPND *opnd)
{
    lsassert(latxs_ir2_opnd_is_mem(opnd));
    IR2_OPND ir2_opnd =
        latxs_ir2_opnd_new(IR2_OPND_GPR, latxs_ir2_opnd_reg(opnd));
    return ir2_opnd;
}
void latxs_ir2_opnd_mem_set_base(IR2_OPND *mem, IR2_OPND *base)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    mem->val = latxs_ir2_opnd_reg(base);
}
int latxs_ir2_opnd_mem_get_offset(IR2_OPND *opnd)
{
    lsassertm(latxs_ir2_opnd_is_mem(opnd),
            "ir2 opnd offset could be used for IR2 OPND MEM only\n");
    return opnd->imm;
}
void latxs_ir2_opnd_mem_adjust_offset(IR2_OPND *mem, int offset)
{
    lsassert(latxs_ir2_opnd_is_mem(mem));
    mem->imm += offset;
}

/* Compare IR2 OPND */
int latxs_ir2_opnd_cmp(IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    return opnd1->_type == opnd2->_type &&
           opnd1->val   == opnd2->val;
}

/* Functions to identify IR2_OPND's type/attribute */
int latxs_ir2_opnd_is_inv(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_INV;
}
int latxs_ir2_opnd_is_gpr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_GPR;
}
int latxs_ir2_opnd_is_fpr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_FPR;
}
int latxs_ir2_opnd_is_scr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_SCR;
}
int latxs_ir2_opnd_is_fcsr(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_FCSR;
}
int latxs_ir2_opnd_is_cc(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_CC;
}
int latxs_ir2_opnd_is_label(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_LABEL;
}
int latxs_ir2_opnd_is_mem(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_MEMY;
}
int latxs_ir2_opnd_is_immd(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_IMMD;
}
int latxs_ir2_opnd_is_immh(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_IMMH;
}
int latxs_ir2_opnd_is_imm(IR2_OPND *opnd)
{
    return opnd->_type == IR2_OPND_IMMD || opnd->_type == IR2_OPND_IMMH;
}

int latxs_ir2_opnd_is_itemp(IR2_OPND *opnd)
{
    return latxs_ir2_opnd_is_gpr(opnd) && latxs_ir2_opnd_is_reg_temp(opnd);
}
int latxs_ir2_opnd_is_ftemp(IR2_OPND *opnd)
{
    return latxs_ir2_opnd_is_fpr(opnd) && latxs_ir2_opnd_is_reg_temp(opnd);
}
int latxs_ir2_opnd_is_mem_base_itemp(IR2_OPND *opnd)
{
    return latxs_ir2_opnd_is_mem(opnd) && latxs_ir2_opnd_is_reg_temp(opnd);
}
int latxs_ir2_opnd_is_reg_temp(IR2_OPND *opnd)
{
    int reg;
    switch (latxs_ir2_opnd_type(opnd)) {
    case IR2_OPND_MEMY:
    case IR2_OPND_GPR: {
        reg = latxs_ir2_opnd_reg(opnd);
#ifdef LATXS_ITMP_CONTINUS
        return LATXS_ITMP_MIN <= reg && reg <= LATXS_ITMP_MAX;
#else
        int i = 0;
        for (; i < latxs_itemp_status_num; i++) {
            if (reg == latxs_itemp_status_default[i].physical_id) {
                return 1;
            }
        }
        return 0;
#endif
    }
    case IR2_OPND_FPR: {
        reg = latxs_ir2_opnd_reg(opnd);
#ifdef LATXS_FTMP_CONTINUS
        return LATXS_FTMP_MIN <= reg && reg <= LATXS_FTMP_MAX;
#else
        int i = 0;
        for (; i < ftemp_status_num; i++) {
            if (reg == ftemp_status_default[i].physical_id) {
                return 1;
            }
        }
        return 0;
#endif
    }
    default: {
        lsassert(0);
        return -1;
    }
    }
}

/* Functions to convert IR2_INST to string */
int latxs_ir2_dump(IR2_INST *ir2)
{
    char str[64];
    int size = 0;

    if (ir2_opcode(ir2) == 0) {
        /*
         * an empty IR2_INST was inserted into the ir2 list,
         * but not assigned yet.
         */
        return 0;
    }

    size = latxs_ir2_to_string(ir2, str);
    fprintf(stderr, "[%03d] %s\n", ir2->_id, str);
    return size;
}

int latxs_ir2_to_string(IR2_INST *ir2, char *str)
{
    int length = 0;
    int i = 0;
    bool hex = false;

    length = sprintf(str, "%-8s  ", ir2_name(ir2_opcode(ir2)));

    if (ir2_opcode(ir2) == LISA_ANDI ||
        ir2_opcode(ir2) == LISA_ORI  ||
        ir2_opcode(ir2) == LISA_XORI ||
        ir2_opcode(ir2) == LISA_LU12I_W) {
        hex = true;
    }

    for (i = 0; i < 4; ++i) {
        IR2_OPND *opnd = &ir2->_opnd[i];
        if (latxs_ir2_opnd_type(opnd) == IR2_OPND_INV) {
            return length;
        } else {
            if (i > 0) {
                strcat(str, ",");
                length += 1;
            }
            length += latxs_ir2_opnd_to_string(opnd, str + length, hex);
        }
    }

    return length;
}

int latxs_ir2_opnd_to_string(IR2_OPND *opnd, char *str, bool hex)
{
    int reg_num = latxs_ir2_opnd_reg(opnd);

    switch (latxs_ir2_opnd_type(opnd)) {
    case IR2_OPND_INV: return 0;
    case IR2_OPND_GPR: {
        strcpy(str, latxs_ir2_name(reg_num));
        return strlen(str);
    }
    case IR2_OPND_FPR: {
        strcpy(str, latxs_ir2_name(40 + reg_num));
        return strlen(str);
    }
    case IR2_OPND_SCR: {
        return sprintf(str, "$scr%d", reg_num);
    }
    case IR2_OPND_FCSR: {
        return sprintf(str, "$c%d", reg_num);
    }
    case IR2_OPND_CC: {
        return sprintf(str, "$c%d", reg_num);
    }
    case IR2_OPND_IMMD: {
        return sprintf(str, "%d", latxs_ir2_opnd_imm(opnd));
    }
    case IR2_OPND_IMMH: {
        if (hex) {
            return sprintf(str, "0x%x", (uint16_t)latxs_ir2_opnd_imm(opnd));
        } else {
            return sprintf(str, "%d", latxs_ir2_opnd_imm(opnd));
        }
    }
    case IR2_OPND_LABEL:
        return sprintf(str, "LABEL %d", latxs_ir2_opnd_imm(opnd));
    case IR2_OPND_MEMY: {
        return sprintf(str, "0x%x(%s)",
                       latxs_ir2_opnd_imm(opnd),
                       latxs_ir2_name(reg_num));
    }
    default:
        lsassertm(0, "type = %d\n", latxs_ir2_opnd_type(opnd));
        return 0;
    }
}

const char *latxs_ir2_name(int value)
{
    return ir2_name(value);
}

/* Function used during label disposing */
void latxs_ir2_opnd_convert_label_to_imm(IR2_OPND *opnd, int imm)
{
    lsassert(latxs_ir2_opnd_is_label(opnd));
    opnd->_type = IR2_OPND_IMMH;
    opnd->val = imm;
}

/* Functions to manage IR2 extension mode */
static
int latxs_ir2_opnd_is_mapping(IR2_OPND *opnd2, int *x86_gpr_num)
{
    lsassert(latxs_ir2_opnd_is_gpr(opnd2));
    int reg = latxs_ir2_opnd_reg(opnd2);
    if (24 <= reg && reg <= 31) {
        *x86_gpr_num = reg - 24;
        return true;
    }
    return false;
}

void latxs_ir2_opnd_set_emb(IR2_OPND *opnd2, EXMode em, EXBits eb)
{
    opnd2->em = em; opnd2->eb = eb;
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        latxs_td_set_reg_extmb(gpr_num, em, eb);
    }
}

void latxs_ir2_opnd_set_em(IR2_OPND *opnd2, EXMode em)
{
    opnd2->em = em;
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        latxs_td_set_reg_extm(gpr_num, em);
    }
}

void latxs_ir2_opnd_set_eb(IR2_OPND *opnd2, EXBits eb)
{
    opnd2->eb = eb;
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        latxs_td_set_reg_extb(gpr_num, eb);
    }
}

EXMode latxs_ir2_opnd_get_em(IR2_OPND *opnd2)
{
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        return latxs_td_get_reg_extm(gpr_num);
    }
    return opnd2->em;
}

EXBits latxs_ir2_opnd_get_eb(IR2_OPND *opnd2)
{
    int gpr_num = 0;
    if (latxs_ir2_opnd_is_mapping(opnd2, &gpr_num)) {
        return latxs_td_get_reg_extb(gpr_num);
    }
    return opnd2->eb;
}

/* ------------------ IR2_INST ------------------ */

/* Fucntion to build IR2_INST */
void latxs_ir2_build(
        IR2_INST *ir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    ir2->_opcode  = opcode;
    ir2->_opnd[0] = opnd0 ? *opnd0 : latxs_invalid_ir2_opnd;
    ir2->_opnd[1] = opnd1 ? *opnd1 : latxs_invalid_ir2_opnd;
    ir2->_opnd[2] = opnd2 ? *opnd2 : latxs_invalid_ir2_opnd;
    ir2->_opnd[3] = opnd3 ? *opnd3 : latxs_invalid_ir2_opnd;
}
void latxs_ir2_build0(IR2_INST *pir2, IR2_OPCODE opcode)
{
    latxs_ir2_build(pir2, opcode, NULL, NULL, NULL, NULL);
}
void latxs_ir2_build1(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0)
{
    latxs_ir2_build(pir2, opcode, opnd0, NULL, NULL, NULL);
}
void latxs_ir2_build2(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1)
{
    latxs_ir2_build(pir2, opcode, opnd0, opnd1, NULL, NULL);
}
void latxs_ir2_build3(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2)
{
    latxs_ir2_build(pir2, opcode, opnd0, opnd1, opnd2, NULL);
}
void latxs_ir2_build4(
        IR2_INST *pir2, IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
                IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    latxs_ir2_build(pir2, opcode, opnd0, opnd1, opnd2, opnd3);
}

/* Fucntions ot access IR2_INST's fields */
IR2_OPND *latxs_ir2_branch_get_label(IR2_INST *pir2)
{
    int index = latxs_ir2_branch_label_index(pir2);

    if (index >= 0) {
        IR2_OPND *label = &pir2->_opnd[index];
        if (latxs_ir2_opnd_is_label(label)) {
            return label;
        }
    } else {
        lsassertm(0, "unsupported branch label.\n");
    }

    return NULL;
}
int latxs_ir2_branch_label_index(IR2_INST *pir2)
{
    IR2_OPCODE opc = ir2_opcode(pir2);
    lsassert(latxs_ir2_opcode_is_branch(opc));
    switch (opc) {
    case LISA_BEQZ:
    case LISA_BNEZ: return 1; break;
    case LISA_B:
    case LISA_BL:   return 0; break;
    case LISA_BEQ:
    case LISA_BNE:
    case LISA_BLT:
    case LISA_BGE:
    case LISA_BLTU:
    case LISA_BGEU: return 2; break;
    case LISA_BCEQZ:
    case LISA_BCNEZ: return 1; break;
    default: return -1; break;
    }
    return -1;
}

/* Functions to generate IR2_INST and add into the linked list */
IR2_INST *latxs_append_ir2_opnd3i(IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        IR2_OPND *opnd2, int imm)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build4(pir2, opcode, opnd0, opnd1, opnd2, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2ii(IR2_OPCODE opcode,
        IR2_OPND *opnd0, IR2_OPND *opnd1,
        int imm1, int imm2)
{
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND ir2_opnd2, ir2_opnd3;
    latxs_ir2_opnd_build(&ir2_opnd2, IR2_OPND_IMMH, imm1);
    latxs_ir2_opnd_build(&ir2_opnd3, IR2_OPND_IMMH, imm2);

    switch (opcode) {
    case LISA_BSTRINS_W:
    case LISA_BSTRINS_D:
    case LISA_BSTRPICK_W:
    case LISA_BSTRPICK_D:
        latxs_ir2_build4(pir2, opcode,
                opnd0, opnd1, &ir2_opnd2, &ir2_opnd3);
        break;
    default:
        lsassertm(0, "append opnd2ii unsupport LISA instruciton (%d)%s\n",
                opcode, latxs_ir2_name(opcode));
        break;
    }

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd3(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1, IR2_OPND *opnd2)
{
    IR2_INST *pir2 = ir2_allocate();
    latxs_ir2_build3(pir2, opcode, opnd0, opnd1, opnd2);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd4(IR2_OPCODE opcode,
    IR2_OPND  *opnd0, IR2_OPND *opnd1, IR2_OPND *opnd2, IR2_OPND *opnd3)
{
    IR2_INST *pir2 = ir2_allocate();
    latxs_ir2_build4(pir2, opcode, opnd0, opnd1, opnd2, opnd3);
    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));
    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2i(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1, int32_t imm)
{
    IR2_INST *pir2 = ir2_allocate();

    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build3(pir2, opcode, opnd0, opnd1, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1)
{
    if (latxs_ir2_opcode_is_load(opcode) ||
        latxs_ir2_opcode_is_store(opcode)) {
        /* opnd1 could be IR2 OPND MEM */
        if (latxs_ir2_opnd_type(opnd1) == IR2_OPND_MEMY) {
            IR2_OPND base = latxs_ir2_opnd_mem_get_base(opnd1);
            int offset = latxs_ir2_opnd_mem_get_offset(opnd1);
            return latxs_append_ir2_opnd2i(opcode, opnd0, &base, offset);
        }
    }

    IR2_INST *pir2 = ir2_allocate();
    latxs_ir2_build2(pir2, opcode, opnd0, opnd1);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd1i(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, int32 imm)
{
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build2(pir2, opcode, opnd0, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd1(IR2_OPCODE opcode,
        IR2_OPND  *opnd0)
{
    IR2_INST *pir2 = ir2_allocate();
    latxs_ir2_build1(pir2, opcode, opnd0);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opndi(IR2_OPCODE opcode,
        int32_t imm)
{
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, imm);
    latxs_ir2_build1(pir2, opcode, &imm_opnd);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnda(IR2_OPCODE opcode,
        ADDR addr)
{
    IR2_INST *pir2 = ir2_allocate();
    IR2_OPND imm_opnd = latxs_ir2_opnd_new(IR2_OPND_IMMH, addr);

    switch (opcode) {
    case LISA_B:
    case LISA_BL:
        lsassert(int32_in_int26(addr));
        latxs_ir2_build1(pir2, opcode, &imm_opnd);
        break;
    case LISA_X86_INST:
    case LISA_DUP:
        latxs_ir2_build1(pir2, opcode, &imm_opnd);
        break;
    default:
        lsassertm(0, "append opnda unsupport LISA instruciton (%d)%s\n",
                     opcode, latxs_ir2_name(opcode));
        break;
    }

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd0(
        IR2_OPCODE opcode)
{
    IR2_INST *pir2 = ir2_allocate();
    latxs_ir2_build0(pir2, opcode);

    lsassertm(latxs_ir2_op_check(pir2),
            "Maybe you should check the type of operand %s\n",
            latxs_ir2_name(opcode));

    ir2_append(pir2);
    return pir2;
}

IR2_INST *latxs_append_ir2_opnd2_(IR2_OPCODE opcode,
        IR2_OPND  *opnd0, IR2_OPND *opnd1)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_mov:
        pir2 = latxs_append_ir2_opnd3(LISA_OR,
                opnd0, opnd1, &latxs_zero_ir2_opnd);
        break;
    case lisa_mov32z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1, 31, 0);
        break;
    case lisa_mov24z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1, 23, 0);
        break;
    case lisa_mov16z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1, 15, 0);
        break;
    case lisa_mov8z:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_D,
                opnd0, opnd1,  7, 0);
        break;
    case lisa_mov32s:
        pir2 = latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W,
                opnd0, opnd1, 31, 0);
        break;
    case lisa_mov16s:
        pir2 = latxs_append_ir2_opnd2(LISA_EXT_W_H, opnd0, opnd1);
        break;
    case lisa_mov8s:
        pir2 = latxs_append_ir2_opnd2(LISA_EXT_W_B, opnd0, opnd1);
        break;
    case lisa_not:
        pir2 = latxs_append_ir2_opnd3(LISA_NOR,
                opnd0, opnd1, &latxs_zero_ir2_opnd);
        break;
    default:
        lsassertm(0, "append opnd2_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

IR2_INST *latxs_append_ir2_opnd1_(IR2_OPCODE opcode,
        IR2_OPND  *opnd0)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_call:
        pir2 = latxs_append_ir2_opnd2i(LISA_JIRL,
                &latxs_ra_ir2_opnd, opnd0, 0);
        break;
    case lisa_jr:
        pir2 = latxs_append_ir2_opnd2i(LISA_JIRL,
                &latxs_zero_ir2_opnd, opnd0, 0);
        break;
    default:
        lsassertm(0, "append opnd1_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

IR2_INST *latxs_append_ir2_opnd0_(IR2_OPCODE opcode)
{
    IR2_INST *pir2 = NULL;

    switch (opcode) {
    case lisa_return:
        pir2 = latxs_append_ir2_opnd2i(LISA_JIRL,
                &latxs_zero_ir2_opnd,
                &latxs_ra_ir2_opnd, 0);
        break;
    case lisa_nop:
        pir2 = latxs_append_ir2_opnd2i(LISA_ANDI,
                &latxs_zero_ir2_opnd,
                &latxs_zero_ir2_opnd, 0);
        break;
    default:
        lsassertm(0, "append opnd0_ only for fake LISA.\n");
        break;
    }

    return pir2;
}

bool latxs_ir2_opcode_is_load(IR2_OPCODE opcode)
{
    return la_ir2_opcode_is_load(opcode);
}
bool latxs_ir2_opcode_is_store(IR2_OPCODE opcode)
{
    return la_ir2_opcode_is_store(opcode);
}
bool latxs_ir2_opcode_is_branch(IR2_OPCODE opcode)
{
    return (opcode >= LISA_BEQZ && opcode <= LISA_BNEZ) ||
           (opcode >= LISA_BCEQZ && opcode <= LISA_BCNEZ) ||
           (opcode >= LISA_B && opcode <= LISA_BGEU);
}
bool latxs_ir2_opcode_is_convert(IR2_OPCODE opcode)
{
    return ir2_opcode_is_convert(opcode);
}
bool latxs_ir2_opcode_is_fcmp(IR2_OPCODE opcode)
{
    return ir2_opcode_is_fcmp(opcode);
}

#endif
