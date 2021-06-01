#include "latx-options.h"
#include "error.h"
#include "qemu/osdep.h"

int option_by_hand;
int option_flag_pattern;
int option_flag_reduction;
int option_dump;
int option_dump_host;
int option_dump_ir1;
int option_dump_ir2;
int option_trace_tb;
int option_trace_ir1;
int option_check;
int option_tb_link;
int option_shadow_stack;
int option_lsfpu;
int option_profile;
int option_xmm128map;
int option_ibtc;

unsigned long long counter_tb_exec;
unsigned long long counter_tb_tr;

unsigned long long counter_ir1_tr;
unsigned long long counter_mips_tr;

void options_init(void)
{
    option_dump_host = 0;
    option_dump_ir1 = 0;
    option_dump_ir2 = 0;
    option_dump = 0;
    option_trace_tb = 0;
    option_trace_ir1 = 0;
    option_check = 0;
#if defined(CONFIG_LATX_DEBUG)
    option_by_hand = 0;
    option_flag_pattern = 0;
    option_flag_reduction = 0;
    option_tb_link = 0;
#else
    option_by_hand = 1;
    option_flag_pattern = 1;
    option_flag_reduction = 1;
    option_tb_link = 1;
#endif
    option_ibtc= 0;
    option_shadow_stack = 0;
    option_lsfpu = 0;
    option_xmm128map = 1;

    counter_tb_exec = 0;
    counter_tb_tr = 0;

    counter_ir1_tr = 0;
    counter_mips_tr = 0;

}

#define OPTIONS_DUMP_FUNC 0
#define OPTIONS_DUMP_IR1 1
#define OPTIONS_DUMP_IR2 2
#define OPTIONS_DUMP_HOST 3

void options_parse_dump(const char *bits)
{
    if (!bits) {
        return;
    }

    if (bits[OPTIONS_DUMP_IR1] == '1') {
        option_dump_ir1 = 1;
    } else if (bits[OPTIONS_DUMP_IR1] == '0') {
        option_dump_ir1 = 0;
    } else {
        lsassertm(0, "wrong options for dump ir1.");
    }

    if (bits[OPTIONS_DUMP_IR2] == '1') {
        option_dump_ir2 = 1;
    } else if (bits[OPTIONS_DUMP_IR2] == '0') {
        option_dump_ir2 = 0;
    } else {
        lsassertm(0, "wrong options for dump ir2.");
    }

    if (bits[OPTIONS_DUMP_HOST] == '1') {
        option_dump_host = 1;
    } else if (bits[OPTIONS_DUMP_HOST] == '0') {
        option_dump_host = 0;
    } else {
        lsassertm(0, "wrong options for dump host.");
    }

    if (bits[OPTIONS_DUMP_FUNC] == '1') {
        option_dump = 1;
    } else if (bits[OPTIONS_DUMP_FUNC] == '0') {
        option_dump = 0;
    } else {
        lsassertm(0, "wrong options for dump func.");
    }
}

#define OPTIONS_TRACE_TB 0
#define OPTIONS_TRACE_IR1 1

void options_parse_trace(const char *bits)
{
    if (!bits) {
        return;
    }

    if (bits[OPTIONS_TRACE_TB] == '1') {
        option_trace_tb = 1;
    } else if (bits[OPTIONS_TRACE_TB] == '0') {
        option_trace_tb = 0;
    } else {
        lsassertm(0, "wrong options for trace tb.");
    }

    if (bits[OPTIONS_TRACE_IR1] == '1') {
        option_trace_ir1 = 1;
    } else if (bits[OPTIONS_TRACE_IR1] == '0') {
        option_trace_ir1 = 0;
    } else {
        lsassertm(0, "wrong options for trace ir1 .");
    }
}

uint8 options_to_save(void)
{
    uint8 option_bitmap = 0;
    if (option_shadow_stack)
        option_bitmap |= 0x10;
    return option_bitmap;
}
