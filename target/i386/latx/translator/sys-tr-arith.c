#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

bool latxs_translate_add(IR1_INST *pir1)
{
    /* TODO if (option_by_hand) return translate_add_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_ADD_W, &dest, &src0, &src1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_adc(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_adc_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_load_eflags_cf_to_ir2(&dest);
    latxs_append_ir2_opnd3(LISA_ADD_W, &dest, &dest, &src0);
    latxs_append_ir2_opnd3(LISA_ADD_W, &dest, &dest, &src1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_sub(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_sub_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &src0, &src1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_sbb(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_sbb_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_load_eflags_cf_to_ir2(&dest);
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &src0, &dest);
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &dest, &src1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_inc(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_inc_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &dest, &src0, 1);

    IR2_OPND imm1 = latxs_ir2_opnd_new(IR2_OPND_IMMH, 1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    return true;
}

bool latxs_translate_dec(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_dec_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ADDI_W, &dest, &src0, -1);

    IR2_OPND imm1 = latxs_ir2_opnd_new(IR2_OPND_IMMH, 1);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, &src0, &imm1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    return true;
}

bool latxs_translate_neg(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_neg_byhand(pir1); */

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, zero, &src0);

    if (ir1_opnd_is_mem(opnd0)) {
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
        /* calculate elfags after store */
        latxs_generate_eflag_calculation(&dest, zero, &src0, pir1, true);
    } else {
        /* Destination is GPR, no exception will generate */
        latxs_generate_eflag_calculation(&dest, zero, &src0, pir1, true);
        latxs_store_ir2_to_ir1(&dest, opnd0, false);
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    return true;
}

bool latxs_translate_cmp(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_cmp_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);
    latxs_load_ir1_to_ir2(&src1, opnd1, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_SUB_W, &dest, &src0, &src1);

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_mul(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_mul_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_Z, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &al_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd, false);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd, false);
        latxs_append_ir2_opnd2i(LISA_SRLI_W, &dest, &dest, 16);
        latxs_store_ir2_to_ir1(&dest, &dx_ir1_opnd, false);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_Z);
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
        latxs_store_ir2_to_ir1(&dest, &eax_ir1_opnd, false);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &dest, &dest, 32);
        latxs_store_ir2_to_ir1(&dest, &edx_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_mul is unimplemented.\n");
        break;
    }

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

static bool latxs_translate_imul_1_opnd(IR1_INST *pir1)
{
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();

    /* Destination is always GPR, no exception will generate */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &al_ir1_opnd, EXMode_S);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd, false);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_S);
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, &ax_ir1_opnd, false);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &dest, &dest, 16);
        latxs_store_ir2_to_ir1(&dest, &dx_ir1_opnd, false);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_S);
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
        latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);
        latxs_store_ir2_to_ir1(&dest, &eax_ir1_opnd, false);
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &dest, &dest, 32);
        latxs_store_ir2_to_ir1(&dest, &edx_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_imul_1_opnd is unimplemented.\n");
        break;
    }

    latxs_ra_free_temp(&dest);
    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);
    return true;
}

bool latxs_translate_imul(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_imul_byhand(pir1); */

    if (ir1_opnd_num(pir1) == 1) {
        return latxs_translate_imul_1_opnd(pir1);
    }

    IR1_OPND *srcopnd0 = ir1_get_src_opnd(pir1, 0);
    IR1_OPND *srcopnd1 = ir1_get_src_opnd(pir1, 1);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, srcopnd0, EXMode_S, false);
    latxs_load_ir1_to_ir2(&src1, srcopnd1, EXMode_S, false);

    IR2_OPND dest = latxs_ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(ir1_get_opnd(pir1, 0));

    lsassertm_illop(ir1_addr(pir1), opnd_size != 64,
            "64-bit translate_imul is unimplemented.\n");

    if (opnd_size == 32) {
        latxs_append_ir2_opnd3(LISA_MUL_D, &dest, &src1, &src0);
    } else {
        latxs_append_ir2_opnd3(LISA_MUL_W, &dest, &src1, &src0);
    }

    latxs_generate_eflag_calculation(&dest, &src0, &src1, pir1, true);

    latxs_ra_free_temp(&src0);
    latxs_ra_free_temp(&src1);

    /* Destination is always GPR, no exception will generate */
    if (ir1_opnd_num(pir1) == 3) {
        latxs_store_ir2_to_ir1(&dest, ir1_get_dest_opnd(pir1, 0), false);
    } else {
        latxs_store_ir2_to_ir1(&dest, ir1_get_src_opnd(pir1, 0), false);
    }

    latxs_ra_free_temp(&dest);
    return true;
}

void latxs_tr_gen_div_result_check(IR1_INST *pir1,
        IR2_OPND result, int size, int is_idiv)
{
    /*
     * u8   [0,     2^8 -1] = [0x0,         0xff        ]
     * u16  [0,     2^16-1] = [0x0,         0xffff      ]
     * u32  [0,     2^32-1] = [0x0,         0xffff_ffff ]
     * s8   [-2^7,  2^7 -1] = [0xffff_ff80, 0x7f        ]
     * s16  [-2^15, 2^15-1] = [0xffff_8000, 0x7fff      ]
     * s32  [-2^31, 2^31-1] = [0x8000_0000, 0x7fff_ffff ]
     */
    IR2_OPND label_no_excp = latxs_ir2_opnd_new_label();
    IR2_OPND tmp = latxs_ra_alloc_itemp();

    /* 1. chechk and branch to 'label_no_excp' */
    switch (size) {
    case 8:
        if (is_idiv) { /* signed 8-bit */
            latxs_append_ir2_opnd2_(lisa_mov8s, &tmp, &result);
        } else { /* unsigned 8-bit */
            latxs_append_ir2_opnd2_(lisa_mov8z, &tmp, &result);
        }
        break;
    case 16:
        if (is_idiv) { /* signed 16-bit */
            latxs_append_ir2_opnd2_(lisa_mov16s, &tmp, &result);
        } else { /* unsigned 16-bit */
            latxs_append_ir2_opnd2_(lisa_mov16z, &tmp, &result);
        }
        break;
    case 32:
        if (is_idiv) { /* signed 32-bit */
            latxs_append_ir2_opnd2_(lisa_mov32s, &tmp, &result);
        } else { /* unsigned 32-bit */
            latxs_append_ir2_opnd2_(lisa_mov32z, &tmp, &result);
        }
        break;
    default:
        /* should never reach here */
        lsassertm(0, "should never reach here [%s:%d].\n",
                __func__, __LINE__);
        break;
    }
    latxs_append_ir2_opnd3(LISA_BEQ, &result, &tmp, &label_no_excp);
    latxs_ra_free_temp(&tmp);

    /* 2. not branch: generate exception */
    latxs_tr_gen_excp_divz(pir1, 0);

    /* 3. branch: no exception */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_no_excp);
}

bool latxs_translate_div(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_div_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    IR2_OPND src2 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_Z, false);

    /* 1. if div zero, generate DIVZ exception */
    IR2_OPND label_not_zero = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &src0,
            &latxs_zero_ir2_opnd, &label_not_zero);
    /* 1.1 not branch : generate exception and never return */
    latxs_tr_gen_excp_divz(pir1, 0);
    /* 1.2 branch: go on finish div operation */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    IR2_OPND div_res = latxs_ra_alloc_itemp();
    IR2_OPND div_mod = latxs_ir2_opnd_new_inv();

    IR2_OPND src_tmp = latxs_ir2_opnd_new_inv();

    /* 2. do div operation */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd3(LISA_DIV_DU, &div_res, &src1, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src1, &src0);
        latxs_ra_free_temp(&src0);
        latxs_ra_free_temp(&src1);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 8, 0);
        /* 2.3 store the results into destination */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &div_mod, &div_mod, 8);
        latxs_append_ir2_opnd3(LISA_OR, &div_res, &div_res, &div_mod);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &dx_ir1_opnd, EXMode_Z);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 16);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 16, 0);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &dx_ir1_opnd, false);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &edx_ir1_opnd, EXMode_N);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 32);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_DU, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_DU, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 32, 0);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &edx_ir1_opnd, false);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &eax_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_div is unimplemented.\n");
        break;
    }

    latxs_ra_free_temp(&div_res);
    return true;
}

bool latxs_translate_idiv(IR1_INST *pir1)
{
    /* if (option_by_hand) return translate_idiv_byhand(pir1); */

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    IR2_OPND src0 = latxs_ra_alloc_itemp();
    IR2_OPND src1 = latxs_ra_alloc_itemp();
    IR2_OPND src2 = latxs_ra_alloc_itemp();

    latxs_load_ir1_to_ir2(&src0, opnd0, EXMode_S, false);

    /* 1. if div zero, generate DIVZ exception */
    IR2_OPND label_not_zero = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BNE, &src0,
            &latxs_zero_ir2_opnd, &label_not_zero);
    /* 1.1 not branch : generate exception and never return */
    latxs_tr_gen_excp_divz(pir1, 0);
    /* 1.2 branch: go on finish div operation */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_not_zero);

    IR2_OPND div_res = latxs_ra_alloc_itemp();
    IR2_OPND div_mod = latxs_ir2_opnd_new_inv();

    IR2_OPND src_tmp = latxs_ir2_opnd_new_inv();

    /* 2. do div operation */
    int opnd_size = ir1_opnd_size(opnd0);
    switch (opnd_size) {
    case 8:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_S);
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd3(LISA_DIV_D, &div_res, &src1, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_D, &div_mod, &src1, &src0);
        latxs_ra_free_temp(&src0);
        latxs_ra_free_temp(&src1);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 8, 1);
        /* 2.3 store the results into destination */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &div_mod, &div_mod, 8);
        latxs_append_ir2_opnd3(LISA_OR, &div_res, &div_res, &div_mod);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 16:
        latxs_load_ir1_gpr_to_ir2(&src1, &ax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &dx_ir1_opnd, EXMode_S);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 16);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_D, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 16, 1);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &dx_ir1_opnd, false);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &ax_ir1_opnd, false);
        break;
    case 32:
        latxs_load_ir1_gpr_to_ir2(&src1, &eax_ir1_opnd, EXMode_Z);
        latxs_load_ir1_gpr_to_ir2(&src2, &edx_ir1_opnd, EXMode_N);
        src_tmp = latxs_ra_alloc_itemp();
        div_mod = latxs_ra_alloc_itemp();

        /* 2.1 do div operation and get result */
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &src_tmp, &src2, 32);
        latxs_append_ir2_opnd3(LISA_OR, &src_tmp, &src_tmp, &src1);
        latxs_ra_free_temp(&src1);
        latxs_ra_free_temp(&src2);

        latxs_append_ir2_opnd3(LISA_DIV_D, &div_res, &src_tmp, &src0);
        latxs_append_ir2_opnd3(LISA_MOD_D, &div_mod, &src_tmp, &src0);
        latxs_ra_free_temp(&src_tmp);
        latxs_ra_free_temp(&src0);

        /* 2.2 check if result is valid and generate exception if not */
        latxs_tr_gen_div_result_check(pir1, div_res, 32, 1);
        /* 2.3 store the results into destination */
        latxs_store_ir2_to_ir1(&div_mod, &edx_ir1_opnd, false);
        latxs_ra_free_temp(&div_mod);
        latxs_store_ir2_to_ir1(&div_res, &eax_ir1_opnd, false);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "64-bit translate_idiv is unimplemented.\n");
        break;
    }

    latxs_ra_free_temp(&div_res);
    return true;
}
