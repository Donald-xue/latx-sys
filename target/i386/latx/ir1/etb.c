#include "common.h"
#include "mem.h"
#include "env.h"
#include "etb.h"
#include "ir2/ir2.h"

/*
 * functions to access ETB items
 */
IR1_INST *etb_ir1_inst_first(ETB *etb) { return etb->_ir1_instructions; }

IR1_INST *etb_ir1_inst_last(ETB *etb)
{
    return etb->_ir1_instructions + etb->_ir1_num - 1;
}

IR1_INST *etb_ir1_inst(ETB *etb, const int i)
{
    return etb->_ir1_instructions + i;
}

int etb_ir1_num(ETB *etb) { return etb->_ir1_num; }

int8 etb_get_top_in(ETB *etb) { return etb->_top_in; }

void etb_set_top_in(ETB *etb, int8 top_in)
{
    lsassert(top_in >= 0 && top_in <= 7);
    etb->_top_in = top_in;
}

int8 etb_get_top_out(ETB *etb) { return etb->_top_out; }

void etb_set_top_out(ETB *etb, int8 top_out)
{
    lsassert(top_out >= 0 && top_out <= 7);
    etb->_top_out = top_out;
}

int8 etb_get_top_increment(ETB *etb) { return etb->_top_increment; }

void etb_check_top_in(struct TranslationBlock *tb, int top)
{
    ETB *etb = &tb->extra_tb;
    if (etb_get_top_in(etb) == -1) {
        lsassert(etb_get_top_out(etb) == -1);
        etb_set_top_in(etb, top);
    } else {
        /*
        assertm(top_in() == top, "\n%s: TB<0x%x>: top_in<%d> does not equal
        top<%d>\n\
            NOTE: last_tb_executed: first time: 0x%x, current time: 0x%x\n",\
            BIN_INFO::get_exe_short_name(), this->addr(), top_in(), top,\
            _last_tb_x86_addr, env->last_executed_tb()->addr());
            */
    }
}

void etb_check_top_out(ETB *etb, int top)
{
    lsassert(etb_get_top_in(etb) != -1);

    if (etb_get_top_out(etb) == -1) {
        etb_set_top_out(etb, top);
    } else {
        /*
        assertm(top_out() == top, "\n%s: TB<0x%x>: top_out<%d> does not equal
        top<%d>\n\
            NOTE: last_tb_executed: first time: 0x%x, current time: 0x%x\n",\
            BIN_INFO::get_exe_short_name(), this->addr(), top_out(), top,\
            _last_tb_x86_addr, env->last_executed_tb()->addr());
            */
    }
}
