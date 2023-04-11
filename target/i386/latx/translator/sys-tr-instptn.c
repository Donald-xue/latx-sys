#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>
#include "etb.h"
#include "latx-sys-inst-ptn.h"

#ifdef LATXS_INSTPTN_ENABLE

#define WRAP(ins) (X86_INS_##ins)

#define LOAD_TB_PTR_TO_DBT_ARG1() do {                      \
    if (!option_lsfpu && !option_soft_fpu) {                \
        IR2_OPND tb_ptr_opnd = latxs_ra_alloc_dbt_arg1();   \
        latxs_tr_gen_exit_tb_load_tb_addr(&tb_ptr_opnd,     \
                (ADDR)lsenv->tr_data->curr_tb);             \
    }                                                       \
} while (0)

static
bool translate_cmp_jcc(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;
    IR1_INST *nir1 = NULL;

    lsassert(pir1->instptn.next >= 0);

    nir1 = tb_ir1_inst(tb, pir1->instptn.next);

#if 0
    latxs_translate_cmp_byhand(pir1);
    switch (ir1_opcode(nir1)) {
    case WRAP(JB):  latxs_translate_jb(nir1); break;
    case WRAP(JAE): latxs_translate_jae(nir1); break;
    case WRAP(JE):  latxs_translate_jz(nir1); break;
    case WRAP(JNE): latxs_translate_jnz(nir1); break;
    case WRAP(JBE): latxs_translate_jbe(nir1); break;
    case WRAP(JA):  latxs_translate_ja(nir1); break;
    case WRAP(JL):  latxs_translate_jl(nir1); break;
    case WRAP(JGE): latxs_translate_jge(nir1); break;
    case WRAP(JLE): latxs_translate_jle(nir1); break;
    case WRAP(JG):  latxs_translate_jg(nir1); break;
    default:        lsassert(0);        break;
    }
    return true;
#endif

#if 0
    ir1_dump(pir1);
    ir1_dump(nir1);
    fprintf(stderr, "---------------\n");
#endif

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    EXMode em = EXMode_S;

    IR2_OPND src0, src1;
    if (ir1_opnd_is_gpr(opnd0)) {
        src0 = latxs_convert_gpr_opnd(opnd0, em);
        if (ir1_opnd_is_gpr(opnd1)) {
            /* gpr, gpr */
            src1 = latxs_convert_gpr_opnd(opnd1, em);
        } else if (ir1_opnd_is_imm(opnd1)) {
            /* gpr, imm */
            src1 = latxs_ra_alloc_itemp();
            latxs_load_ir1_imm_to_ir2(&src1, opnd1, em);
        } else {
            lsassert(ir1_opnd_is_mem(opnd1));
            /* gpr, mem */
            src1 = latxs_ra_alloc_itemp();
            latxs_load_ir1_mem_to_ir2(&src1, opnd1, em, -1);
        }
    } else {
        lsassert(ir1_opnd_is_mem(opnd0));
        src0 = latxs_ra_alloc_itemp();
        latxs_load_ir1_mem_to_ir2(&src0, opnd0, em, -1);
        if (ir1_opnd_is_gpr(opnd1)) {
            /* mem, gpr */
            src1 = latxs_convert_gpr_opnd(opnd1, em);
        } else {
            lsassert(ir1_opnd_is_imm(opnd1));
            /* mem, imm */
            src1 = latxs_ra_alloc_itemp();
            latxs_load_ir1_imm_to_ir2(&src1, opnd1, em);
        }
    }

    IR2_OPND target_label = latxs_ir2_opnd_new_label();
    IR2_OPCODE jcc_op;

    int swap_opnd = 0;
    switch (ir1_opcode(nir1)) {
    case WRAP(JB):  jcc_op = LISA_BLTU; break;
    case WRAP(JAE): jcc_op = LISA_BGEU; break;
    case WRAP(JE):  jcc_op = LISA_BEQ;  break;
    case WRAP(JNE): jcc_op = LISA_BNE;  break;
    case WRAP(JBE): jcc_op = LISA_BGEU; swap_opnd = 1; break;
    case WRAP(JA):  jcc_op = LISA_BLTU; swap_opnd = 1; break;
    case WRAP(JL):  jcc_op = LISA_BLT;  break;
    case WRAP(JGE): jcc_op = LISA_BGE;  break;
    case WRAP(JLE): jcc_op = LISA_BGE;  swap_opnd = 1; break;
    case WRAP(JG):  jcc_op = LISA_BLT;  swap_opnd = 1; break;
    default:        lsassert(0);        break;
    }

    int os = ir1_opnd_size(opnd0); /* 8, 16, 32 */
    latxs_append_ir2_opnd2(LISA_X86SUB_B + (os >> 4), &src0, &src1);

    latxs_tr_gen_eob();
    LOAD_TB_PTR_TO_DBT_ARG1();

    if (swap_opnd) {
        latxs_append_ir2_opnd3(jcc_op, &src1, &src0, &target_label);
    } else {
        latxs_append_ir2_opnd3(jcc_op, &src0, &src1, &target_label);
    }

    latxs_tr_generate_exit_tb(nir1, 0);
    latxs_append_ir2_opnd1(LISA_LABEL, &target_label);
    latxs_tr_generate_exit_tb(nir1, 1);

    return true;

#if 0
pattern_fail:
    pir1->instptn.opc = INSTPTN_OPC_NONE;
    nir1->instptn.opc = INSTPTN_OPC_NONE;
    return false;
#endif
}

static
bool translate_test_jcc(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;
    IR1_INST *nir1 = NULL;

    lsassert(pir1->instptn.next >= 0);

    nir1 = tb_ir1_inst(tb, pir1->instptn.next);

    /*ir1_dump(pir1);*/
    /*ir1_dump(nir1);*/
    /*fprintf(stderr, "---------------\n");*/

    pir1->instptn.opc = INSTPTN_OPC_NONE;
    nir1->instptn.opc = INSTPTN_OPC_NONE;
    return false;
}

bool try_translate_instptn(IR1_INST *pir1)
{
    latxs_instptn_check_false();

    switch (pir1->instptn.opc) {
    case INSTPTN_OPC_NONE:
        return false;
    case INSTPTN_OPC_NOP:
        return true;
    case INSTPTN_OPC_CMP_JCC:
        return translate_cmp_jcc(pir1);
    case INSTPTN_OPC_TEST_JCC:
        return translate_test_jcc(pir1);
    default:
        lsassert(0);
        break;
    }

    return false;
}

#endif
