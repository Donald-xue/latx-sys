#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

int latxs_np_enabled(void)
{
    return option_native_printer;
}

int latxs_np_cs_enabled(void)
{
    return option_native_printer & LATXS_NP_CS;
}

int latxs_np_tlbcmp_enabled(void)
{
    return option_native_printer & LATXS_NP_TLBCMP;
}

int latxs_np_tb_enabled(void)
{
    return option_native_printer & LATXS_NP_TB;
}

static void latxs_native_printer_helper(lsenv_np_data_t *npd,
        int type, int r1, int r2, int r3, int r4, int r5)
{
    switch(npd->np_type) {
    case LATXS_NP_CS:
        /* optimization/sys-fastcs.c */
        latxs_native_printer_cs(npd, type,
                r1, r2, r3, r4, r5);
        return;
    default:
        break;
    }

    switch(type) {
    case LATXS_NP_TLBCMP:
        /* optimization/sys-softmmu.c */
        latxs_native_printer_tlbcmp(npd, type,
                r1, r2, r3, r4, r5);
        break;
    case LATXS_NP_TB:
        /* sys-translate.c */
        latxs_native_printer_tb(npd, type,
                r1, r2, r3, r4, r5);
        break;
    default:
        lsassertm(0, "unsupported native printer type");
        break;
    }
}

/*
 * native printer usage:
 * > lisa_bl latxs_native_printer
 *
 * note:
 * > all GPRs and only GPRs will be saved/restored
 */
int gen_latxs_native_printer(void *code_ptr)
{
    /* IR2_OPND *zero = &latxs_zero_ir2_opnd; */
    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;

    IR2_OPND reg = latxs_zero_ir2_opnd;

    int i = 0;

    latxs_tr_init(NULL);

    latxs_append_ir2_opnd2i(LISA_LD_D, env, env,
            offsetof(CPUX86State, np_data_ptr));

    /* save all native registers */
    for (i = 1; i < 32; ++i) {
        reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i);
        latxs_append_ir2_opnd2i(LISA_ST_D, &reg, env,
                offsetof(lsenv_np_data_t, np_regs) +
                sizeof(uint64_t) * i);
    }

    /* arg0 is pointing to lsenv->np_data */
    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
    latxs_tr_gen_call_to_helper((ADDR)latxs_native_printer_helper);

    /* read all native registers */
    reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i);
    for (i = 1; i < 32; ++i) {
        reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i);
        latxs_append_ir2_opnd2i(LISA_LD_D, &reg, env,
                offsetof(lsenv_np_data_t, np_regs) +
                sizeof(uint64_t) * i);
    }

    /* reset env = &CPUX86State */
    latxs_append_ir2_opnd2i(LISA_LD_D, env, env, 0);

    /* return */
    latxs_append_ir2_opnd0_(lisa_return);

    i = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return i;
}

void latxs_np_env_init(CPUX86State *env)
{
    env->np_data_ptr = &lsenv->np_data;
    lsenv->np_data.env = env;

    if (!lsenv->np_data.np_tb_chain_info) {
        lsenv->np_data.np_tb_chain_info =
            (uint8_t*)malloc(1024 * 1024 * 1024);
        lsenv->np_data.np_tb_chain_max = 1024 * 1024 * 1024;
        lsenv->np_data.np_tb_chain_nr = 0;
    }
}

/* context switch */
void latxs_np_tr_cs_prologue(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    if (latxs_np_cs_enabled()) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND ptr = latxs_ra_alloc_itemp();

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS_PRO);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_fastcs_t, cs_type));

        latxs_ra_free_temp(&tmp);

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = context_switch_bt_to_native + offset;
        int64_t ins_offset = ((int64_t)(latxs_native_printer - here)) >> 2;
        fprintf(stderr, "prologue %ld\n", ins_offset);
        latxs_append_ir2_opnda(LISA_BL, ins_offset);

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ST_D, zero, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_ra_free_temp(&ptr);
    }
}

void latxs_np_tr_cs_epilogue(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    if (latxs_np_cs_enabled()) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND ptr = latxs_ra_alloc_itemp();

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS_EPI);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_fastcs_t, cs_type));

        latxs_ra_free_temp(&tmp);

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = context_switch_native_to_bt_ret_0 + offset;
        int64_t ins_offset = (int64_t)(latxs_native_printer - here) >> 2;
        fprintf(stderr, "epilogue %ld\n", ins_offset);
        latxs_append_ir2_opnda(LISA_BL, ins_offset);

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ST_D, zero, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_ra_free_temp(&ptr);
    }
}

void latxs_np_tr_scs_prologue(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd;

    if (latxs_np_cs_enabled()) {
        latxs_append_ir2_opnd2i(LISA_LD_D, stmp2,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, stmp1, zero, LATXS_NP_CS);
        latxs_append_ir2_opnd2i(LISA_ST_D, stmp1, stmp2,
                offsetof(lsenv_np_data_t, np_type));

        latxs_append_ir2_opnd2i(LISA_LD_D, stmp2,
                env, offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, stmp1, zero, LATXS_NP_CS_SPRO);
        latxs_append_ir2_opnd2i(LISA_ST_D, stmp1, stmp2,
                offsetof(lsenv_fastcs_t, cs_type));

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = latxs_sc_scs_prologue + offset;
        int64_t ins_offset = (int64_t)(latxs_native_printer - here) >> 2;
        fprintf(stderr, "static prologue %ld\n", ins_offset);
        latxs_append_ir2_opnda(LISA_BL, ins_offset);

        latxs_append_ir2_opnd2i(LISA_LD_D, stmp2,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ST_D, zero, stmp2,
                offsetof(lsenv_np_data_t, np_type));
    }
}

void latxs_np_tr_scs_epilogue(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd;

    if (latxs_np_cs_enabled()) {
        latxs_append_ir2_opnd2i(LISA_LD_D, stmp2,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, stmp1, zero, LATXS_NP_CS);
        latxs_append_ir2_opnd2i(LISA_ST_D, stmp1, stmp2,
                offsetof(lsenv_np_data_t, np_type));

        latxs_append_ir2_opnd2i(LISA_LD_D, stmp2,
                env, offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, stmp1, zero, LATXS_NP_CS_SEPI);
        latxs_append_ir2_opnd2i(LISA_ST_D, stmp1, stmp2,
                offsetof(lsenv_fastcs_t, cs_type));

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = latxs_sc_scs_epilogue + offset;
        int64_t ins_offset = (int64_t)(latxs_native_printer - here) >> 2;
        fprintf(stderr, "static epilogue %ld\n", ins_offset);
        latxs_append_ir2_opnda(LISA_BL, ins_offset);

        latxs_append_ir2_opnd2i(LISA_LD_D, stmp2,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ST_D, zero, stmp2,
                offsetof(lsenv_np_data_t, np_type));
    }
}

void latxs_np_tr_hcs_prologue(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    if (latxs_np_cs_enabled()) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;

        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND ptr = latxs_ra_alloc_itemp();

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS_HPRO);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_fastcs_t, cs_type));

        latxs_ra_free_temp(&tmp);

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = (ADDR)(tb->tc.ptr) + offset;
        int64_t ins_offset = ((int64_t)(latxs_native_printer - here)) >> 2;
        latxs_append_ir2_opnda(LISA_BL, ins_offset);

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ST_D, zero, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_ra_free_temp(&ptr);
    }
}

void latxs_np_tr_hcs_epilogue(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    if (latxs_np_cs_enabled()) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;

        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND ptr = latxs_ra_alloc_itemp();

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd2i(LISA_ORI, &tmp, zero, LATXS_NP_CS_HEPI);
        latxs_append_ir2_opnd2i(LISA_ST_D, &tmp, &ptr,
                offsetof(lsenv_fastcs_t, cs_type));

        latxs_ra_free_temp(&tmp);

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = (ADDR)(tb->tc.ptr) + offset;
        int64_t ins_offset = (int64_t)(latxs_native_printer - here) >> 2;
        latxs_append_ir2_opnda(LISA_BL, ins_offset);

        latxs_append_ir2_opnd2i(LISA_LD_D, &ptr,
                env, offsetof(CPUX86State, np_data_ptr));
        latxs_append_ir2_opnd2i(LISA_ST_D, zero, &ptr,
                offsetof(lsenv_np_data_t, np_type));

        latxs_ra_free_temp(&ptr);
    }
}

/* TB execution */
void latxs_np_tr_tb_start(void)
{
    if (latxs_np_tb_enabled()) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;

        IR2_OPND *zero = &latxs_zero_ir2_opnd;
        IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
        IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
        IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;

        latxs_append_ir2_opnd2i(LISA_ORI, arg1, zero, LATXS_NP_TB);
        latxs_append_ir2_opnd2i(LISA_ORI, arg2, zero, 1);
        latxs_append_ir2_opnd2i(LISA_ORI, arg3, zero, tb->flags & 0x3);

        if (latxs_fastcs_enable_tbctx()) {
            IR2_OPND *arg4 = &latxs_arg4_ir2_opnd;
            latxs_append_ir2_opnd2i(LISA_ORI, arg4, zero, tb->fastcs_ctx);
        }

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = (ADDR)(tb->tc.ptr) + offset;
        int64_t ins_offset = (int64_t)(latxs_native_printer - here) >> 2;
        latxs_append_ir2_opnda(LISA_BL, ins_offset);
    }
}

void latxs_np_tr_tb_end(void)
{
    if (latxs_np_tb_enabled()) {
        TranslationBlock *tb = lsenv->tr_data->curr_tb;

        IR2_OPND *zero = &latxs_zero_ir2_opnd;
        IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
        IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
        IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;

        latxs_append_ir2_opnd2i(LISA_ORI, arg1, zero, LATXS_NP_TB);
        latxs_append_ir2_opnd2i(LISA_ORI, arg2, zero, 2);
        latxs_append_ir2_opnd2i(LISA_ORI, arg3, zero, tb->flags & 0x3);

        if (latxs_fastcs_enable_tbctx()) {
            IR2_OPND *arg4 = &latxs_arg4_ir2_opnd;
            latxs_append_ir2_opnd2i(LISA_ORI, arg4, zero, tb->fastcs_ctx);
        }

        int offset = lsenv->tr_data->real_ir2_inst_num << 2;
        ADDR here = (ADDR)(tb->tc.ptr) + offset;
        int64_t ins_offset = (int64_t)(latxs_native_printer - here) >> 2;
        latxs_append_ir2_opnda(LISA_BL, ins_offset);
    }
}

/* native printer for TB's execution */
void latxs_native_printer_tb(lsenv_np_data_t *npd, int type,
        int r1, int r2, int r3, int r4, int r5)
{
    /*
     * @r1: TB execution type
     * type = 1 : one TB start executing
     * type = 2 : one TB exit by interrupt
     *
     * @r2: TB CPL (0/3)
     *
     * @r3: TB.fastcs_ctx when fastcs TB ctx is enabled
     */
    int tb_type = r1;
    switch(tb_type) {
    case 1:
        npd->np_tb_counter += 1;
        break;
    case 2:
        /*
         * npd->np_tb_flag
         * [0]   = 0 : TB exit normally
         *       = 1 : TB exit because of interrupt
         * [2:1] TB's CPL
         */
        npd->np_tb_flag = 1;
        break;
    default:
        break;
    }

    npd->np_tb_flag |= (r2 & 0x3) << 1;

    if (latxs_fastcs_enable_tbctx()) {
        int tb_ctx = r3 & 0x3;

        uint8_t tb_chain_info = 0;
        tb_chain_info |= tb_ctx & 0x3;

        int idx = npd->np_tb_chain_nr;
        npd->np_tb_chain_info[idx] = tb_chain_info;
        npd->np_tb_chain_nr += 1;
        if (idx > npd->np_tb_chain_max) {
            lsassertm(0, "[NPD] not enough space for tb chain info");
        }
    }
}

void latxs_np_tb_print(CPUX86State *env)
{
    if (latxs_np_tb_enabled()) {
        lsenv_np_data_t *npd = env->np_data_ptr;
        int tb_cpl = (npd->np_tb_flag >> 1) & 0x3;

        if ((npd->np_tb_flag & 1) == 1) {
            fprintf(stderr, "TB %ld CPL %d I",
                    npd->np_tb_counter, tb_cpl);
        } else {
            fprintf(stderr, "TB %ld CPL %d",
                    npd->np_tb_counter, tb_cpl);
        }

        int i = 0;
        if (npd->np_tb_chain_nr > 0) {
            fprintf(stderr, " %d", (int)npd->np_tb_chain_info[0]);
        }
        for (i = 1; i < npd->np_tb_chain_nr; ++i) {
            fprintf(stderr, "%d", (int)npd->np_tb_chain_info[i]);
        }

        fprintf(stderr, "\n");

        npd->np_tb_counter = 0;
        npd->np_tb_flag = 0;
        npd->np_tb_chain_nr = 0;
    }
}
