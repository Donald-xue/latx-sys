#include "common.h"
#include "reg-alloc.h"
#include "env.h"
#include "latx-options.h"

bool translate_popf(IR1_INST *pir1)
{
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    IR2_OPND eflags_opnd = ra_alloc_eflags();

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, esp_opnd, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, esp_opnd, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i(LISA_LD_W, eflags_opnd, tmp,
                -lsenv->tr_data->curr_esp_need_decrease);
        ra_free_temp(tmp);
        ra_free_temp(gbase);
    } else {
        la_append_ir2_opnd2i(LISA_LD_W, eflags_opnd, esp_opnd,
                -lsenv->tr_data->curr_esp_need_decrease);
    }
    if (option_lbt) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, eflags_opnd, 0x3f);
        la_append_ir2_opnd2i(LISA_ANDI, eflags_opnd, eflags_opnd,  0x400);
    }

    la_append_ir2_opnd2i(LISA_ORI, eflags_opnd, eflags_opnd, 0x202);

    // IR1_OPCODE next_opcode = ((IR1_INST *)(pir1 + 1))->_opcode;
    IR1_OPCODE next_opcode = ir1_opcode((IR1_INST *)(pir1 + 1));

    if (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
        next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF) {
        if (4 - lsenv->tr_data->curr_esp_need_decrease != 0)
            la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                              4 - lsenv->tr_data->curr_esp_need_decrease);
        lsenv->tr_data->curr_esp_need_decrease = 0;
    } else
        lsenv->tr_data->curr_esp_need_decrease -= 4;

    return true;
}

bool translate_pushf(IR1_INST *pir1)
{
    IR2_OPND eflags_opnd = ra_alloc_eflags();
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);

    if (option_lbt) {
        IR2_OPND temp   = ra_alloc_itemp();
        la_append_ir2_opnd1i_em(LISA_X86MFFLAG, temp, 0x3f);
        la_append_ir2_opnd3(LISA_OR, eflags_opnd, eflags_opnd, temp);
        ra_free_temp(temp);
    }

    if (cpu_get_guest_base() != 0) {
        IR2_OPND tmp = ra_alloc_itemp();
        IR2_OPND gbase = ra_alloc_guest_base();
        if (!ir2_opnd_is_address(&esp_opnd)) {
            la_append_ir2_opnd2_em(LISA_MOV_ADDRX, esp_opnd, esp_opnd);
        }
        la_append_ir2_opnd3_em(LISA_ADD_ADDR, tmp, esp_opnd, gbase);
        ir2_opnd_set_em(&tmp, EM_MIPS_ADDRESS, 32);
        la_append_ir2_opnd2i(LISA_ST_W, eflags_opnd, tmp,
                -4 - lsenv->tr_data->curr_esp_need_decrease);
        ra_free_temp(tmp);
        ra_free_temp(gbase);
    } else {
        la_append_ir2_opnd2i(LISA_ST_W, eflags_opnd, esp_opnd,
                -4 - lsenv->tr_data->curr_esp_need_decrease);
    }

    IR1_OPCODE next_opcode = ir1_opcode((IR1_INST *)(pir1 + 1));
    if (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
        next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF) {
        if (-4 - lsenv->tr_data->curr_esp_need_decrease != 0)
            la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                              -4 - lsenv->tr_data->curr_esp_need_decrease);
        lsenv->tr_data->curr_esp_need_decrease = 0;
    } else
        lsenv->tr_data->curr_esp_need_decrease += 4;

    return true;
}

bool translate_clc(IR1_INST *pir1) {
    if (option_lbt) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x1);
    } else {
        IR2_OPND eflags = ra_alloc_eflags();

        la_append_ir2_opnd2ii(LISA_BSTRINS_W, eflags, zero_ir2_opnd, 0, 0);
    }

    return true;
}

bool translate_cld(IR1_INST *pir1)
{
    IR2_OPND eflags = ra_alloc_eflags();

    la_append_ir2_opnd2ii(LISA_BSTRINS_W, eflags, zero_ir2_opnd, 10, 10);

    return true;
}

bool translate_stc(IR1_INST *pir1) {
    if (option_lbt) {
        la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);
    } else {
        IR2_OPND eflags = ra_alloc_eflags();

        la_append_ir2_opnd2i(LISA_ORI, eflags, eflags, 1);
    }

    return true;
}

bool translate_std(IR1_INST *pir1)
{
    IR2_OPND eflags = ra_alloc_eflags();

    la_append_ir2_opnd2i_em(LISA_ORI, eflags, eflags, 0x400);

    return true;
}
