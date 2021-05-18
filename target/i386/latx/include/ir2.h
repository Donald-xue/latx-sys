#ifndef _IR2_H_
#define _IR2_H_

#include "common.h"
#include "la-ir2.h"

#define IR2_ITEMP_MAX 4096

void ir2_opnd_build(IR2_OPND *, IR2_OPND_TYPE, int value);
void ir2_opnd_build2(IR2_OPND *, IR2_OPND_TYPE, int base, int16 offset);
void ir2_opnd_build_none(IR2_OPND *);
void ir2_opnd_build_type(IR2_OPND *, IR2_OPND_TYPE);

IR2_OPND ir2_opnd_new(IR2_OPND_TYPE, int value);
IR2_OPND ir2_opnd_new2(IR2_OPND_TYPE, int base, int16 offset);
IR2_OPND ir2_opnd_new_none(void);
IR2_OPND ir2_opnd_new_type(IR2_OPND_TYPE);

int16 ir2_opnd_imm(IR2_OPND *);
int ir2_opnd_base_reg_num(IR2_OPND *);
IR2_OPND_TYPE ir2_opnd_type(IR2_OPND *);
EXTENSION_MODE ir2_opnd_default_em(IR2_OPND *);
uint32 ir2_opnd_addr(IR2_OPND *);
int ir2_opnd_eb(IR2_OPND *);
EXTENSION_MODE ir2_opnd_em(IR2_OPND *);
int32 ir2_opnd_label_id(IR2_OPND *);

int ir2_opnd_cmp(IR2_OPND *, IR2_OPND *);

int ir2_opnd_is_ireg(IR2_OPND *);
int ir2_opnd_is_freg(IR2_OPND *);
int ir2_opnd_is_creg(IR2_OPND *);
int ir2_opnd_is_itemp(IR2_OPND *);
int ir2_opnd_is_ftemp(IR2_OPND *);
int ir2_opnd_is_mem_base_itemp(IR2_OPND *);
int ir2_opnd_is_mem(IR2_OPND *);
int ir2_opnd_is_imm(IR2_OPND *);
int ir2_opnd_is_label(IR2_OPND *);
int ir2_opnd_is_sx(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_zx(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_bx(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_ax(IR2_OPND *, int bits); /* default bits = 32 */
int ir2_opnd_is_address(IR2_OPND *);
int ir2_opnd_is_x86_address(IR2_OPND *);
int ir2_opnd_is_mips_address(IR2_OPND *);

void ir2_opnd_set_em(IR2_OPND *, EXTENSION_MODE,
                     int bits); /* default bits = 32 */
void ir2_opnd_set_em_add(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_add2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_sub(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_sub2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_xor(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_xor2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_or(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                        int eb1);
void ir2_opnd_set_em_or2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_and(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_and2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_nor(IR2_OPND *, IR2_OPND *src0, EXTENSION_MODE em1,
                         int eb1);
void ir2_opnd_set_em_nor2(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_dsra(IR2_OPND *, IR2_OPND *src, int shift);
void ir2_opnd_set_em_dsll(IR2_OPND *, IR2_OPND *src, int shift);
void ir2_opnd_set_em_mov(IR2_OPND *, IR2_OPND *src);
void ir2_opnd_set_em_movcc(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_dsrl(IR2_OPND *, IR2_OPND *src, int shift);
void ir2_opnd_set_em_dmult_g(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);
void ir2_opnd_set_em_dmultu_g(IR2_OPND *, IR2_OPND *src0, IR2_OPND *src1);

int ir2_opnd_to_string(IR2_OPND *, char *, bool);

void ir2_opnd_convert_label_to_imm(IR2_OPND *, int imm);

typedef struct IR2_INST {
    ADDR _addr;
    int16 _opcode;
    int16 _id;
    int16 _prev;
    int16 _next;
    int op_count;
    IR2_OPND _opnd[4]; /*LA has 4 opnds*/
} IR2_INST;

void ir2_build(IR2_INST *, IR2_OPCODE, IR2_OPND, IR2_OPND, IR2_OPND);

void ir2_set_id(IR2_INST *, int);
int ir2_get_id(IR2_INST *);
IR2_OPCODE ir2_opcode(IR2_INST *);
void ir2_set_opcode(IR2_INST *ir2, IR2_OPCODE type);
int ir2_dump(IR2_INST *);
int ir2_to_string(IR2_INST *, char *);
IR2_INST *ir2_prev(IR2_INST *);
IR2_INST *ir2_next(IR2_INST *);
ADDR ir2_addr(IR2_INST *);
void ir2_set_addr(IR2_INST *, ADDR a);

void ir2_append(IR2_INST *);
void ir2_remove(IR2_INST *);
void ir2_insert_before(IR2_INST *ir2, IR2_INST *next);
void ir2_insert_after(IR2_INST *ir2, IR2_INST *prev);

uint32 ir2_assemble(IR2_INST *);
bool ir2_op_check(IR2_INST *);


bool ir2_opcode_is_branch(IR2_OPCODE);
bool ir2_opcode_is_branch_with_3opnds(IR2_OPCODE);
bool ir2_opcode_is_f_branch(IR2_OPCODE opcode);
bool ir2_opcode_is_convert(IR2_OPCODE opcode);
bool ir2_opcode_is_fcmp(IR2_OPCODE opcode);

extern IR2_OPND zero_ir2_opnd;
extern IR2_OPND env_ir2_opnd;
extern IR2_OPND sp_ir2_opnd;
extern IR2_OPND n1_ir2_opnd;
extern IR2_OPND f32_ir2_opnd;
extern IR2_OPND fcsr_ir2_opnd;
extern IR2_OPND fp_ir2_opnd;
extern IR2_OPND a0_ir2_opnd;
extern IR2_OPND t5_ir2_opnd;
extern IR2_OPND ra_ir2_opnd;
extern IR2_OPND fcc0_ir2_opnd;
extern IR2_OPND fcc1_ir2_opnd;
extern IR2_OPND fcc2_ir2_opnd;
extern IR2_OPND fcc3_ir2_opnd;
extern IR2_OPND fcc4_ir2_opnd;
extern IR2_OPND fcc5_ir2_opnd;
extern IR2_OPND fcc6_ir2_opnd;
extern IR2_OPND fcc7_ir2_opnd;

IR2_OPND create_ir2_opnd(IR2_OPND_TYPE type, int val);
IR2_INST *la_append_ir2_opnd0(IR2_OPCODE type);
IR2_INST *la_append_ir2_opndi(IR2_OPCODE type, int imm);
IR2_INST *la_append_ir2_opnd1(IR2_OPCODE type, IR2_OPND op0);
IR2_INST *la_append_ir2_opnd1i(IR2_OPCODE type, IR2_OPND op0, int imm);
IR2_INST *la_append_ir2_opnd2(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1);
IR2_INST *la_append_ir2_opnd2i(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, int imm);
IR2_INST *la_append_ir2_opnd2ii(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, int imm0, int imm1);
IR2_INST *la_append_ir2_opnd3(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, IR2_OPND op2);
IR2_INST *la_append_ir2_opnd3i(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, IR2_OPND op2, int imm0);
IR2_INST *la_append_ir2_opnd4(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1, IR2_OPND op2, IR2_OPND op3);

IR2_INST *la_append_ir2_opnd1i_em(IR2_OPCODE opcode, IR2_OPND op0, int imm);
IR2_INST *la_append_ir2_opnd2_em(IR2_OPCODE type, IR2_OPND op0, IR2_OPND op1);
IR2_INST *la_append_ir2_opnd2i_em(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, int imm);
IR2_INST *la_append_ir2_opnd3_em(IR2_OPCODE type, IR2_OPND op0,IR2_OPND op1, IR2_OPND op2);
IR2_INST *la_append_ir2_opnda(IR2_OPCODE opcode, ADDR addr);
bool la_ir2_opcode_is_store(IR2_OPCODE opcode);
bool la_ir2_opcode_is_load(IR2_OPCODE opcode);
#endif
