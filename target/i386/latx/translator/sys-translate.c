#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

/* main translation procees */

void latxs_tr_init(TranslationBlock *tb)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td);

    /* 1. set current tb and ir1 */
    latxs_tr_tb_init(td, tb);

    /* 2. register allocation init */
    latxs_tr_ra_init(td);

    /* 3. reset all registers extension mode */
    latxs_tr_em_init(td);

    /* 4. reset ir2 array */
    latxs_tr_ir2_array_init(td);

    /* 5. set up lsfpu */
    latxs_tr_fpu_init(td, tb);
}

void latxs_tr_tb_init(TRANSLATION_DATA *td, TranslationBlock *tb)
{
    td->curr_tb = NULL;
    td->curr_ir1_inst = NULL;
}

void latxs_tr_ir2_array_init(TRANSLATION_DATA *td)
{
    if (td->ir2_inst_array == NULL) {
        td->ir2_inst_array = mm_calloc(512, sizeof(IR2_INST));
        td->ir2_inst_num_max = 512;
    }
    td->ir2_inst_num_current = 0;
    td->real_ir2_inst_num = 0;

    td->first_ir2 = NULL;
    td->last_ir2 = NULL;

    td->label_num = 0;
}

void latxs_tr_fini(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td != NULL);

    td->curr_tb = NULL;
    td->curr_ir1_inst = NULL;

    td->ir2_inst_num_current = 0;
    td->real_ir2_inst_num = 0;

    td->first_ir2 = NULL;
    td->last_ir2 = NULL;

    td->label_num = 0;
    td->itemp_num = 32;
    td->ftemp_num = 32;

    if (!option_lsfpu) {
        latxs_td_fpu_set_top(0);
    }
}

void latxs_label_dispose(void *code_buffer)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    int label_nr = td->label_num;

    /* 1. record the positions of label */
    int *label_pos = (int *)alloca(label_nr * 4 + 20);
    memset(label_pos , -1, label_nr * 4 + 20);

    int ir2_num = 0;
    IR2_INST *ir2_current = td->first_ir2;
    IR2_OPCODE ir2_opc = latxs_ir2_opcode(ir2_current);

    while (ir2_current != NULL) {
        ir2_opc = latxs_ir2_opcode(ir2_current);

        if (ir2_opc  == LISA_LABEL) {
            int label_id = latxs_ir2_opnd_label_id(&ir2_current->_opnd[0]);
            lsassertm(label_pos[label_id] == -1,
                      "label %d is in multiple positions\n", label_id);
            label_pos[label_id] = ir2_num;
        } else if (ir2_opc == LISA_X86_INST) {
            /* will not be assembled */
        } else {
            ir2_num++;
        }

        ir2_current = latxs_ir2_next(ir2_current);
    }

    /*
     * 2. resolve the offset of successor linkage code
     * @jmp_target_arg recoed the jmp  inst position
     * @jmp_reset_offset record the successor inst of jmp(exclude delay slot)
     *                   when the tb is removed from buffer. the jmp inst use
     *                   this position to revert the original "fall through"
     */
    {
        TranslationBlock *tb = td->curr_tb;
        /* prologue/epilogue has no tb */
        if (tb) {
            /* ctx->jmp_insn_offset point to tb->jmp_target_arg */
            int label_id_0 = tb->jmp_reset_offset[0];
            if (label_id_0 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[0] =
                    (label_pos[label_id_0] << 2) + 8;
                tb->jmp_target_arg[0] = (label_pos[label_id_0] << 2);
            }
            int label_id_1 = tb->jmp_reset_offset[1];
            if (label_id_1 != TB_JMP_RESET_OFFSET_INVALID) {
                tb->jmp_reset_offset[1] =
                    (label_pos[label_id_1] << 2) + 8;
                tb->jmp_target_arg[1] = (label_pos[label_id_1] << 2);
            }
        }
    }

    /* 3. resolve the branch instructions */
    ir2_num = 0;
    ir2_current = td->first_ir2;

    while (ir2_current != NULL) {
        ir2_opc = latxs_ir2_opcode(ir2_current);
        if (latxs_ir2_opcode_is_branch(ir2_opc)) {

            IR2_OPND *label_opnd = latxs_ir2_branch_get_label(ir2_current);

            /* LISA_B and LISA_BL could contain label or imm */
            if (label_opnd && latxs_ir2_opnd_is_label(label_opnd)) {
                int label_id = latxs_ir2_opnd_label_id(label_opnd);
                lsassert(label_id > 0 && label_id <= label_nr);

                int label_pos_ir2_num = label_pos[label_id];

                lsassertm(label_pos_ir2_num != -1,
                        "label %d is not inserted\n",
                         label_id);

                latxs_ir2_opnd_convert_label_to_imm(
                        label_opnd, label_pos_ir2_num - ir2_num);
            }
        }

        if (ir2_opc != LISA_LABEL && ir2_opc != LISA_X86_INST) {
            ir2_num++;
        }

        ir2_current = latxs_ir2_next(ir2_current);
    }
}

int latxs_tr_ir2_assemble(void *code_base)
{
    /* 1. label dispose */
    latxs_label_dispose(code_base);

    /* 2. assemble */
    IR2_INST *pir2 = lsenv->tr_data->first_ir2;
    void *code_ptr = code_base;
    int code_nr = 0;

    while (pir2 != NULL) {
        IR2_OPCODE ir2_opc = latxs_ir2_opcode(pir2);

        if (ir2_opc == LISA_X86_INST) {
            goto _NEXT_IR2_;
        }

        if (ir2_opc != LISA_LABEL) {
            uint32_t ir2_binary = latxs_ir2_assemble(pir2);
            if (option_dump_host) {
                fprintf(stderr, "IR2 at %p LISA Binary = 0x%08x ",
                        code_ptr, ir2_binary);
                latxs_ir2_dump(pir2);
            }

            *(uint32_t *)code_ptr = ir2_binary;
            code_ptr = code_ptr + 4;
            code_nr += 1;
        }

_NEXT_IR2_:
        pir2 = latxs_ir2_next(pir2);
    }

    return code_nr;
}
