#ifndef _IR2_INST_H_
#define _IR2_INST_H_

#ifndef CONFIG_SOFTMMU
typedef struct IR2_INST {
    ADDR _addr;
    int16 _opcode;
    int16 _id;
    int op_count;
    IR2_OPND _opnd[4]; /*LA has 4 opnds*/
} IR2_INST;
#else
typedef struct IR2_INST {
    ADDR _addr;
    int _opcode;
    int _id;
    int op_count;
    IR2_OPND _opnd[4];
} IR2_INST;
#endif

ADDR ir2_addr(IR2_INST *);
IR2_OPCODE ir2_opcode(IR2_INST *);
int  ir2_get_id(IR2_INST *);

void ir2_set_addr(IR2_INST *, ADDR a);
void ir2_set_opcode(IR2_INST *ir2, IR2_OPCODE type);
void ir2_set_id(IR2_INST *, int);

#endif
