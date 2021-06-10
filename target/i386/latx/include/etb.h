#ifndef _ETB_H_
#define _ETB_H_

#include "ir1.h"
#include "qemu-def.h"

#define MAX_DEPTH 5 
typedef enum {
    TB_TYPE_NONE = 80,
    TB_TYPE_BRANCH,
    TB_TYPE_CALL,
    TB_TYPE_RETURN,
    TB_TYPE_JUMP,
    TB_TYPE_CALLIN,
    TB_TYPE_JUMPIN,
} TB_TYPE;

#define ETB_ARRAY_SIZE (1<<19)
extern ETB *etb_array[ETB_ARRAY_SIZE];
extern int etb_num;

/* func to access EXTRA TB */
static inline ETB *qm_tb_get_extra_tb(void *tb)
{
    struct TranslationBlock *ptb = (struct TranslationBlock *)tb;
    return &ptb->extra_tb;
}

/* functions to access ETB items */
static inline IR1_INST *etb_ir1_inst_first(ETB *etb)
{
    return etb->_ir1_instructions;
}

static inline IR1_INST *etb_ir1_inst_last(ETB *etb)
{
    return etb->_ir1_instructions + etb->_ir1_num - 1;
}

static inline IR1_INST *etb_ir1_inst(ETB *etb, const int i)
{
    return etb->_ir1_instructions + i;
}

static inline int etb_ir1_num(ETB *etb)
{
    return etb->_ir1_num;
}

static inline int8 etb_get_top_in(struct TranslationBlock *tb)
{
    return tb->_top_in;
}

static inline void etb_check_top_in(struct TranslationBlock *tb, int top_in)
{
    if (tb->_top_in == -1) {
        lsassert(tb->_top_out == -1);
        lsassert(top_in >= 0 && top_in <= 7);
        tb->_top_in = top_in;
    /* } else {
     *     assertm(top_in() == top, "\n%s: TB<0x%x>: top_in<%d> does not equal
     *     top<%d>\n\
     *         NOTE: last_tb_executed: first time: 0x%x, current time: 0x%x\n",\
     *         BIN_INFO::get_exe_short_name(), this->addr(), top_in(), top,\
     *         _last_tb_x86_addr, env->last_executed_tb()->addr());
     */
    }
}

static inline void etb_check_top_out(struct TranslationBlock *tb, int top_out)
{
    lsassert(tb->_top_in != -1);

    if (tb->_top_out == -1) {
        lsassert(top_out >= 0 && top_out <= 7);
        tb->_top_out = top_out;
    /* } else {
     *     assertm(top_out() == top, "\n%s: TB<0x%x>: top_out<%d> does not equal
     *     top<%d>\n\
     *         NOTE: last_tb_executed: first time: 0x%x, current time: 0x%x\n",\
     *         BIN_INFO::get_exe_short_name(), this->addr(), top_out(), top,\
     *         _last_tb_x86_addr, env->last_executed_tb()->addr());
     */
    }
}

void etb_qht_init(void);

#endif
