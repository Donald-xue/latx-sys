#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>
#include <math.h>

void latxs_sys_farith_register_ir1(void)
{
    latxs_register_ir1(X86_INS_FADD);
    latxs_register_ir1(X86_INS_FADDP);
    latxs_register_ir1(X86_INS_FIADD);
    latxs_register_ir1(X86_INS_FSUB);
    latxs_register_ir1(X86_INS_FISUB);
    latxs_register_ir1(X86_INS_FISUBR);
    latxs_register_ir1(X86_INS_FSUBR);
    latxs_register_ir1(X86_INS_FSUBRP);
    latxs_register_ir1(X86_INS_FSUBP);
    latxs_register_ir1(X86_INS_FMUL);
    latxs_register_ir1(X86_INS_FIMUL);
    latxs_register_ir1(X86_INS_FMULP);
    latxs_register_ir1(X86_INS_FDIV);
    latxs_register_ir1(X86_INS_FDIVR);
    latxs_register_ir1(X86_INS_FIDIV);
    latxs_register_ir1(X86_INS_FIDIVR);
    latxs_register_ir1(X86_INS_FDIVRP);
    latxs_register_ir1(X86_INS_FDIVP);
    latxs_register_ir1(X86_INS_FNOP);
    latxs_register_ir1(X86_INS_FSQRT);
    latxs_register_ir1(X86_INS_FABS);
    latxs_register_ir1(X86_INS_FCHS);
    latxs_register_ir1(X86_INS_FSIN);
    latxs_register_ir1(X86_INS_FCOS);
    latxs_register_ir1(X86_INS_FPATAN);
    latxs_register_ir1(X86_INS_FPREM);
    latxs_register_ir1(X86_INS_FPREM1);
    latxs_register_ir1(X86_INS_FRNDINT);
    latxs_register_ir1(X86_INS_FSCALE);
    latxs_register_ir1(X86_INS_FXAM);
    latxs_register_ir1(X86_INS_F2XM1);
    latxs_register_ir1(X86_INS_FXTRACT);
    latxs_register_ir1(X86_INS_FYL2X);
    latxs_register_ir1(X86_INS_FYL2XP1);
    latxs_register_ir1(X86_INS_FSINCOS);
    latxs_register_ir1(X86_INS_FXCH);
    latxs_register_ir1(X86_INS_FTST);
    latxs_register_ir1(X86_INS_FPTAN);
}

bool latxs_translate_fadd(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);
    if (opnd_num == 1) {
        IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        latxs_append_ir2_opnd3(LISA_FADD_D, &st0_opnd,
                                            &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FADD_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }

    return true;
}

bool latxs_translate_faddp(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        IR2_OPND st1 = latxs_ra_alloc_st(1);
        latxs_append_ir2_opnd3(LISA_FADD_D, &st1, &st0, &st1);
    } else if (opnd_num == 1) {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        latxs_append_ir2_opnd3(LISA_FADD_D, &dest_opnd,
                                            &dest_opnd, &st0);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FADD_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }
    latxs_tr_fpu_pop();

    return true;
}

bool latxs_translate_fiadd(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);
    (void)opnd_num; /* to avoid compile warning */

    IR2_OPND st0    = latxs_ra_alloc_st(0);
    IR2_OPND t_freg = latxs_ra_alloc_ftemp();

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    latxs_load_freg_from_ir1_2(&t_freg, opnd0, false, false);

    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    latxs_append_ir2_opnd3(LISA_FADD_D, &st0, &t_freg, &st0);
    latxs_ra_free_temp(&t_freg);
    return true;
}

bool latxs_translate_fsub(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &st0_opnd,
                                            &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }

    return true;
}

bool latxs_translate_fisub(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);
    (void)opnd_num; /* to avoid compile warning */

    IR2_OPND st0    = latxs_ra_alloc_st(0);
    IR2_OPND t_freg = latxs_ra_alloc_ftemp();

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    latxs_load_freg_from_ir1_2(&t_freg, opnd0, false, false);

    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    latxs_append_ir2_opnd3(LISA_FSUB_D, &st0, &st0, &t_freg);
    latxs_ra_free_temp(&t_freg);
    return true;
}

bool latxs_translate_fisubr(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);
    (void)opnd_num; /* to avoid compile warning */

    IR2_OPND st0    = latxs_ra_alloc_st(0);
    IR2_OPND t_freg = latxs_ra_alloc_ftemp();

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    latxs_load_freg_from_ir1_2(&t_freg, opnd0, false, false);

    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    latxs_append_ir2_opnd3(LISA_FSUB_D, &st0, &t_freg, &st0);
    latxs_ra_free_temp(&t_freg);
    return true;
}

bool latxs_translate_fsubr(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &st0_opnd,
                                            &src1_opnd, &st0_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &dest_opnd,
                                            &src1_opnd, &dest_opnd);
    }

    return true;
}

bool latxs_translate_fsubrp(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        IR2_OPND st1 = latxs_ra_alloc_st(1);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &st1, &st0, &st1);
    } else if (opnd_num == 1) {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &dest_opnd,
                                            &st0, &dest_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &dest_opnd,
                                            &src1_opnd, &dest_opnd);
    }
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fsubp(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        IR2_OPND st1 = latxs_ra_alloc_st(1);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &st1, &st1, &st0);
    } else if (opnd_num == 1) {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &dest_opnd,
                                            &dest_opnd, &st0);
    }    else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FSUB_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fmul(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        latxs_append_ir2_opnd3(LISA_FMUL_D, &st0_opnd,
                                            &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FMUL_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }

    return true;
}

bool latxs_translate_fimul(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);
    (void)opnd_num; /* to avoid compile warning */

    IR2_OPND st0    = latxs_ra_alloc_st(0);
    IR2_OPND t_freg = latxs_ra_alloc_ftemp();

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    latxs_load_freg_from_ir1_2(&t_freg, opnd0, false, false);

    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    latxs_append_ir2_opnd3(LISA_FMUL_D, &st0, &t_freg, &st0);
    latxs_ra_free_temp(&t_freg);
    return true;
}

bool latxs_translate_fmulp(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        IR2_OPND st1 = latxs_ra_alloc_st(1);
        latxs_append_ir2_opnd3(LISA_FMUL_D, &st1, &st0, &st1);
    } else if (opnd_num == 1) {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        latxs_append_ir2_opnd3(LISA_FMUL_D, &dest_opnd,
                                            &dest_opnd, &st0);
    } else {
        IR2_OPND src0_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FMUL_D, &src0_opnd,
                                            &src0_opnd, &src1_opnd);
    }
    latxs_tr_fpu_pop();

    return true;
}

bool latxs_translate_fdiv(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &st0_opnd,
                                            &st0_opnd, &src1_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }
    return true;
}
bool latxs_translate_fdivr(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1 || opnd_num == 2);

    if (opnd_num == 1) {
        IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &st0_opnd,
                                            &src1_opnd, &st0_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &dest_opnd,
                                            &src1_opnd, &dest_opnd);
    }
    return true;
}

bool latxs_translate_fidiv(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);
    (void)opnd_num; /* to avoid compile warning */

    IR2_OPND st0    = latxs_ra_alloc_st(0);
    IR2_OPND t_freg = latxs_ra_alloc_ftemp();

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    latxs_load_freg_from_ir1_2(&t_freg, opnd0, false, false);

    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    latxs_append_ir2_opnd3(LISA_FDIV_D, &st0, &st0, &t_freg);
    latxs_ra_free_temp(&t_freg);
    return true;
}
bool latxs_translate_fidivr(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);
    lsassert(opnd_num == 1);
    (void)opnd_num; /* to avoid compile warning */

    IR2_OPND st0    = latxs_ra_alloc_st(0);
    IR2_OPND t_freg = latxs_ra_alloc_ftemp();

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    latxs_load_freg_from_ir1_2(&t_freg, opnd0, false, false);

    if (ir1_opnd_size(opnd0) > 32) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &t_freg, &t_freg);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &t_freg, &t_freg);
    }

    latxs_append_ir2_opnd3(LISA_FDIV_D, &st0, &t_freg, &st0);
    latxs_ra_free_temp(&t_freg);
    return true;
}

bool latxs_translate_fdivrp(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        IR2_OPND st1 = latxs_ra_alloc_st(1);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &st1, &st0, &st1);
    } else if (opnd_num == 1) {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &dest_opnd,
                                            &st0, &dest_opnd);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &dest_opnd,
                                            &src1_opnd, &dest_opnd);
    }
    latxs_tr_fpu_pop();
    return true;
}
bool latxs_translate_fdivp(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    int opnd_num = ir1_opnd_num(pir1);

    if (opnd_num == 0) {
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        IR2_OPND st1 = latxs_ra_alloc_st(1);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &st1, &st1, &st0);
    } else if (opnd_num == 1) {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND st0 = latxs_ra_alloc_st(0);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &dest_opnd,
                                            &dest_opnd, &st0);
    } else {
        IR2_OPND dest_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src1_opnd =
            latxs_load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false, true);
        latxs_append_ir2_opnd3(LISA_FDIV_D, &dest_opnd,
                                            &dest_opnd, &src1_opnd);
    }
    latxs_tr_fpu_pop();
    return true;
}

bool latxs_translate_fnop(IR1_INST *pir1)
{
    helper_cfg_t cfg = all_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fwait, cfg);
    return true;
}

bool latxs_translate_fsqrt(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
    latxs_append_ir2_opnd3(LISA_FSQRT_D, &st0_opnd, &st0_opnd, &st0_opnd);
    return true;
}

bool latxs_translate_fabs(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
    latxs_append_ir2_opnd3(LISA_FABS_D, &st0_opnd, &st0_opnd, &st0_opnd);
    return true;
}

bool latxs_translate_fchs(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
    latxs_append_ir2_opnd3(LISA_FNEG_D, &st0_opnd, &st0_opnd, &st0_opnd);
    return true;
}

bool latxs_translate_fsin(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU fsin to be implemented in LoongArch.\n");
    return true;
}

bool latxs_translate_fcos(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU fcos to be implemented in LoongArch.\n");
    return true;
}

bool latxs_translate_fpatan(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU fpatan to be implemented in LoongArch.\n");
    return true;
}

bool latxs_translate_fprem(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fprem, all_helper_cfg);
    return true;
}

bool latxs_translate_fprem1(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fprem1, all_helper_cfg);
    return true;
}

bool latxs_translate_frndint(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_frndint, all_helper_cfg);
    return true;
}

bool latxs_translate_fscale(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fscale, all_helper_cfg);
    return true;
}

bool latxs_translate_fxam(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fxam_ST0, all_helper_cfg);
    return true;
}

bool latxs_translate_f2xm1(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_f2xm1, all_helper_cfg);
    return true;
}

bool latxs_translate_fxtract(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU fxtract to be implemented in LoongArch.\n");
    return true;
}

bool latxs_translate_fyl2x(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND st0_opnd = latxs_ra_alloc_st(0);
    /* dispose the arguments */
    IR2_OPND param_0   = latxs_ir2_opnd_new(IR2_OPND_FPR, 0);
    IR2_OPND param_tmp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FMOV_D, &param_tmp, &st0_opnd);

    /* save regs before call helper func */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    latxs_append_ir2_opnd2(LISA_FMOV_D, &param_0, &param_tmp);
    /* Call the function  */
    latxs_tr_gen_call_to_helper((ADDR)log2);

    IR2_OPND ret_opnd, ret_value;
    ret_opnd = latxs_ir2_opnd_new(IR2_OPND_FPR, 0);
    ret_value = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FMOV_D, &ret_value, &ret_opnd);

    /* restore regs after native call*/
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    latxs_append_ir2_opnd2(LISA_FMOV_D, &st0_opnd, &ret_value);

    IR2_OPND st1_opnd = latxs_ra_alloc_st(1);
    latxs_append_ir2_opnd3(LISA_FMUL_D, &st1_opnd, &st0_opnd, &st1_opnd);

    latxs_tr_fpu_pop();

    return true;
}

bool latxs_translate_fyl2xp1(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU fyl2xp1 to be implemented in LoongArch.\n");

    return true;
}

bool latxs_translate_fsincos(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU fsincos to be implemented in LoongArch.\n");

    return true;
}

bool latxs_translate_fxch(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    IR2_OPND st0_opnd = latxs_ra_alloc_st(0);

    int opnd2_index;
    if (ir1_opnd_num(pir1) == 0) {
        opnd2_index = 1;
    } else {
        opnd2_index = ir1_opnd_base_reg_num(ir1_get_opnd(pir1, 0));
    }

    IR2_OPND opnd2 = latxs_ra_alloc_st(opnd2_index);
    IR2_OPND tmp_opnd = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd2(LISA_FMOV_D, &tmp_opnd, &opnd2);
    latxs_append_ir2_opnd2(LISA_FMOV_D, &opnd2, &st0_opnd);
    latxs_append_ir2_opnd2(LISA_FMOV_D, &st0_opnd, &tmp_opnd);

    latxs_ra_free_temp(&tmp_opnd);
    return true;
}
bool latxs_translate_ftst(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    lsassertm(0, "FPU ftst to be implemented in LoongArch.\n");

    return true;
}

bool latxs_translate_fptan(IR1_INST *pir1)
{
    if (latxs_tr_gen_fp_common_excp_check(pir1)) {
        return true;
    }

    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_fptan, all_helper_cfg);

    /*
     * for software matained top we have a difficulty here:
     * fptan can push(normal input) or not push(for out of range input)
     * we seems to have no way to decide next top statically
     * with lsfpu support, the adjusted top will be restored to hardware
     * when we return from helper.
     */
    if (!option_lsfpu) {
        latxs_tr_fpu_push();
    }

    return true;
}
