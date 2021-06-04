#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"

bool translate_pop(IR1_INST *pir1)
{
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);

    int esp_increment = 4;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        esp_increment = 2;

    /* 1. if esp is used or this pir1 is the last pushpop, We should update esp
     */
    /* first; o.w, increase curr_esp_need_decrease */
    IR1_OPCODE next_opcode = ir1_opcode((IR1_INST *)(pir1 + 1));
    if (ir1_opnd_is_gpr_used(ir1_get_opnd(pir1, 0), esp_index) ||
        (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
         next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF)) {
        if (esp_increment - lsenv->tr_data->curr_esp_need_decrease != 0) {
            la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                esp_increment - lsenv->tr_data->curr_esp_need_decrease);
        }
        lsenv->tr_data->curr_esp_need_decrease = 0;
    } else {
        lsenv->tr_data->curr_esp_need_decrease -= esp_increment;
    }

    /*  2. pop value from mem */
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR1_OPND mem_ir1_opnd;
        // ir1_opnd_build_mem(&mem_ir1_opnd, IR1_OPND_MEM, 16, 4,
        //                -esp_increment - lsenv->tr_data->curr_esp_need_decrease);
        ir1_opnd_build_mem(&mem_ir1_opnd, 16, X86_REG_ESP, 
                        -esp_increment - lsenv->tr_data->curr_esp_need_decrease);

        if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) &&
            ir1_opnd_is_zx(ir1_get_opnd(pir1, 0), 16)) {
            /* when dest is gpr, and high 16 bits are zero, load into gpr */
            /* directly */
            IR2_OPND dest_opnd =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
            load_ireg_from_ir1_2(dest_opnd, &mem_ir1_opnd, ZERO_EXTENSION,
                                 false);
        } else {
            /* load value */
            IR2_OPND value_opnd =
                load_ireg_from_ir1(&mem_ir1_opnd, ZERO_EXTENSION, false);
            store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
        }
    } else {
        IR1_OPND mem_ir1_opnd;
        // ir1_opnd_build(&mem_ir1_opnd, IR1_OPND_MEM, 32, 4,
        //                -esp_increment - lsenv->tr_data->curr_esp_need_decrease);
        ir1_opnd_build_mem(&mem_ir1_opnd, 32, X86_REG_ESP,
                       -esp_increment - lsenv->tr_data->curr_esp_need_decrease);

        if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0))) {
            /* when dest is gpr, load into gpr directly */
            IR2_OPND dest_opnd =
                load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
            EXTENSION_MODE dest_em = SIGN_EXTENSION;
            if (ir2_opnd_default_em(&dest_opnd) != SIGN_EXTENSION)
                dest_em = ZERO_EXTENSION;

            load_ireg_from_ir1_2(dest_opnd, &mem_ir1_opnd, dest_em, false);
        } else {
            /* dest is mem, as normal */
            IR2_OPND value_opnd =
                load_ireg_from_ir1(&mem_ir1_opnd, SIGN_EXTENSION, false);
            store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false); /* TODO */
        }
    }

    return true;
}

bool translate_push(IR1_INST *pir1)
{
    /* 1. if esp is used by push, update esp now */
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    if ((ir1_opnd_is_gpr_used(ir1_get_opnd(pir1, 0), esp_index) != 0) &&
        (lsenv->tr_data->curr_esp_need_decrease != 0)) {
        la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                          -lsenv->tr_data->curr_esp_need_decrease);
        lsenv->tr_data->curr_esp_need_decrease = 0;
    }

    int esp_decrement = 4;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        esp_decrement = 2;

    /* 2. push value onto stack */
    IR1_OPND mem_ir1_opnd;
    // ir1_opnd_build(&mem_ir1_opnd, IR1_OPND_MEM, esp_decrement << 3, 4,
    //                -esp_decrement - lsenv->tr_data->curr_esp_need_decrease);
    ir1_opnd_build_mem(&mem_ir1_opnd, esp_decrement << 3, X86_REG_ESP,
                    -esp_decrement - lsenv->tr_data->curr_esp_need_decrease);

    IR2_OPND value_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);
    store_ireg_to_ir1(value_opnd, &mem_ir1_opnd, false);

    /* 3. adjust esp */
    IR1_OPCODE next_opcode = ir1_opcode(((IR1_INST *)(pir1 + 1)));
    if (next_opcode != X86_INS_PUSH && next_opcode != X86_INS_POP &&
        next_opcode != X86_INS_PUSHF && next_opcode != X86_INS_POPF) {
        la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
            -esp_decrement - lsenv->tr_data->curr_esp_need_decrease);
        lsenv->tr_data->curr_esp_need_decrease = 0;
    } else
        lsenv->tr_data->curr_esp_need_decrease += esp_decrement;

    return true;
}

bool translate_mov(IR1_INST *pir1)
{
    IR2_OPND source_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), UNKNOWN_EXTENSION, false);

    store_ireg_to_ir1(source_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool translate_movzx(IR1_INST *pir1)
{
    IR1_OPND *source_ir1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *dest_ir1 = ir1_get_opnd(pir1, 0);

    if((ir1_opnd_base_reg_num(source_ir1) == ir1_opnd_base_reg_num(dest_ir1))
        && (ir1_opnd_type(source_ir1) == X86_OP_REG))
    {
        /*
         * MOVZX in x86 have only three situation.
         *  --> MOVZX r16, r/m8, MOVZX r32, r/m8, MOVZX r32, r/m16
         * but if you want to support X64, you must add two case
         *  --> MOVZX r64, r/m8*, MOVZX r64, r/m16
         */
        IR2_OPND dest_opnd = ra_alloc_gpr(ir1_opnd_base_reg_num(source_ir1));

        if(ir1_opnd_is_8l(source_ir1)){
            if(ir1_opnd_is_16l(dest_ir1))
                la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, zero_ir2_opnd, 15, 8);
            else
                la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, zero_ir2_opnd, 31, 8);
        } else if(ir1_opnd_is_8h(source_ir1)){
            IR2_OPND tmp_opnd = ra_alloc_itemp();
            if(ir1_opnd_is_16l(dest_ir1)){
                la_append_ir2_opnd2ii(LISA_BSTRPICK_D, tmp_opnd, dest_opnd, 15, 8);
                la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, zero_ir2_opnd, 15, 8);
                la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, tmp_opnd, 7, 0);
            } else {
                la_append_ir2_opnd2ii(LISA_BSTRPICK_D, tmp_opnd, dest_opnd, 15, 8);
                la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, zero_ir2_opnd, 31, 8);
                la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, tmp_opnd, 7, 0);
            }
        } else {
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, zero_ir2_opnd, 31, 16);
        }
        return true;
    }
    IR2_OPND source_opnd =
        load_ireg_from_ir1(source_ir1, ZERO_EXTENSION, false);

    store_ireg_to_ir1(source_opnd, dest_ir1, false);
    return true;
}

bool translate_movsx(IR1_INST *pir1)
{
    IR2_OPND source_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);

    store_ireg_to_ir1(source_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

static void load_step_to_reg(IR2_OPND *p_step_opnd, IR1_INST *pir1)
{
    IR2_OPND df_opnd = ra_alloc_itemp();
    IR2_OPND eflags_opnd = ra_alloc_eflags();
    la_append_ir2_opnd2i_em(LISA_ANDI, df_opnd, eflags_opnd, 0x400);

    int bytes = ir1_opnd_size(ir1_get_opnd(pir1, 0)) >> 3;
    int bits = 0;
    switch (ir1_opnd_size(ir1_get_opnd(pir1, 0))) {
    case 8:
        bits = 9;
        break;
    case 16:
        bits = 8;
        break;
    case 32:
        bits = 7;
        break;
    }
    lsassert(bits != 0);
    IR2_OPND tmp_step = ra_alloc_itemp();
    la_append_ir2_opnd2i_em(LISA_SRAI_W, tmp_step, df_opnd, bits);
    la_append_ir2_opnd2i_em(LISA_ADDI_W, *p_step_opnd, tmp_step, 0 - bytes);

    ra_free_temp(df_opnd);
    ra_free_temp(tmp_step);
}

bool translate_movs(IR1_INST *pir1)
{
    BITS_SET(pir1->flags, FI_MDA);

    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ecx_opnd = ir2_opnd_new_none();
    if (ir1_prefix(pir1) != 0) {
        ecx_opnd = load_ireg_from_ir1(&ecx_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_BEQ, ecx_opnd, zero_ir2_opnd, label_exit);
    }

    /* 2. preparations outside the loop */
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_step_to_reg(&step_opnd, pir1);

    /* 3. loop starts */
    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_LABEL, label_loop_begin);

    /* 3.1 load memory value at ESI, and store into memory at EDI */
    IR2_OPND esi_mem_value =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    store_ireg_to_ir1(esi_mem_value, ir1_get_opnd(pir1, 0), false);

    /* 3.2 adjust ESI and EDI */
    IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, esi_opnd, esi_opnd, step_opnd);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, edi_opnd, edi_opnd, step_opnd);

    /* 4. loop ends? when ecx==0 */
    if (ir1_prefix(pir1) != 0) {
        lsassert(ir1_prefix(pir1) == X86_PREFIX_REP);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ecx_opnd, ecx_opnd, -1);
        la_append_ir2_opnd3(LISA_BNE, ecx_opnd, zero_ir2_opnd, label_loop_begin);
    }

    /* 5. exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(step_opnd);
    return true;
}

bool translate_stos(IR1_INST *pir1)
{

    BITS_SET(pir1->flags, FI_MDA);

    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ecx_opnd = ir2_opnd_new_none();
    if (ir1_prefix(pir1) != 0) {
        ecx_opnd = load_ireg_from_ir1(&ecx_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_BEQ, ecx_opnd, zero_ir2_opnd, label_exit);
    }

    /* 2. preparations outside the loop */
    IR2_OPND eax_value_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_step_to_reg(&step_opnd, pir1);

    /* 3. loop starts */
    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_LABEL, label_loop_begin);

    /* 3.1 store EAX into memory at EDI */
    store_ireg_to_ir1(eax_value_opnd, ir1_get_opnd(pir1, 0), false);

    /* 3.2 adjust EDI */
    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, edi_opnd, edi_opnd, step_opnd);

    /* 4. loop ends? when ecx==0 */
    if (ir1_prefix(pir1) != 0) {
        lsassert(ir1_prefix(pir1) == X86_PREFIX_REP);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ecx_opnd, ecx_opnd, -1);
        la_append_ir2_opnd3(LISA_BNE, ecx_opnd, zero_ir2_opnd, label_loop_begin);
    }

    /* 5. exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(step_opnd);
    return true;
}

bool translate_lods(IR1_INST *pir1)
{
    BITS_SET(pir1->flags, FI_MDA);

    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ecx_opnd = ir2_opnd_new_none();
    if (ir1_prefix(pir1) != 0) {
        ecx_opnd = load_ireg_from_ir1(&ecx_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_BEQ, ecx_opnd, zero_ir2_opnd, label_exit);
    }

    /* 2. preparations outside the loop */
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_step_to_reg(&step_opnd, pir1);

    /* 3. loop starts */
    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_LABEL, label_loop_begin);

    /* 3.1 load memory value at ESI */
    EXTENSION_MODE em = SIGN_EXTENSION;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) < 32)
        em = ZERO_EXTENSION;
    IR2_OPND esi_mem_value = load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, em, false);

    /* 3.2 adjust ESI */
    IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, esi_opnd, esi_opnd, step_opnd);

    /* 4. loop ends? when ecx==0 */
    if (ir1_prefix(pir1) != 0) {
        lsassert(ir1_prefix(pir1) == X86_PREFIX_REP);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ecx_opnd, ecx_opnd, -1);
        la_append_ir2_opnd3(LISA_BNE, ecx_opnd, zero_ir2_opnd, label_loop_begin);
    }

    store_ireg_to_ir1(esi_mem_value, ir1_get_opnd(pir1, 0), false);

    /* 5. exit */
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(step_opnd);
    return true;
}

bool translate_cmps(IR1_INST *pir1)
{
    BITS_SET(pir1->flags, FI_MDA);

    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ecx_opnd = ir2_opnd_new_none();
    if (ir1_prefix(pir1) != 0) {
        ecx_opnd = load_ireg_from_ir1(&ecx_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_BEQ, ecx_opnd, zero_ir2_opnd, label_exit);
    }

    /* 2. preparations outside the loop */
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_step_to_reg(&step_opnd, pir1);

    /* 3. loop starts */
    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_LABEL, label_loop_begin);

    /* 3.1 load memory value at ESI and EDI */
    IR2_OPND esi_mem_value =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND edi_mem_value =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);

    /* 3.2 adjust ESI and EDI */
    IR2_OPND esi_opnd = ra_alloc_gpr(esi_index);
    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, esi_opnd, esi_opnd, step_opnd);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, edi_opnd, edi_opnd, step_opnd);

    /* 3.3 compare */
    IR2_OPND cmp_result = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, cmp_result, esi_mem_value, edi_mem_value);

    /* 4. loop ends? */
    if (ir1_prefix(pir1) != 0) {
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ecx_opnd, ecx_opnd, -1);

        /* 4.1. loop ends when ecx==0 */
        IR2_OPND condition = ra_alloc_itemp();
        la_append_ir2_opnd2i_em(LISA_SLTUI, condition, ecx_opnd,
                          1); /* set 1 when ecx==0 */

        /* 4.2 loop ends when result != 0 (repe), and when result==0 (repne) */
        IR2_OPND condition2 = ra_alloc_itemp();
        if (ir1_prefix(pir1) == X86_PREFIX_REP)
            la_append_ir2_opnd3_em(
                LISA_SLTU, condition2, zero_ir2_opnd,
                cmp_result); /* set 1 when 0<result, i.e., result!=0 */
        else
            la_append_ir2_opnd2i_em(LISA_SLTUI, condition2, cmp_result,
                              1); /* set 1 when result<1, i.e., result==0 */

        /* 4.3 when none of the two conditions is satisfied, the loop continues
         */
        la_append_ir2_opnd3_em(LISA_OR, condition, condition, condition2);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, label_loop_begin);
        ra_free_temp(condition);
        ra_free_temp(condition2);
    }

    /* 5. calculate eflags */
    generate_eflag_calculation(cmp_result, esi_mem_value, edi_mem_value, pir1,
                               true);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(step_opnd);
    ra_free_temp(edi_mem_value);
    ra_free_temp(cmp_result);
    return true;
}

bool translate_scas(IR1_INST *pir1)
{
    BITS_SET(pir1->flags, FI_MDA);

    /* 1. exit when initial count is zero */
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND ecx_opnd = ir2_opnd_new_none();
    if (ir1_prefix(pir1) != 0) {
        ecx_opnd = load_ireg_from_ir1(&ecx_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_BEQ, ecx_opnd, zero_ir2_opnd, label_exit);
    }

    /* 2. preparations outside the loop */
    // IR2_OPND eax_value_opnd =
    //     load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND eax_value_opnd =         //capstone eax first opnd
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND step_opnd = ra_alloc_itemp();
    load_step_to_reg(&step_opnd, pir1);

    /* 3. loop starts */
    IR2_OPND label_loop_begin = ir2_opnd_new_type(IR2_OPND_LABEL);
    la_append_ir2_opnd1(LISA_LABEL, label_loop_begin);

    /* 3.1 load memory value at EDI */
    // IR2_OPND edi_mem_value =       //capstone edi second opnd
    //     load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND edi_mem_value =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 1), SIGN_EXTENSION, false);
    /* 3.2 adjust edi */
    IR2_OPND edi_opnd = ra_alloc_gpr(edi_index);
    la_append_ir2_opnd3_em(LISA_SUB_ADDRX, edi_opnd, edi_opnd, step_opnd);

    /* 3.3 compare */
    IR2_OPND cmp_result = ra_alloc_itemp();
    la_append_ir2_opnd3_em(LISA_SUB_W, cmp_result, eax_value_opnd, edi_mem_value);

    /* 4. loop ends? */
    if (ir1_prefix(pir1) != 0) {
        la_append_ir2_opnd2i_em(LISA_ADDI_W, ecx_opnd, ecx_opnd, -1);

        /* 4.1. loop ends when ecx==0 */
        IR2_OPND condition = ra_alloc_itemp();
        la_append_ir2_opnd2i_em(LISA_SLTUI, condition, ecx_opnd,
                          1); /* set 1 when ecx==0 */

        /* 4.2 loop ends when result != 0 (repe), and when result==0 (repne) */
        IR2_OPND condition2 = ra_alloc_itemp();
        if (ir1_prefix(pir1) == X86_PREFIX_REP)
            la_append_ir2_opnd3_em(LISA_SLTU, condition2, zero_ir2_opnd,
                cmp_result); /* set 1 when 0<result, i.e., result!=0 */
        else
            la_append_ir2_opnd2i_em(LISA_SLTUI, condition2, cmp_result,
                              1); /* set 1 when result<1, i.e., result==0 */

        /* 4.3 when none of the two conditions is satisfied, the loop continues
         */
        la_append_ir2_opnd3_em(LISA_OR, condition, condition, condition2);
        la_append_ir2_opnd3(LISA_BEQ, condition, zero_ir2_opnd, label_loop_begin);
        ra_free_temp(condition);
        ra_free_temp(condition2);
    }

    /* 5. calculate eflags */
    generate_eflag_calculation(cmp_result, eax_value_opnd, edi_mem_value, pir1,
                               true);
    la_append_ir2_opnd1(LISA_LABEL, label_exit);

    ra_free_temp(step_opnd);
    ra_free_temp(edi_mem_value);
    ra_free_temp(cmp_result);
    return true;
}

bool translate_cmovo(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovno(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovb(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovae(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovz(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovnz(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovbe(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmova(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovs(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovns(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovp(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovnp(IR1_INST *pir1)
{
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovl(IR1_INST *pir1)
{ /* sf != of */
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovge(IR1_INST *pir1)
{ /* sf == of */
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovle(IR1_INST *pir1)
{ /* zf==1 || sf!=of */
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BEQZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_cmovg(IR1_INST *pir1)
{ /* zf==0 && sf==of */
    IR2_OPND neg_condition = ra_alloc_itemp();
    if (!fp_translate_pattern_tail(pir1, neg_condition)) {
        get_eflag_condition(&neg_condition, pir1);
    }

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, src_opnd, zero_ir2_opnd);
    } else {
        lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16);
        IR2_OPND src_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        IR2_OPND dest_opnd =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

        IR2_OPND dest_opnd_when_mov = src_opnd;
        bool dest_opnd_when_mov_is_temp = false;
        if (!ir2_opnd_is_zx(&dest_opnd, 16)) {
            dest_opnd_when_mov = ra_alloc_itemp();
            dest_opnd_when_mov_is_temp = true;
            la_append_ir2_opnd2i_em(LISA_SRAI_W, dest_opnd_when_mov, dest_opnd, 16);
            la_append_ir2_opnd2i_em(LISA_SLLI_W, dest_opnd_when_mov, dest_opnd_when_mov,
                              16);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_when_mov, dest_opnd_when_mov,
                             src_opnd);
        }

        la_append_ir2_opnd1i(LISA_BNEZ, neg_condition, 2); 
        la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd_when_mov, zero_ir2_opnd);
        if (dest_opnd_when_mov_is_temp)
            ra_free_temp(dest_opnd_when_mov);
    }

    ra_free_temp(neg_condition);
    return true;
}

bool translate_lea(IR1_INST *pir1)
{
    IR2_OPND value_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), UNKNOWN_EXTENSION, false);

    load_ireg_from_ir1_addrx(ir1_get_opnd(pir1, 0) + 1, value_opnd);

    store_ireg_to_ir1(value_opnd, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool translate_xchg(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_gpr(opnd0) && ir1_opnd_is_gpr(opnd1)) {
        /* if two src is the same reg, do nothing */
        if ((ir1_opnd_size(opnd0) == ir1_opnd_size(opnd1)) &&
            (ir1_opnd_base_reg_num(opnd0) == ir1_opnd_base_reg_num(opnd1)) &&
            (ir1_opnd_base_reg_bits_start(opnd0) ==
            ir1_opnd_base_reg_bits_start(opnd1))) {
            return true;
        } else {
            IR2_OPND src_opnd_0 =
                load_ireg_from_ir1(opnd0, UNKNOWN_EXTENSION, false);
            IR2_OPND src_opnd_1 =
                load_ireg_from_ir1(opnd1, UNKNOWN_EXTENSION, false);

            if (ir2_opnd_is_itemp(&src_opnd_1)  && !ir1_opnd_is_mem(opnd0)) {
                store_ireg_to_ir1(src_opnd_0, opnd1, false);
                store_ireg_to_ir1(src_opnd_1, opnd0, false);
                return true;
            }

            if (ir2_opnd_is_itemp(&src_opnd_0) && !ir1_opnd_is_mem((opnd1))) {
                store_ireg_to_ir1(src_opnd_1, opnd0, false);
                store_ireg_to_ir1(src_opnd_0, opnd1, false);
                return true;
            }

            /* none src is temp reg */
            IR2_OPND t_opnd = ra_alloc_itemp();
            la_append_ir2_opnd2_em(LISA_MOV64, t_opnd, src_opnd_1);

            store_ireg_to_ir1(src_opnd_0, opnd1, false);
            store_ireg_to_ir1(t_opnd, opnd0, false);

            ra_free_temp(t_opnd);
        }
    } else {
        /*
         * If a memory operand is referenced, the processor’s locking protocol
         * is automatically implemented for the duration of the exchange
         * operation, regardless of the presence or absence of the LOCK prefix
         * or of the value of the IOPL.
         */
        /* IR1_OPND *reg_ir1 = NULL; */
        IR2_OPND src_mem = ra_alloc_itemp();
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND imm_opnd = ra_alloc_itemp();
        IR2_OPND mem_opnd_cpy = ra_alloc_itemp();
        IR2_OPND src_mem_cpy = ra_alloc_itemp();
        IR2_OPND sc_opnd_cpy = ra_alloc_itemp();
        IR2_OPND tmp_opnd = ra_alloc_itemp();
        IR2_OPND mask_opnd = ra_alloc_itemp();
        IR2_OPND src_gpr;
        IR2_OPND mem_opnd;

        /* 1. load REG opnand and MEM opnand */
        if (ir1_opnd_is_mem(opnd0)) {
            if (ir1_opnd_size(opnd0) == 16) {
                lsassertm(0, "Invalid operand size (%d) in translate_xchg.\n",
                          ir1_opnd_size(opnd0));
            }

            /* if (ir1_opnd_size(ir1_get_opnd(pir1, 1)) == 32)
             *     reg_ir1 = &eax_ir1_opnd;
             * else if (ir1_opnd_base_reg_bits_start(ir1_get_opnd(pir1, 1)))
             *     reg_ir1 = &ah_ir1_opnd;
             * else
             *     reg_ir1 = &al_ir1_opnd;
             */

            mem_opnd = mem_ir1_to_ir2_opnd(opnd0, false);
            src_gpr = load_ireg_from_ir1(opnd1,
                                        UNKNOWN_EXTENSION, false);
        } else {
            if (ir1_opnd_size(opnd1) == 16) {
                lsassertm(0, "Invalid operand size (%d) in translate_xchg.\n",
                          ir1_opnd_size(opnd0));
            }

            /* if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
             *     reg_ir1 = &eax_ir1_opnd;
             * else if (ir1_opnd_base_reg_bits_start(ir1_get_opnd(pir1, 0)))
             *     reg_ir1 = &ah_ir1_opnd;
             * else
             *     reg_ir1 = &al_ir1_opnd;
             */

            mem_opnd = mem_ir1_to_ir2_opnd(opnd1, false);
            src_gpr = load_ireg_from_ir1(opnd0,
                                        UNKNOWN_EXTENSION, false);
        }

        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        /* 2. add the offset to the base memory adderss */
        la_append_ir2_opnd3_em(LISA_OR, mem_opnd_cpy, zero_ir2_opnd, mem_opnd);
        load_ireg_from_imm32(imm_opnd, imm, SIGN_EXTENSION);
        la_append_ir2_opnd3_em(LISA_ADD_D, mem_opnd_cpy, mem_opnd_cpy, imm_opnd);

        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        la_append_ir2_opnd1(LISA_LABEL, label_ll);

        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, src_gpr);

        /* 3. (original-address % 4) => the target-byte's offset in the 4-bytes loaded*/
        load_ireg_from_imm32(imm_opnd, 0x4, ZERO_EXTENSION);
        la_append_ir2_opnd3(LISA_MOD_WU, tmp_opnd, mem_opnd_cpy, imm_opnd);
        la_append_ir2_opnd3_em(LISA_SUB_D, mem_opnd_cpy, mem_opnd_cpy, tmp_opnd);

        /* 4. now ll the total 4 bytes to reg and make a copy of src_mem*/
        la_append_ir2_opnd2i_em(LISA_LL_W, src_mem, mem_opnd_cpy, 0);
        la_append_ir2_opnd3_em(LISA_OR, src_mem_cpy, zero_ir2_opnd, src_mem);

        /* 5. rebuild sc_opnd and src_mem */
        if (ir1_opnd_size(opnd0) == 16) {
            lsassertm(0, "Invalid operand size (%d) in translate_xchg.\n",
                        ir1_opnd_size(opnd0));
        } else if (ir1_opnd_size(opnd0) == 8) {
            /* rebuild sc_opnd */
            la_append_ir2_opnd3_em(LISA_OR, sc_opnd_cpy, zero_ir2_opnd, sc_opnd);
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, sc_opnd, zero_ir2_opnd, 31, 8);
            la_append_ir2_opnd2i_em(LISA_SLLI_D, tmp_opnd, tmp_opnd, 3);
            la_append_ir2_opnd3_em(LISA_SLL_D, sc_opnd, sc_opnd, tmp_opnd);

            load_ireg_from_imm32(mask_opnd, 0xff, ZERO_EXTENSION);
            la_append_ir2_opnd3_em(LISA_SLL_D, mask_opnd, mask_opnd, tmp_opnd);
            la_append_ir2_opnd3_em(LISA_NOR, mask_opnd, zero_ir2_opnd, mask_opnd);
            la_append_ir2_opnd3_em(LISA_AND, src_mem_cpy, src_mem_cpy, mask_opnd);

            la_append_ir2_opnd3_em(LISA_OR, sc_opnd, sc_opnd, src_mem_cpy);

            /* rebuild src_mem */
            la_append_ir2_opnd3_em(LISA_SRL_D, src_mem, src_mem, tmp_opnd);
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, src_mem, zero_ir2_opnd, 31, 8);
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, sc_opnd_cpy, zero_ir2_opnd, 7, 0);
            la_append_ir2_opnd3_em(LISA_OR, src_mem, src_mem, sc_opnd_cpy);
        } else {
            /* 32bit go through without any processing */
        }

        /* 6. do xchg operation */
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd_cpy, 0);
        /* 6.1 if the first LL/SC failed, the we need to go back to restart,
         * and remember to make mem_opnd_cpy back to the status of the begaining of the loop.
         */
        la_append_ir2_opnd2i_em(LISA_SRLI_D, tmp_opnd, tmp_opnd, 3);
        la_append_ir2_opnd3_em(LISA_ADD_D, mem_opnd_cpy, mem_opnd_cpy, tmp_opnd);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);
        /* FIXME: we should have used store_ireg_to_ir1 to store src_mem back to specified reg_ir1,
         * BUT it causes core dump when running WPS. So I use LISA_OR instead, which is a wrong way,
         * becaus it only recognises AL reg but not AH reg, I think the problem is in store_ireg_to_ir1.
         */
        //store_ireg_to_ir1(src_mem, reg_ir1, false);
        la_append_ir2_opnd3_em(LISA_OR, src_gpr, zero_ir2_opnd, src_mem);

        ra_free_temp(src_mem);
        ra_free_temp(sc_opnd);
        ra_free_temp(imm_opnd);
        ra_free_temp(mem_opnd_cpy);
        ra_free_temp(src_mem_cpy);
        ra_free_temp(sc_opnd_cpy);
        ra_free_temp(mask_opnd);
        ra_free_temp(tmp_opnd);
    }

    return true;
}

bool translate_cmpxchg(IR1_INST *pir1)
{
    IR1_OPND *reg_ir1 = NULL;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        reg_ir1 = &eax_ir1_opnd;
    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8)
        reg_ir1 = &al_ir1_opnd;
    else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        reg_ir1 = &ax_ir1_opnd;
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND eax_opnd = load_ireg_from_ir1(reg_ir1, SIGN_EXTENSION, false);
    IR2_OPND t_dest_opnd = ra_alloc_itemp();

    IR2_OPND label_unequal = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);

    /* such as lock cmpxchg %ecx,(%edx) */
    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND eax_opnd_copy = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        la_append_ir2_opnd3_em(LISA_OR, eax_opnd_copy, zero_ir2_opnd, eax_opnd);
        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, src_opnd_1);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BNE, src_opnd_0, eax_opnd, label_unequal);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        /* equal */
        // set zf
        la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
        la_append_ir2_opnd1(LISA_B, label_exit);

        /* unequal */
        la_append_ir2_opnd1(LISA_LABEL, label_unequal);
        la_append_ir2_opnd0(LISA_DBAR);
        // clear zf
        la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
        store_ireg_to_ir1(src_opnd_0, reg_ir1, false);

        /* exit */
        la_append_ir2_opnd1(LISA_LABEL, label_exit);
        la_append_ir2_opnd3_em(LISA_SUB_D, t_dest_opnd, eax_opnd_copy, src_opnd_0);
        generate_eflag_calculation(t_dest_opnd, eax_opnd_copy, src_opnd_0,
                                    pir1, true);

        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
        ra_free_temp(eax_opnd_copy);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);

        la_append_ir2_opnd3_em(LISA_SUB_D, t_dest_opnd, eax_opnd, src_opnd_0);
        generate_eflag_calculation(t_dest_opnd, eax_opnd, src_opnd_0, pir1, true);
        la_append_ir2_opnd3(LISA_BNE, src_opnd_0, eax_opnd, label_unequal);

        /* equal */
        // set zf
        la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
        store_ireg_to_ir1(src_opnd_1, ir1_get_opnd(pir1, 0), false);
        la_append_ir2_opnd1(LISA_B, label_exit);

        /* unequal */
        la_append_ir2_opnd1(LISA_LABEL, label_unequal);
        // clear zf
        la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
        store_ireg_to_ir1(src_opnd_0, reg_ir1, false);
        la_append_ir2_opnd1(LISA_LABEL, label_exit);
    }

    ra_free_temp(t_dest_opnd);
    return true;
}
/*
 * FIXME: This implementation is NOT thread safe.
 */
bool translate_cmpxchg8b(IR1_INST *pir1)
{
    IR1_OPND *reg_eax = NULL, *reg_edx = NULL, *reg_ebx= NULL, *reg_ecx= NULL;
    lsassert(ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64);

    IR2_OPND mem_opnd =
        mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
    int imm = ir2_opnd_imm(&mem_opnd);
    mem_opnd._type = IR2_OPND_IREG;

    reg_eax = &eax_ir1_opnd;
    reg_edx = &edx_ir1_opnd;

    reg_ecx = &ecx_ir1_opnd;
    reg_ebx = &ebx_ir1_opnd;
     /*
     * There is only one parameter from IR1.
     * if EDX:EAX == m64
     *      set ZF and m64 = ECX:EBX 
     *  else 
     *      clear ZF and EDX:EAX = m64
      */
    IR2_OPND t_dest_opnd = ra_alloc_itemp();
    IR2_OPND edx_eax_opnd = ra_alloc_itemp();
    IR2_OPND ecx_ebx_opnd = ra_alloc_itemp();
    IR2_OPND sc_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_0 = ra_alloc_itemp();
    IR2_OPND label_unequal = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);
    IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);

     /*
     * Got EAX/EDX from IR1
      */
    IR2_OPND eax_opnd = load_ireg_from_ir1(reg_eax, SIGN_EXTENSION, false);
    IR2_OPND edx_opnd = load_ireg_from_ir1(reg_edx, SIGN_EXTENSION, false);
    IR2_OPND ecx_opnd = load_ireg_from_ir1(reg_ecx, SIGN_EXTENSION, false);
    IR2_OPND ebx_opnd = load_ireg_from_ir1(reg_ebx, SIGN_EXTENSION, false);

     /*
     * Merge EDX:EAX as a 64bit data
      */
    la_append_ir2_opnd3_em(LISA_OR, edx_eax_opnd, edx_opnd, zero_ir2_opnd);
    la_append_ir2_opnd2i_em(LISA_SLLI_D, edx_eax_opnd, edx_eax_opnd, 32);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, edx_eax_opnd, eax_opnd, 31, 0);

    la_append_ir2_opnd3_em(LISA_OR, ecx_ebx_opnd, ecx_opnd, zero_ir2_opnd);
    la_append_ir2_opnd2i_em(LISA_SLLI_D, ecx_ebx_opnd, ecx_ebx_opnd, 32);
    la_append_ir2_opnd2ii(LISA_BSTRINS_D, ecx_ebx_opnd, ebx_opnd, 31, 0);

     /*
     * Got m64 data from mem
      */
    la_append_ir2_opnd1(LISA_LABEL, label_ll);
    la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, ecx_ebx_opnd);
    la_append_ir2_opnd2i_em(LISA_LL_D, src_opnd_0, mem_opnd, imm);
    la_append_ir2_opnd3(LISA_BNE, src_opnd_0, edx_eax_opnd, label_unequal);
    la_append_ir2_opnd2i(LISA_SC_D, sc_opnd, mem_opnd, imm);
    la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);
    /* equal */
    // set zf
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    store_ireg_to_ir1(ecx_ebx_opnd, ir1_get_opnd(pir1, 0), false);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* unequal */
    la_append_ir2_opnd1(LISA_LABEL, label_unequal);
    la_append_ir2_opnd0(LISA_DBAR);
    // clear zf
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x8);
    store_ireg_to_ir1(src_opnd_0, reg_eax, false);
    la_append_ir2_opnd2i_em(LISA_SRLI_D, src_opnd_0, src_opnd_0, 32);
    store_ireg_to_ir1(src_opnd_0, reg_edx, false);

    la_append_ir2_opnd1(LISA_LABEL, label_exit);
    la_append_ir2_opnd3_em(LISA_SUB_D, t_dest_opnd, edx_eax_opnd, src_opnd_0);
    generate_eflag_calculation(t_dest_opnd, edx_eax_opnd, src_opnd_0, pir1, true);

    return true;
}

bool translate_movq(IR1_INST *pir1)
{
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        la_append_ir2_opnd2i(LISA_VCLRSTRI_V,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          7);
        return true;
    } else if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        store_freg_to_ir1(ra_alloc_xmm(ir1_opnd_base_reg_num(src)), dest,
                          false, false);
        return true;
    } else if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        la_append_ir2_opnd2i(LISA_VCLRSTRI_V,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)),
                          ra_alloc_xmm(ir1_opnd_base_reg_num(src)), 7);
        return true;
    }
    if (ir1_opnd_is_xmm(dest) || ir1_opnd_is_xmm(src)){
        lsassert(0);
    }
    if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))) { /* dest xmm */
        IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, false);
        load_freg_from_ir1_2(dest_lo, ir1_get_opnd(pir1, 0) + 1, false, false);
        store_ireg_to_ir1(zero_ir2_opnd, ir1_get_opnd(pir1, 0), true);
    } else if (ir1_opnd_is_mmx(ir1_get_opnd(pir1, 0))) { /* dest mmx */
        IR2_OPND dest_opnd = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, false);
        load_freg_from_ir1_2(dest_opnd, ir1_get_opnd(pir1, 0) + 1, false, false);
    } else { /* dest mem */
        IR2_OPND source_opnd =
            load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, false);
        store_freg_to_ir1(source_opnd, ir1_get_opnd(pir1, 0), false, false);
    }
    return true;
}

bool translate_movd(IR1_INST *pir1)
{
    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src = ir1_get_opnd(pir1, 1);
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = load_freg_from_ir1_1(src, false, false);
        la_append_ir2_opnd2i(LISA_VCLRSTRI_V,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          3);
        return true;
    }
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_gpr(src)) {
        IR2_OPND temp = ra_alloc_ftemp();
        la_append_ir2_opnd2_em(LISA_MOVGR2FR_W, temp,
                         ra_alloc_gpr(ir1_opnd_base_reg_num(src)));
        la_append_ir2_opnd2i(LISA_VCLRSTRI_V,
                          ra_alloc_xmm(ir1_opnd_base_reg_num(dest)), temp,
                          3);
        return true;
    }
    if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        store_freg_to_ir1(ra_alloc_xmm(ir1_opnd_base_reg_num(src)), dest,
                          false, false);
        return true;
    }
    if (ir1_opnd_is_gpr(dest) && ir1_opnd_is_xmm(src)) {
        la_append_ir2_opnd2_em(LISA_MOVFR2GR_S,
                         ra_alloc_gpr(ir1_opnd_base_reg_num(dest)),
                         ra_alloc_xmm(ir1_opnd_base_reg_num(src)));
        return true;
    }
    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        lsassert(0);
        // no movd xmm,xmm
    }

    if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0)) || ir1_opnd_is_mem(ir1_get_opnd(pir1, 0))) {
        lsassert(ir1_opnd_is_mmx(ir1_get_opnd(pir1, 0) + 1) ||
                 ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0) + 1));
        IR2_OPND src =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, UNKNOWN_EXTENSION, false);
        store_ireg_to_ir1(src, ir1_get_opnd(pir1, 0), false);
    } else if (ir1_opnd_is_gpr(ir1_get_opnd(pir1, 0) + 1) ||
               ir1_opnd_is_mem(ir1_get_opnd(pir1, 0) + 1)) {
        lsassert(ir1_opnd_is_mmx(ir1_get_opnd(pir1, 0)) || ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
        IR2_OPND src =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, ZERO_EXTENSION, false);
        store_ireg_to_ir1(src, ir1_get_opnd(pir1, 0), false);
        if (ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)))
            store_ireg_to_ir1(zero_ir2_opnd, ir1_get_opnd(pir1, 0), true);
    }
    return true;
}
bool translate_pusha(IR1_INST *pir1) {
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                              -lsenv->tr_data->curr_esp_need_decrease);
    lsenv->tr_data->curr_esp_need_decrease = 0;

    int esp_decrement = 4;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        esp_decrement = 2;

    IR1_OPND mem_ir1_opnd;
    for (int i = 0; i < 8; i++) {
        ir1_opnd_build_mem(&mem_ir1_opnd, esp_decrement << 3, X86_REG_ESP,
                           -esp_decrement * (i + 1));
        IR2_OPND src_opnd = ra_alloc_gpr(i);
        store_ireg_to_ir1(src_opnd, &mem_ir1_opnd, false);
    }
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, -esp_decrement * 8);

    return true;
}
bool translate_popa(IR1_INST *pir1) {
    IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd,
                              -lsenv->tr_data->curr_esp_need_decrease);
    lsenv->tr_data->curr_esp_need_decrease = 0;

    int esp_increment = 4;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16)
        esp_increment = 2;

    lsassert(esp_increment == 4);//only support 32,

    IR1_OPND mem_ir1_opnd;
    IR2_OPND dst_opnd;

    for (int i = 7; i >= 0; i--) {
        if (i == esp_index) {
            // skip esp
            continue;
        }
        dst_opnd = ra_alloc_gpr(i);
        ir1_opnd_build_mem(&mem_ir1_opnd, esp_increment << 3, X86_REG_ESP,
                           esp_increment * (7 - i));
        load_ireg_from_ir1_2(dst_opnd, &mem_ir1_opnd, UNKNOWN_EXTENSION, false);
    }
    la_append_ir2_opnd2i_em(LISA_ADDI_ADDRX, esp_opnd, esp_opnd, esp_increment * 8);

    return true;
}
