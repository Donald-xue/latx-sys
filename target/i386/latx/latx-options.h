#ifndef _LATX_OPTIONS_H_
#define _LATX_OPTIONS_H_

#include "types.h"

extern int option_by_hand;
extern int option_flag_pattern;
extern int option_flag_reduction;
extern int option_dump;
extern int option_dump_host;
extern int option_dump_ir1;
extern int option_dump_ir2;
extern int option_trace_tb;
extern int option_trace_ir1;
extern int option_check;
extern int option_tb_link;
extern int option_lbt;
extern int option_shadow_stack;
extern int option_lsfpu;
extern int option_profile;
extern int option_xmm128map;
extern int option_ibtc;

extern unsigned long long counter_tb_exec;
extern unsigned long long counter_tb_tr;

extern unsigned long long counter_ir1_tr;
extern unsigned long long counter_mips_tr;

void options_init(void);
void options_parse_dump(const char *bits);
void options_parse_trace(const char *bits);
uint8 options_to_save(void);

#endif
