#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "flag-lbt.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_mov_register_ir1(void)
{
    latxs_register_ir1(X86_INS_MOV);
    latxs_register_ir1(X86_INS_MOVZX);
    latxs_register_ir1(X86_INS_MOVSX);
    latxs_register_ir1(X86_INS_LEA);
    latxs_register_ir1(X86_INS_MOVQ);
    latxs_register_ir1(X86_INS_MOVD);

    latxs_register_ir1(X86_INS_CMOVO);
    latxs_register_ir1(X86_INS_CMOVNO);
    latxs_register_ir1(X86_INS_CMOVB);
    latxs_register_ir1(X86_INS_CMOVAE);
    latxs_register_ir1(X86_INS_CMOVE);
    latxs_register_ir1(X86_INS_CMOVNE);
    latxs_register_ir1(X86_INS_CMOVBE);
    latxs_register_ir1(X86_INS_CMOVA);
    latxs_register_ir1(X86_INS_CMOVS);
    latxs_register_ir1(X86_INS_CMOVNS);
    latxs_register_ir1(X86_INS_CMOVP);
    latxs_register_ir1(X86_INS_CMOVNP);
    latxs_register_ir1(X86_INS_CMOVL);
    latxs_register_ir1(X86_INS_CMOVGE);
    latxs_register_ir1(X86_INS_CMOVLE);
    latxs_register_ir1(X86_INS_CMOVG);
}

bool latxs_translate_mov(IR1_INST *pir1)
{
    /* TranslationBlock *tb = lsenv->tr_data->curr_tb; */
    /* if (tb->cflags & CF_USE_ICOUNT) { TODO */
        /* if (ir1_is_mov_to_cr(pir1) || ir1_is_mov_from_cr(pir1)){ */
            /* tr_gen_io_start(); */
        /* } */
    /* } */

    if (option_by_hand) {
        return latxs_translate_mov_byhand(pir1);
    }

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 1), EXMode_N, false);
    latxs_store_ir2_to_ir1(&src, ir1_get_opnd(pir1, 0), false);

    return true;
}

bool latxs_translate_movzx(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_movzx_byhand(pir1);
    }

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 1), EXMode_Z, false);
    latxs_store_ir2_to_ir1(&src, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool latxs_translate_movsx(IR1_INST *pir1)
{
    if (option_by_hand) {
        return latxs_translate_movsx_byhand(pir1);
    }

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, ir1_get_opnd(pir1, 1), EXMode_S, false);
    latxs_store_ir2_to_ir1(&src, ir1_get_opnd(pir1, 0), false);
    return true;
}

bool latxs_translate_lea(IR1_INST *pir1)
{
    IR2_OPND value_opnd = latxs_ra_alloc_itemp();
    int addr_size = latxs_ir1_addr_size(pir1);
    latxs_load_ir1_mem_addr_to_ir2(&value_opnd,
            ir1_get_opnd(pir1, 1), addr_size);

    latxs_store_ir2_to_ir1(&value_opnd,
            ir1_get_opnd(pir1, 0), false);
    return true;
}

bool latxs_translate_movq(IR1_INST *pir1)
{
    if (latxs_tr_gen_sse_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    /* XMM */

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&temp, src, EXMode_Z, false, -1);
        /* IR2_OPND temp = latxs_load_freg_from_ir1_1(src, false, false); */
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        /* latxs_append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &temp, 7); */
        latxs_append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_D, &xmm_dest, &temp, 0);
        return true;
    }

    if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_src = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        latxs_store_freg_to_ir1(&xmm_src, dest, false, false);
        return true;
    }

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        /* latxs_append_ir2_opnd2i(LISA_VCLRSTRI_V, &xmm_dest, &xmm_src, 7); */
        /* latxs_append_ir2_opnd2i(LISA_XVPICKVE_D, &xmm_dest, &xmm_src, 0); */
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_D, &temp, &xmm_src, 0);
        latxs_append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_D, &xmm_dest, &temp, 0);
        return true;
    }

    /* MMX */

    if (ir1_opnd_is_mmx(dest)) { /* dest mmx : src mmx/mem */
        IR2_OPND dest_opnd = latxs_load_freg_from_ir1_1(dest, false, false);
        latxs_load_freg_from_ir1_2(&dest_opnd, src, false, false);
        return true;
    }

    if (ir1_opnd_is_mmx(src)) { /* src mmx  : dest mmx/mem */
        IR2_OPND source_opnd = latxs_load_freg_from_ir1_1(src, false, false);
        latxs_store_freg_to_ir1(&source_opnd, dest, false, false);
        return true;
    }

    lsassertm(0, "unknown SIMD type %s.\n", __func__);

    return true;
}

bool latxs_translate_movd(IR1_INST *pir1)
{
    if (latxs_tr_gen_sse_common_excp_check(pir1)) {
        return true;
    }

    IR1_OPND *dest = ir1_get_opnd(pir1, 0);
    IR1_OPND *src  = ir1_get_opnd(pir1, 1);

    /* XMM */

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_mem(src)) {
        IR2_OPND temp = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&temp, src, EXMode_Z, false, -1);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        latxs_append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_W, &xmm_dest, &temp, 0);
        return true;
    }

    if (ir1_opnd_is_xmm(dest) && ir1_opnd_is_gpr(src)) {
        IR2_OPND gpr_src = latxs_ra_alloc_itemp();
        latxs_load_ir1_gpr_to_ir2(&gpr_src, src, EXMode_Z);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(dest));
        latxs_append_ir2_opnd1i(LISA_VLDI, &xmm_dest, 0);
        latxs_append_ir2_opnd2i(LISA_VINSGR2VR_W, &xmm_dest, &gpr_src, 0);
        return true;
    }

    if (ir1_opnd_is_mem(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND xmm_src = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        latxs_store_freg_to_ir1(&xmm_src, dest, false, false);
        return true;
    }

    if (ir1_opnd_is_gpr(dest) && ir1_opnd_is_xmm(src)) {
        IR2_OPND gpr_dest = latxs_ra_alloc_gpr(ir1_opnd_base_reg_num(dest));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(ir1_opnd_base_reg_num(src));
        latxs_append_ir2_opnd2(LISA_MOVFR2GR_S, &gpr_dest, &xmm_src);
        return true;
    }

    /* MMX */
    IR2_OPND src_tmp = latxs_ra_alloc_itemp();

    if (ir1_opnd_is_gpr(dest) || ir1_opnd_is_mem(dest)) {
        lsassert(ir1_opnd_is_mmx(src));
        latxs_load_ir1_to_ir2(&src_tmp, src, EXMode_N, false);
        latxs_store_ir2_to_ir1(&src_tmp, dest, false);
        return true;
    } else if (ir1_opnd_is_gpr(src) || ir1_opnd_is_mem(src)) {
        lsassert(ir1_opnd_is_mmx(dest));
        latxs_load_ir1_to_ir2(&src_tmp, src, EXMode_Z, false);
        latxs_store_ir2_to_ir1(&src_tmp, dest, false);
        return true;
    }

    lsassertm(0, "unknown SIMD type %s.\n", __func__);

    return true;
}

#define CMOVCC_ASSERT(pir1, os0, os1) \
    do { \
        lsassertm_illop(ir1_addr(pir1), \
            (os0 == 16 || os1 == 32) && os0 == os1, \
            "cmov opnd size0 %d size1 %d is unsupported.\n", \
            os0, os1); \
    } while (0)

#define CMOVCC_STORE_IR2_TO_IR1(src, opnd00) \
    do { \
        latxs_store_ir2_to_ir1(&src, opnd00, false); \
        if (option_by_hand && ir1_opnd_is_gpr(opnd00)) { \
            int gpr_num = ir1_opnd_base_reg_num(opnd00); \
            latxs_td_set_reg_extmb(gpr_num, EXMode_N, 32); \
        } \
    } while (0)

bool latxs_translate_cmovo(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* OF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if OF = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovno(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* OF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if OF = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovb(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* CF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if CF = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovae(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* CF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if CF = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovz(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* ZF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd  = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if ZF = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovnz(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* ZF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if ZF = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovbe(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* ZF, CF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if ZF = 1 or CF = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmova(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* ZF, CF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if ZF = 0 && CF = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovs(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* SF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if SF = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovns(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* SF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if SF = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovp(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* PF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if PF = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovnp(IR1_INST *pir1)
{
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* PF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if PF = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovl(IR1_INST *pir1)
{ /* sf != of */
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* cond = SF xor OF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if cond = 1 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovge(IR1_INST *pir1)
{ /* sf == of */
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* cond = SF xor OF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if cond = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovle(IR1_INST *pir1)
{ /* zf==1 || sf!=of */
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* ZF, OF xor SF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if cond != 0 */
    latxs_append_ir2_opnd2(LISA_BEQZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}

bool latxs_translate_cmovg(IR1_INST *pir1)
{ /* zf==0 && sf==of */
    IR2_OPND condition = latxs_ra_alloc_itemp(); /* ZF, OF xor SF */
    latxs_get_eflag_condition(&condition, pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1); /* GPR/MEM */

    int opnd_size_0 = ir1_opnd_size(opnd0);
    int opnd_size_1 = ir1_opnd_size(opnd1);
    CMOVCC_ASSERT(pir1, opnd_size_0, opnd_size_1);

    IR2_OPND src_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_opnd, opnd1, EXMode_N, false);

    IR2_OPND no_mov = latxs_ir2_opnd_new_label(); /* mov if cond = 0 */
    latxs_append_ir2_opnd2(LISA_BNEZ, &condition, &no_mov);
    CMOVCC_STORE_IR2_TO_IR1(src_opnd, opnd0);
    latxs_append_ir2_opnd1(LISA_LABEL, &no_mov);

    latxs_ra_free_temp(&condition);
    return true;
}
