#include "common.h"
#include "la-ir2.h"
#include "ir2-inst.h"

ADDR ir2_addr(IR2_INST *ir2)
{
    return ir2->_addr;
}

IR2_OPCODE ir2_opcode(IR2_INST *ir2)
{
    return (IR2_OPCODE)(ir2->_opcode);
}

int ir2_get_id(IR2_INST *ir2)
{
    return ir2->_id;
}

void ir2_set_addr(IR2_INST *ir2, ADDR a)
{
    ir2->_addr = a;
}

void ir2_set_opcode(IR2_INST *ir2, IR2_OPCODE type)
{
    ir2->_opcode = type;
}

void ir2_set_id(IR2_INST *ir2, int id)
{
    ir2->_id = id;
}
