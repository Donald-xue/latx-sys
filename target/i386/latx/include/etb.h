#ifndef _ETB_H_
#define _ETB_H_

#include "common.h"
#include "latx-config.h"
#include "ir1/ir1.h"

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

/* func to access EXTRA TB */
ETB *qm_tb_get_extra_tb(void *tb);

/* functions to access ETB items */
IR1_INST *etb_ir1_inst_first(ETB *etb);
IR1_INST *etb_ir1_inst_last(ETB *etb);
IR1_INST *etb_ir1_inst(ETB *etb, const int i);
int etb_ir1_num(ETB *etb);

/* flag_pattern.c */
void tb_find_flag_pattern(void *tb);

/* flag_reduction.c */
void tb_flag_reduction(void *tb);
void tb_flag(void *tb);
uint8 pending_use_of_succ(ETB* etb, int max_depth);
void free_etb(ETB* etb); 

void etb_check_top_out(ETB *etb, int top);
int8 etb_get_top_in(ETB *etb);
void etb_set_top_in(ETB *etb, int8 top_in);
int8 etb_get_top_out(ETB *etb);
void etb_set_top_out(ETB *etb, int8 top_out);
int8 etb_get_top_increment(ETB *etb);

#endif
