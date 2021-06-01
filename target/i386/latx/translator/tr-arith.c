#include "common.h"
#include "env.h"
#include "reg-alloc.h"
#include "translate.h"

bool translate_add(IR1_INST *pir1)
{
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();
        
    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_opnd_0, src_opnd_1);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_opnd_0, src_opnd_1);
        fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);

    return true;
}

bool translate_adc(IR1_INST *pir1)
{
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        load_ireg_from_cf_opnd(&dest_opnd);
        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, dest_opnd, src_opnd_0);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, dest_opnd, src_opnd_1);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        load_ireg_from_cf_opnd(&dest_opnd);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, dest_opnd, src_opnd_0);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, dest_opnd, src_opnd_1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_inc(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, dest_opnd, src_opnd_0, 1);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, t_imm1, pir1, true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, dest_opnd, src_opnd_0, 1);
        IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, t_imm1, pir1, true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_dec(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, dest_opnd, src_opnd_0, -1);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, t_imm1, pir1, true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        la_append_ir2_opnd2i_em(LISA_ADDI_W, dest_opnd, src_opnd_0, -1);
        IR2_OPND t_imm1 = ir2_opnd_new(IR2_OPND_IMM, 1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, t_imm1, pir1, true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_sub(IR1_INST *pir1)
{
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, src_opnd_1);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, src_opnd_1);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_sbb(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);
    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        load_ireg_from_cf_opnd(&dest_opnd);
        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        if (ir2_opnd_cmp(&src_opnd_0, &src_opnd_1)) {
            la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, dest_opnd);
        } else {
            la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, dest_opnd);
            la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, dest_opnd, src_opnd_1);
        }
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    }
    else{
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        load_ireg_from_cf_opnd(&dest_opnd);
        if (ir2_opnd_cmp(&src_opnd_0, &src_opnd_1)) {
            la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, dest_opnd);
        } else {
            la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, dest_opnd);
            la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, dest_opnd, src_opnd_1);
        }
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_neg(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_is_prefix_lock(pir1)) {
        if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 32) {
            lsassertm(0, "Invalid operand size (%d) in %s.\n",
                      ir1_opnd_size(ir1_get_opnd(pir1, 0)), __func__);
        }
        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND src_opnd_0 = ra_alloc_itemp();
        IR2_OPND mem_opnd =
            mem_ir1_to_ir2_opnd(ir1_get_opnd(pir1, 0), false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        la_append_ir2_opnd1(LISA_LABEL, label_ll);
        la_append_ir2_opnd2i_em(LISA_LL_W, src_opnd_0, mem_opnd, imm);
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, src_opnd_0);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd, imm);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        generate_eflag_calculation(dest_opnd, zero_ir2_opnd, src_opnd_0, pir1,
                                   true);
        ra_free_temp(sc_opnd);
        ra_free_temp(src_opnd_0);
    } else {
        IR2_OPND src_opnd_0 =
            load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, zero_ir2_opnd, src_opnd_0);
        generate_eflag_calculation(dest_opnd, zero_ir2_opnd, src_opnd_0, pir1,
                                   true);
        store_ireg_to_ir1(dest_opnd, ir1_get_opnd(pir1, 0), false);
    }

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_cmp(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0) + 1, SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    fp_save_src_opnd(pir1, src_opnd_0, src_opnd_1);
    la_append_ir2_opnd3_em(LISA_SUB_W, dest_opnd, src_opnd_0, src_opnd_1);

    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    ra_free_temp(dest_opnd);

    return true;
}

bool translate_mul(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_1;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        src_opnd_1 = load_ireg_from_ir1(&al_ir1_opnd, ZERO_EXTENSION, false);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        ir2_opnd_set_em(&src_opnd_0,ZERO_EXTENSION,16);
        ir2_opnd_set_em(&src_opnd_1,ZERO_EXTENSION,16);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest_opnd, src_opnd_0, src_opnd_1);
        store_ireg_to_ir1(dest_opnd, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        src_opnd_1 = load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest_opnd, src_opnd_0, src_opnd_1);
        store_ireg_to_ir1(dest_opnd, &ax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_W, dest_opnd, dest_opnd, 16);
        store_ireg_to_ir1(dest_opnd, &dx_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        src_opnd_1 = load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
        generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);
        la_append_ir2_opnd3_em(LISA_MUL_D, dest_opnd, src_opnd_0, src_opnd_1);
        store_ireg_to_ir1(dest_opnd, &eax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, dest_opnd, dest_opnd, 32);
        store_ireg_to_ir1(dest_opnd, &edx_ir1_opnd, false);
    } else {
        lsassertm(0, "64-bit translate_imul_1_opnd is unimplemented.\n");
        /* NOT_IMPLEMENTED; */
    }


    ra_free_temp(dest_opnd);
    return true;
}

static bool translate_imul_1_opnd(IR1_INST *pir1)
{
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND dest = ra_alloc_itemp();
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&al_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest, src_opnd_1, src_opnd_0);
        /* calculate eflag */
        generate_eflag_calculation(dest, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_MUL_W, dest, src_opnd_1, src_opnd_0);
        /* calculate eflag */
        generate_eflag_calculation(dest, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest, &ax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, dest, dest, 16);
        store_ireg_to_ir1(dest, &dx_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&eax_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_MUL_D, dest, src_opnd_1, src_opnd_0);
        /* calculate eflag */
        generate_eflag_calculation(dest, src_opnd_0, src_opnd_1, pir1, true);
        store_ireg_to_ir1(dest, &eax_ir1_opnd, false);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, dest, dest, 32);
        store_ireg_to_ir1(dest, &edx_ir1_opnd, false);
    } else {
        /* IR2_OPND dest_opnd_0 = */
        /* load_ireg_from_ir1(pir1->get_dest_opnd_implicit(0), SIGN_EXTENSION,
         */
        /* false); IR2_OPND dest_opnd_1 = */
        /* load_ireg_from_ir1(pir1->get_dest_opnd_implicit(1), SIGN_EXTENSION,
         */
        /* false); append_ir2_opnd2(mips_dmult, src_opnd_1, src_opnd_0); */
        /* append_ir2_opnd1(mips_mflo, dest_opnd_0); */
        /* append_ir2_opnd1(mips_mfhi, dest_opnd_1); */
        lsassertm(0, "64-bit translate_imul_1_opnd is unimplemented.\n");
    }
    ra_free_temp(dest);
    return true;
}

bool translate_imul(IR1_INST *pir1)
{
    if (ir1_opnd_num(pir1) == 1)
        return translate_imul_1_opnd(pir1);

    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_src_opnd(pir1, 0), SIGN_EXTENSION, false);
    IR2_OPND src_opnd_1 =
        load_ireg_from_ir1(ir1_get_src_opnd(pir1, 1), SIGN_EXTENSION, false);

    IR2_OPND dest_opnd = ra_alloc_itemp();
    lsassertm(ir1_opnd_size(ir1_get_opnd(pir1, 0)) != 64,
              "64-bit translate_imul is unimplemented.\n");
    IR2_OPCODE opcode;
    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32)
        opcode = LISA_MUL_D;
    else
        opcode = LISA_MUL_W;
    la_append_ir2_opnd3_em(opcode, dest_opnd, src_opnd_1, src_opnd_0);
    generate_eflag_calculation(dest_opnd, src_opnd_0, src_opnd_1, pir1, true);

    if (ir1_opnd_num(pir1) == 3)
        store_ireg_to_ir1(dest_opnd, ir1_get_dest_opnd(pir1, 0), false);
    else
        store_ireg_to_ir1(dest_opnd, ir1_get_src_opnd(pir1, 0), false);
    ra_free_temp(dest_opnd);
    return true;
}

bool translate_div(IR1_INST *pir1)
{
    IR2_OPND small_opnd =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), ZERO_EXTENSION, false);
    IR2_OPND result = ra_alloc_itemp();
    IR2_OPND result_remainder = ra_alloc_itemp();

    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        IR2_OPND large_opnd =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);

        la_append_ir2_opnd3(LISA_DIV_DU, result, large_opnd, small_opnd);
	la_append_ir2_opnd3(LISA_BNE, small_opnd, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);

        la_append_ir2_opnd3(LISA_MOD_DU, result_remainder, large_opnd, small_opnd);
        /*  result larger than uint8 would raise an exception */
        ir2_opnd_set_em(&result, ZERO_EXTENSION, 8); 
        ir2_opnd_set_em(&result_remainder, ZERO_EXTENSION, 8); 

        /* set AL and AH at the same time */
        la_append_ir2_opnd2i_em(LISA_SLLI_D, result_remainder, result_remainder, 8);
        la_append_ir2_opnd3_em(LISA_OR, result, result, result_remainder);
        store_ireg_to_ir1(result, &ax_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR2_OPND large_opnd =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND large_opnd_high_bits =
            load_ireg_from_ir1(&dx_ir1_opnd, ZERO_EXTENSION, false);
        la_append_ir2_opnd2i_em(LISA_SLLI_D, large_opnd_high_bits, large_opnd_high_bits, 16);
        la_append_ir2_opnd3_em(LISA_OR, large_opnd, large_opnd_high_bits, large_opnd);

        la_append_ir2_opnd3(LISA_DIV_DU, result, large_opnd, small_opnd);
	la_append_ir2_opnd3(LISA_BNE, small_opnd, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);

        la_append_ir2_opnd3(LISA_MOD_DU, result_remainder, large_opnd, small_opnd);
        /*  result larger than uint16 would raise an exception */
        ir2_opnd_set_em(&result, ZERO_EXTENSION, 16);
        ir2_opnd_set_em(&result_remainder, ZERO_EXTENSION, 16);

        store_ireg_to_ir1(result, &ax_ir1_opnd, false);
        store_ireg_to_ir1(result_remainder, &dx_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND large_opnd =
            load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND large_opnd_high_bits =
            load_ireg_from_ir1(&edx_ir1_opnd, UNKNOWN_EXTENSION, false);
        la_append_ir2_opnd2i_em(LISA_SLLI_D, large_opnd_high_bits,
                          large_opnd_high_bits, 32);
        la_append_ir2_opnd3_em(LISA_OR, large_opnd, large_opnd_high_bits, large_opnd);

        IR2_OPND ir2_eax = ra_alloc_gpr(ir1_opnd_base_reg_num(&eax_ir1_opnd));
        IR2_OPND ir2_edx = ra_alloc_gpr(ir1_opnd_base_reg_num(&edx_ir1_opnd));

        /* load_ireg_from_ir1() may return the old register */
        if (ir2_eax._reg_num == large_opnd._reg_num) {
            IR2_OPND temp_large = ra_alloc_itemp();
            la_append_ir2_opnd3_em(LISA_OR, temp_large, large_opnd, zero_ir2_opnd);
            large_opnd = temp_large;
        }

        la_append_ir2_opnd3(LISA_DIV_DU, ir2_eax, large_opnd, small_opnd);
	la_append_ir2_opnd3(LISA_BNE, small_opnd, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        la_append_ir2_opnd3(LISA_MOD_DU, ir2_edx, large_opnd, small_opnd);
        ir2_opnd_set_em(&ir2_eax, ZERO_EXTENSION, 32);
        ir2_opnd_set_em(&ir2_edx, ZERO_EXTENSION, 32);
    } else
        lsassertm(0, "64-bit translate_div is unimplemented.\n");

    ra_free_temp(result);
    ra_free_temp(result_remainder);
    return true;
}

bool translate_idiv(IR1_INST *pir1)
{
    IR2_OPND dest_opnd = ra_alloc_itemp();
    IR2_OPND src_opnd_0 =
        load_ireg_from_ir1(ir1_get_opnd(pir1, 0), SIGN_EXTENSION, false);

    IR2_OPND label_z = ir2_opnd_new_type(IR2_OPND_LABEL);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 8) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, SIGN_EXTENSION, false);
        la_append_ir2_opnd3(LISA_MOD_D, dest_opnd, src_opnd_1, src_opnd_0);
	la_append_ir2_opnd3(LISA_BNE, src_opnd_0, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        la_append_ir2_opnd3(LISA_DIV_D, src_opnd_1, src_opnd_1, src_opnd_0);
        store_ireg_to_ir1(dest_opnd, &ah_ir1_opnd, false);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&ax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND src_opnd_2 =
            load_ireg_from_ir1(&dx_ir1_opnd, UNKNOWN_EXTENSION, false);
        IR2_OPND temp_src = ra_alloc_itemp();
        IR2_OPND temp1_opnd = ra_alloc_itemp();

        la_append_ir2_opnd2i_em(LISA_SLLI_W, temp_src, src_opnd_2, 16);
        la_append_ir2_opnd3(LISA_OR, temp_src, temp_src, src_opnd_1);
        la_append_ir2_opnd3(LISA_DIV_D, temp1_opnd, temp_src, src_opnd_0);
	la_append_ir2_opnd3(LISA_BNE, src_opnd_0, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        store_ireg_to_ir1(temp1_opnd, &ax_ir1_opnd, false);
        la_append_ir2_opnd3(LISA_MOD_D, temp1_opnd, temp_src, src_opnd_0);
        store_ireg_to_ir1(temp1_opnd, &dx_ir1_opnd, false);

        ra_free_temp(temp_src);
        ra_free_temp(temp1_opnd);
    } else if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 32) {
        IR2_OPND src_opnd_1 =
            load_ireg_from_ir1(&eax_ir1_opnd, ZERO_EXTENSION, false);
        IR2_OPND src_opnd_2 =
            load_ireg_from_ir1(&edx_ir1_opnd, UNKNOWN_EXTENSION, false);
        IR2_OPND temp_src = ra_alloc_itemp();
        IR2_OPND temp1_opnd = ra_alloc_itemp();

        la_append_ir2_opnd2i_em(LISA_SLLI_D, temp_src, src_opnd_2, 32);
        la_append_ir2_opnd3_em(LISA_OR, temp_src, temp_src, src_opnd_1);
        la_append_ir2_opnd3(LISA_DIV_D, temp1_opnd, temp_src, src_opnd_0);
	la_append_ir2_opnd3(LISA_BNE, src_opnd_0, zero_ir2_opnd, label_z);
	la_append_ir2_opndi(LISA_BREAK, 0x7);
	la_append_ir2_opnd1(LISA_LABEL, label_z);
        store_ireg_to_ir1(temp1_opnd, &eax_ir1_opnd, false);
        la_append_ir2_opnd3(LISA_MOD_D, temp1_opnd, temp_src, src_opnd_0);
        store_ireg_to_ir1(temp1_opnd, &edx_ir1_opnd, false);

        ra_free_temp(temp_src);
        ra_free_temp(temp1_opnd);
    } else
        lsassertm(0, "64-bit translate_idiv is unimplemented.\n");

    ra_free_temp(dest_opnd);
    return true;
}

bool translate_xadd(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR2_OPND src_reg_opnd =
        load_ireg_from_ir1(opnd1, SIGN_EXTENSION, false);
    IR2_OPND dest_opnd = ra_alloc_itemp();

    if (ir1_is_prefix_lock(pir1)) {
        /* opnd0 is mem, opnd1 is reg */
        IR1_OPND *reg_ir1 = NULL;
        if (ir1_opnd_size(opnd0) == 16) {
            lsassertm(0, "Invalid operand size (%d) in translate_xadd.\n",
                        ir1_opnd_size(opnd0));
        }

        /* make sure which reg is in the pir1, al/ah/ax ? */
        if (ir1_opnd_size(ir1_get_opnd(pir1, 1)) == 32)
            reg_ir1 = &eax_ir1_opnd;
        else if (ir1_opnd_base_reg_bits_start(ir1_get_opnd(pir1, 1)))
            reg_ir1 = &ah_ir1_opnd;
        else
            reg_ir1 = &al_ir1_opnd;

        IR2_OPND label_ll = ir2_opnd_new_type(IR2_OPND_LABEL);
        /* mem addr */
        IR2_OPND tmp_opnd = ra_alloc_itemp();
        IR2_OPND imm_opnd = ra_alloc_itemp();
        IR2_OPND sc_opnd = ra_alloc_itemp();
        IR2_OPND mask_opnd = ra_alloc_itemp();
        IR2_OPND src_mem_opnd = ra_alloc_itemp();
        IR2_OPND src_mem_opnd_cpy = ra_alloc_itemp();
        IR2_OPND src_mem_opnd_restore = ra_alloc_itemp();
        IR2_OPND mem_opnd_cpy = ra_alloc_itemp();
        IR2_OPND dest_opnd_cpy = ra_alloc_itemp();

        IR2_OPND mem_opnd = mem_ir1_to_ir2_opnd(opnd0, false);
        int imm = ir2_opnd_imm(&mem_opnd);
        mem_opnd._type = IR2_OPND_IREG;

        /* 1. make a copy of mem_opnd before we do anything,
         *    because next inst may use it in the same tb */
        la_append_ir2_opnd3_em(LISA_OR, mem_opnd_cpy, zero_ir2_opnd, mem_opnd);

        /* 2. add the offset to the base memory adderss */
        load_ireg_from_imm32(imm_opnd, imm, SIGN_EXTENSION);
        la_append_ir2_opnd3_em(LISA_ADD_D, mem_opnd_cpy, mem_opnd_cpy, imm_opnd);

        la_append_ir2_opnd1(LISA_LABEL, label_ll);

        /* 3. (original-address % 4) => the target-byte's offset in the 4-bytes loaded*/
        load_ireg_from_imm32(imm_opnd, 0x4, ZERO_EXTENSION);
        la_append_ir2_opnd3(LISA_MOD_WU, tmp_opnd, mem_opnd_cpy, imm_opnd);
        la_append_ir2_opnd3_em(LISA_SUB_D, mem_opnd_cpy, mem_opnd_cpy, tmp_opnd);

        /* 4. now ll the total 4 bytes to reg and make a copy of src_mem_opnd */
        la_append_ir2_opnd2i_em(LISA_LL_W, src_mem_opnd, mem_opnd_cpy, 0);
        la_append_ir2_opnd3_em(LISA_OR, src_mem_opnd_restore, zero_ir2_opnd, src_mem_opnd);

        if (ir1_opnd_size(opnd0) == 8) {
            /* 5. rebuild src_mem_opnd */
            /* 5.1. make a copy of src_mem_opnd */
            la_append_ir2_opnd3_em(LISA_OR, src_mem_opnd_cpy, zero_ir2_opnd, src_mem_opnd);
            /* 5.2. extract the single byte in src_mem_opnd_cpy which is specified by tmp_opnd,
             *      and shift it right to the lowest 8 bit. */
            la_append_ir2_opnd2i_em(LISA_SLLI_D, tmp_opnd, tmp_opnd, 3);
            la_append_ir2_opnd3_em(LISA_SRL_D, src_mem_opnd_cpy, src_mem_opnd_cpy, tmp_opnd);
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, src_mem_opnd_cpy, zero_ir2_opnd, 31, 8);
            /* 5.3. clear the single byte in src_mem_opnd which is specified tmp_opnd */
            load_ireg_from_imm32(mask_opnd, 0xff, ZERO_EXTENSION);
            la_append_ir2_opnd3_em(LISA_SLL_D, mask_opnd, mask_opnd, tmp_opnd);
            la_append_ir2_opnd3_em(LISA_NOR, mask_opnd, zero_ir2_opnd, mask_opnd);
            la_append_ir2_opnd3_em(LISA_AND, src_mem_opnd, src_mem_opnd, mask_opnd);
            /* 5.4. do the ADD operation, so we get pre-target dest_opnd whose
             *      data is in the lowest 8-bit */
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, src_reg_opnd, zero_ir2_opnd, 31, 8);
            la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_mem_opnd_cpy, src_reg_opnd);
            /* this LISA_BSTRINS_D is to solve the problem of overflow, the dest should always
             * has 8 bits. */
            la_append_ir2_opnd2ii(LISA_BSTRINS_D, dest_opnd, zero_ir2_opnd, 31, 8);
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd_cpy, zero_ir2_opnd, dest_opnd);
            /* 5.5. shift it left to the coresponding positon specified by tmp_opnd */
            la_append_ir2_opnd3_em(LISA_SLL_D, dest_opnd, dest_opnd, tmp_opnd);
            /* 5.6. do the OR operation, finally we get the final-target dest_opnd */
            la_append_ir2_opnd3_em(LISA_OR, dest_opnd, dest_opnd, src_mem_opnd);
        } else if (ir1_opnd_size(opnd0) == 16) {
            lsassertm(0, "Invalid operand size (%d) in translate_xadd.\n",
                        ir1_opnd_size(opnd0));
        } else {
            /* 32-bit go through without any additional processing */
            la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_mem_opnd, src_reg_opnd);
        }

        //la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_mem_opnd, src_reg_opnd);
        la_append_ir2_opnd3_em(LISA_OR, sc_opnd, zero_ir2_opnd, dest_opnd);
        la_append_ir2_opnd2i(LISA_SC_W, sc_opnd, mem_opnd_cpy, 0);
        la_append_ir2_opnd2i_em(LISA_SRLI_D, tmp_opnd, tmp_opnd, 3);
        la_append_ir2_opnd3_em(LISA_ADD_D, mem_opnd_cpy, mem_opnd_cpy, tmp_opnd);
        la_append_ir2_opnd3(LISA_BEQ, sc_opnd, zero_ir2_opnd, label_ll);

        if (ir1_opnd_size(opnd0) == 8) {
            /* there is a little trouble in eflags stuff: we need to guarantee the two opnds which are
             * used to calculate eflags are the original opnds, or eflag will generated unexpected,
             * so we use src_mem_opnd_cpy but not src_mem_opnd here, you can check the generate_eflag_calculation
             * if you are confused. */
            generate_eflag_calculation(dest_opnd_cpy, src_reg_opnd, src_mem_opnd_cpy, pir1, true);
            store_ireg_to_ir1(src_mem_opnd_restore, reg_ir1, false);
        } else if (ir1_opnd_size(opnd0) == 16) {
            lsassertm(0, "Invalid operand size (%d) in translate_xadd.\n",
                        ir1_opnd_size(opnd0));
        } else {
            generate_eflag_calculation(dest_opnd, src_reg_opnd, src_mem_opnd, pir1, true);
            store_ireg_to_ir1(src_mem_opnd_restore, opnd1, false);
        }
        ra_free_temp(sc_opnd);
        ra_free_temp(src_mem_opnd);
    } else {
        IR2_OPND src_mem_opnd =
            load_ireg_from_ir1(opnd0, SIGN_EXTENSION, false);
        la_append_ir2_opnd3_em(LISA_ADD_W, dest_opnd, src_mem_opnd, src_reg_opnd);
        generate_eflag_calculation(dest_opnd, src_mem_opnd, src_reg_opnd, pir1, true);

        if (ir1_opnd_is_gpr(opnd0)) {
            store_ireg_to_ir1(src_mem_opnd, opnd1, false);
            store_ireg_to_ir1(dest_opnd, opnd0, false);
        } else {
            lsassert(ir1_opnd_is_mem(opnd0));
            store_ireg_to_ir1(dest_opnd, opnd0, false);
            store_ireg_to_ir1(src_mem_opnd, opnd1, false);
        }
    }

    ra_free_temp(dest_opnd);

    return true;
}
