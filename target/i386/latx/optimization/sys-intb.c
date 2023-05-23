#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "latxs-fastcs-cfg.h"
#include "latx-perfmap.h"
#include "latx-bpc-sys.h"
#include "latx-np-sys.h"
#include "latx-intb-sys.h"
#include "latx-sigint-sys.h"
#include "latxs-cc-pro.h"

#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#ifdef LATXS_INTB_LINK_ENABLE

#ifdef LATXS_INTB_LINK_OPTION_CHECK
int __intb_link_enable(void)
{
    return option_intb_link;
}
#endif

#ifdef TARGET_X86_64
#define LISA_LOAD_PC    LISA_LD_D
#define LISA_STORE_PC   LISA_ST_D
#else
#define LISA_LOAD_PC    LISA_LD_WU
#define LISA_STORE_PC   LISA_ST_W
#endif

static void tr_gen_pb_insert(IR2_OPND *next_tcptr,
        IR2_OPND *env,
        IR2_OPND *thistb, IR2_OPND *nexttb,
        IR2_OPND *tmp,
        int use_cc_pro)
{
    if (!option_intb_pb) return;

    latxs_append_ir2_opnd2i(LISA_ST_D, next_tcptr, thistb,
        offsetof(TranslationBlock, intb_target[0].tc_ptr));

    latxs_append_ir2_opnd2i(LISA_LOAD_PC, tmp, env,
            lsenv_offset_of_eip(lsenv));
    latxs_append_ir2_opnd2i(LISA_STORE_PC, tmp, thistb,
        offsetof(TranslationBlock, intb_target[0].pc));

    if (use_cc_pro && latxs_cc_pro()) {
        lsassertm(!(CC_FLAG_MASK >> 12),
                "%s %d : cc mask out of range\n", __func__, __LINE__);
        latxs_append_ir2_opnd2i(LISA_LD_WU, tmp, nexttb,
                offsetof(TranslationBlock, flags));
        latxs_append_ir2_opnd2i(LISA_ANDI, tmp, tmp, CC_FLAG_MASK);
        latxs_append_ir2_opnd2i(LISA_ST_W, tmp, thistb,
                offsetof(TranslationBlock, intb_target[0].flags));

        latxs_append_ir2_opnd2i(LISA_LD_WU, tmp, nexttb,
                offsetof(TranslationBlock, cc_mask));
        latxs_append_ir2_opnd2i(LISA_ST_W, tmp, thistb,
                offsetof(TranslationBlock, intb_target[0].mask));
    }
}

static void tr_gen_pb_lookup(IR2_OPND *thistb,
        IR2_OPND *env,
        IR2_OPND *tmp, IR2_OPND *tmp0, IR2_OPND *tmp1,
        int use_cc_pro)
{

    if (!option_intb_pb) return;

    IR2_OPND pb_miss = latxs_ir2_opnd_new_label();

    /* check PC */
    latxs_append_ir2_opnd2i(LISA_LOAD_PC, tmp0, thistb,
            offsetof(TranslationBlock, intb_target[0].pc));
    latxs_append_ir2_opnd2i(LISA_LOAD_PC, tmp1, env,
            lsenv_offset_of_eip(lsenv));
    latxs_append_ir2_opnd3(LISA_BNE, tmp0, tmp1, &pb_miss);

    /* cc pro check hflags */
    if (use_cc_pro && latxs_cc_pro()) {

        latxs_append_ir2_opnd2i(LISA_LD_WU, tmp0, thistb,
                offsetof(TranslationBlock, intb_target[0].flags));
        latxs_append_ir2_opnd2i(LISA_LD_WU, tmp1, env,
                offsetof(CPUX86State, hflags));

        latxs_append_ir2_opnd2i(LISA_LD_WU, tmp, thistb,
                offsetof(TranslationBlock, intb_target[0].mask));
        latxs_append_ir2_opnd3(LISA_AND, tmp0, tmp0, tmp);
        latxs_append_ir2_opnd3(LISA_AND, tmp1, tmp1, tmp);

        latxs_append_ir2_opnd3(LISA_BNE, tmp0, tmp1, &pb_miss);
    }

    /* private buffer hit: jump to next TB */
    latxs_append_ir2_opnd2i(LISA_LD_D, tmp0, thistb,
            offsetof(TranslationBlock, intb_target[0].tc_ptr));
    latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_ra_ir2_opnd, tmp0, 0);

    latxs_append_ir2_opnd1(LISA_LABEL, &pb_miss);
}

/*
 * 0. Private Buffer Lookup
 *    0.1 lookup private buffer // => 1. if fail
 *    0.2 jump to next TB
 * 1. Native Jmp Cache Lookup
 *    1.1 lookup jmp cache // => 2. if NULL
 *    1.2 check PC // => 2. if miss
 *    1.3 check TB.cflags CF_INVALID // => 2. if miss
 *    1.4 check CS_BASE // => 2. if miss
 *    1.5 check TB.flags with env->hflags // => 2. if miss
 *    1.6 => 3.
 * 2. Helper Lookup TB
 *    2.1 call helper lookup tb // => 3. if find TB
 *    2.2 jump to epilogue //  => epilogue if no valid TB
 * 3. Jmp Glue Check
 *    3.1 (FastCS.nolink) check // => 2.2 if check fail
 *    3.2 (FastCS.jmpglue) process
 *    3.3 (FPU Rotate) process // rotate and jump to next TB
 *    3.4 (SIGINT) check // => epilogue if interrupt pending
 *    3.5 (PB) insert private buffer
 *    3.6 jump to next TB
 */

int gen_latxs_intb_lookup(void *code_ptr)
{
    int code_nr = 0;

    latxs_tr_init(NULL);

    TRANSLATION_DATA *td = lsenv->tr_data;
    int rid = td->region_id;
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    int use_cc_pro = latxs_cc_pro();
#ifdef LATX_USE_MULTI_REGION
    if (!latx_rid_is_cpl3(rid)) {
        use_cc_pro = 0;
    }
#endif

    int start = (td->ir2_asm_nr << 2);
    int offset = 0;
    int64_t ins_offset = 0;

    int off_tb_tc_ptr = 0;
    off_tb_tc_ptr += offsetof(TranslationBlock, tc);
    off_tb_tc_ptr += offsetof(struct tb_tc, ptr);
    if (use_cc_pro && latxs_cc_pro_checktb()) {
        off_tb_tc_ptr = offsetof(TranslationBlock, cc_ok_ptr);
    }

    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;    /* $4   a0/v0 */
    IR2_OPND *zero = &latxs_zero_ir2_opnd;    /* $0   zero  */
    IR2_OPND *env  = &latxs_env_ir2_opnd;     /* $23  s0    */
                                                      
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;    /* $4   a0/v0 */
                                                      
    IR2_OPND tb = latxs_ra_alloc_dbt_arg1();  /* $10  a6    */

    IR2_OPND tmp  = latxs_ra_alloc_itemp();
    IR2_OPND tmp0 = latxs_ra_alloc_itemp();
    IR2_OPND tmp1 = latxs_ra_alloc_itemp();

    IR2_OPND label_next_tb_exist = latxs_ir2_opnd_new_label();

    /*
     * $a6: previous TB
     *
     * $a0/v0: next TB (lookup result)       @ret0
     *         > valid after TB lookup
     */

    /* ======== 0. Private Buffer ========  */
    tr_gen_pb_lookup(&tb, env, &tmp, &tmp0, &tmp1, use_cc_pro);

    /* ======== 1. NJC Lookup TB ======== */
    IR2_OPND njc_miss = latxs_ir2_opnd_new_label();
    if (intb_njc_enabled()) {
        /* 1.1 */
        latxs_load_addr_to_ir2(&tmp, GET_SC_TABLE(rid, intb_njc));
        latxs_append_ir2_opnd1_(lisa_call, &tmp);
        latxs_append_ir2_opnd3(LISA_BEQ, ret0, zero,
                                         &njc_miss);

        /* 1.2 check if PC == TB.PC */
        latxs_append_ir2_opnd2i(LISA_LOAD_PC, &tmp0, ret0,
                offsetof(TranslationBlock, pc));
        IR2_OPND eip = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_LOAD_PC, &eip, &latxs_env_ir2_opnd,
                                lsenv_offset_of_eip(lsenv));
        latxs_append_ir2_opnd3(LISA_BNE, &tmp0, &eip, &njc_miss);
        latxs_ra_free_temp(&eip);

        /* 1.3 check next TB cflags CF_INVALID */
        latxs_append_ir2_opnd2i(LISA_LD_WU, &tmp0, ret0,
                offsetof(TranslationBlock, cflags));
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &tmp0, &tmp0, 16);
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp0, &tmp0,
                                           (CF_INVALID >> 16));
        latxs_append_ir2_opnd3(LISA_BNE, &tmp0, zero, &njc_miss);

        /* 1.4 check next TB's CSBASE */
        latxs_append_ir2_opnd2i(LISA_LOAD_PC, &tmp0, ret0,
                offsetof(TranslationBlock, cs_base));
        latxs_append_ir2_opnd2i(LISA_LOAD_PC, &tmp1, env,
                offsetof(CPUX86State, segs[R_CS].base));
        latxs_append_ir2_opnd3(LISA_BNE, &tmp0, &tmp1, &njc_miss);

        /* 1.5 check next TB's flags */
        /*
         * *flags = env->hflags |
         * (env->eflags & (IOPL_MASK | TF_MASK | RF_MASK |
         *                 VM_MASK | AC_MASK));
         */
        if (use_cc_pro && latxs_cc_pro()) {
            /*
             * cc_flags != 0 : cc_mask = ~0x0
             * cc_flags == 0 : cc_mask = ~0xe00
             */
            latxs_append_ir2_opnd2i(LISA_LD_WU, &tmp, ret0,
                    offsetof(TranslationBlock, cc_mask));
        }
        IR2_OPND eflags = latxs_ra_alloc_itemp();
        /* eflags is target_ulong, but high 32 bits is all zeros */
        latxs_append_ir2_opnd2i(LISA_LD_WU, &eflags, &latxs_env_ir2_opnd,
            lsenv_offset_of_eflags(lsenv));
        latxs_load_imm64_to_ir2(&tmp0, (IOPL_MASK | TF_MASK | RF_MASK |
                                          VM_MASK | AC_MASK));
        latxs_append_ir2_opnd3(LISA_AND, &tmp0, &eflags, &tmp0);
        latxs_ra_free_temp(&eflags);
        latxs_append_ir2_opnd2i(LISA_LD_WU, &tmp1, env,
                offsetof(CPUX86State, hflags));
        latxs_append_ir2_opnd3(LISA_OR, &tmp1, &tmp1, &tmp0);
        /* tb->flags */
        latxs_append_ir2_opnd2i(LISA_LD_WU, &tmp0, ret0,
                offsetof(TranslationBlock, flags));
        if (use_cc_pro && latxs_cc_pro()) {
            latxs_append_ir2_opnd3(LISA_AND, &tmp0, &tmp0, &tmp);
            latxs_append_ir2_opnd3(LISA_AND, &tmp1, &tmp1, &tmp);
        }
        latxs_append_ir2_opnd3(LISA_BNE, &tmp0, &tmp1, &njc_miss);

        /* 1.6 NJC lookup success, finish lookup */
        latxs_append_ir2_opnd1(LISA_B, &label_next_tb_exist);

        latxs_append_ir2_opnd1(LISA_LABEL, &njc_miss);
    }

    /* ======== 2. helper_lookup_tb ======== */

    /* 2.1 call helper lookup tb    => 3. if find TB */
    offset = (td->ir2_asm_nr << 2) - start;
    ins_offset = (GET_SC_TABLE(rid, scs_prologue) - (ADDR)code_ptr - offset) >> 2;
    latxs_append_ir2_jmp_far(ins_offset, 1);

    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
    latxs_tr_gen_call_to_helper((ADDR)helper_lookup_tb);

    offset = (td->ir2_asm_nr << 2) - start;
    ins_offset = (GET_SC_TABLE(rid, scs_epilogue) - (ADDR)code_ptr - offset) >> 2;
    latxs_append_ir2_jmp_far(ins_offset, 1);

    latxs_append_ir2_opnd3(LISA_BNE, ret0, zero, &label_next_tb_exist);

    /* label for 3.1 FastCS.nolink check */
    IR2_OPND fastcs_no_link = latxs_invalid_ir2_opnd;
    if (latxs_fastcs_is_no_link()) {
        fastcs_no_link = latxs_ir2_opnd_new_label();
        latxs_append_ir2_opnd1(LISA_LABEL, &fastcs_no_link);
    }

    /* 2.2 if TB is NULL, jump to epilogue */
    offset = (td->ir2_asm_nr << 2) - start;
    ins_offset = (GET_SC_TABLE(rid, cs_native_to_bt_ret_0) -
            (ADDR)code_ptr - offset) >> 2;
    latxs_append_ir2_jmp_far(ins_offset, 0);

    /* ======== 3. Jmp Glue Check ======== */

    latxs_append_ir2_opnd1(LISA_LABEL, &label_next_tb_exist);

    /* 3.1 FastCS.nolink check      => 2.2 if context is not equal */
    if (latxs_fastcs_is_no_link()) {
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp0, ret0,
                offsetof(TranslationBlock, fastcs_ctx));
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp1, env,
                offsetof(CPUX86State, fastcs_ctx));
        latxs_append_ir2_opnd3(LISA_BNE, &tmp0, &tmp1, &fastcs_no_link);
    }

    /* 3.2 FastCS.jmpglue check : load native context */
    if (latxs_fastcs_is_jmp_glue()) {
#if 1
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp0, ret0,
                offsetof(TranslationBlock, fastcs_ctx));
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp1, env,
                offsetof(CPUX86State, fastcs_ctx));
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp1, &tmp1, 2);
        latxs_append_ir2_opnd3(LISA_ADD_D,  &tmp1, &tmp1, &tmp0);
#else
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp1, ret0,
                offsetof(TranslationBlock, fastcs_ctx));
#endif
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp1, &tmp1, 3);

        latxs_append_ir2_opnd2i(LISA_LD_D, &tmp0, env,
                offsetof(CPUX86State, fastcs_ptr));
        latxs_append_ir2_opnd3(LISA_ADD_D, &tmp0, &tmp0, &tmp1);
        latxs_append_ir2_opnd2i(LISA_LD_D, &tmp1, &tmp0, 8);

        IR2_OPND just_go_on = latxs_ir2_opnd_new_label();
        latxs_append_ir2_opnd3(LISA_BEQ, &tmp1, zero, &just_go_on);
        latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_ra_ir2_opnd, &tmp1, 0);
        latxs_append_ir2_opnd1(LISA_LABEL, &just_go_on);
    }

    /* 3.3 FPU Rotate + SIGINT check + jump to next TB */
    if (!option_lsfpu && !option_soft_fpu) {
        IR2_OPND step_opnd = latxs_ra_alloc_dbt_arg1(); /* $10 a6 */

        if (option_intb_pb)
            latxs_append_ir2_opnd2_(lisa_mov, &tmp, &tb);

        /* 1. tb->_top_out from @tb */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp1, &tb,
                offsetof(TranslationBlock, _top_out));
        /* 2. next_tb->_top_in from @ret0 */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp0, ret0,
                offsetof(TranslationBlock, _top_in));
        /* 3. calculate top_bias, store rotate step in arg1 */
        latxs_append_ir2_opnd3(LISA_SUB_D, &step_opnd, &tmp1, &tmp0);

        /*
         * For indirect jmps, we will check step to
         * decide wehter to rotate FPU or not
         */
        IR2_OPND label_rotate = latxs_ir2_opnd_new_label();
        latxs_append_ir2_opnd3(LISA_BNE, &step_opnd, zero,
                &label_rotate);
        /* bias ==0, no need to ratate */
        /* fetch native address of next_tb to arg2 */
        latxs_append_ir2_opnd2i(LISA_LD_D, &tmp1, ret0,
                off_tb_tc_ptr);
        {
            /* SIGINT check */
            if (sigint_enabled() == 1) {
                latxs_append_ir2_opnd2i(LISA_LD_W, &tmp0, env,
                        (int32_t)offsetof(X86CPU, neg.icount_decr.u32) -
                        (int32_t)offsetof(X86CPU, env));

                int offset = (td->ir2_asm_nr << 2) - start;
                int64_t ins_offset =
                    (GET_SC_TABLE(rid, cs_native_to_bt_ret_0) - (ADDR)code_ptr - offset) >>
                    2;
                latxs_append_ir2_opnd2i(LISA_BLT, &tmp0, zero, ins_offset);
            }
        }

        /* insert into private buffer */
        tr_gen_pb_insert(&tmp1, env, &tmp, ret0, &tmp0, use_cc_pro);

        latxs_append_ir2_opnd2i(LISA_JIRL, zero, &tmp1, 0);
        latxs_append_ir2_opnd1(LISA_LABEL, &label_rotate);

        /* top_bias != 0, need to rotate, step is in arg1 */
        /* fetch native address of next_tb to ra */
        latxs_append_ir2_opnd2i(LISA_LD_D, &latxs_ra_ir2_opnd, ret0,
                off_tb_tc_ptr);
        offset = (lsenv->tr_data->ir2_asm_nr << 2) - start;

        int64_t ins_offset =
            (GET_SC_TABLE(rid, fpu_rotate) - (ADDR)code_ptr - offset) >> 2;
        latxs_append_ir2_jmp_far(ins_offset, 0);
    }

    /* 3.4 SIGINT check interrupt */
    if (option_lsfpu || option_soft_fpu) {
        IR2_OPND sigint_label;
        IR2_OPND sigint_check_label_start;
        IR2_OPND sigint_check_label_end;
        if (sigint_enabled() == 1) {
            sigint_check_label_start = latxs_ir2_opnd_new_label();
            sigint_check_label_end   = latxs_ir2_opnd_new_label();

            sigint_label = latxs_ir2_opnd_new_label();
            latxs_append_ir2_opnd2i(LISA_LD_W, &tmp, env,
                    (int32_t)offsetof(X86CPU, neg.icount_decr.u32) -
                    (int32_t)offsetof(X86CPU, env));

            latxs_append_ir2_opnd1(LISA_LABEL, &sigint_check_label_start);
            latxs_append_ir2_opnd3(LISA_BLT, &tmp, zero,
                    &sigint_label);
        }

        /* 3.5 jump to next TB (with LSFPU or SoftFPU enabled) */
        latxs_append_ir2_opnd2i(LISA_LD_D, &tmp, ret0,
                off_tb_tc_ptr);

        /* insert into private buffer */
        tr_gen_pb_insert(&tmp, env, &tb, ret0, &tmp0, use_cc_pro);

        latxs_append_ir2_opnd2i(LISA_JIRL, zero, &tmp, 0);

        if (sigint_enabled() == 1) {
            latxs_append_ir2_opnd1(LISA_LABEL, &sigint_check_label_end);
            latxs_append_ir2_opnd1(LISA_LABEL, &sigint_label);

            offset = (td->ir2_asm_nr << 2) - start;
            int64_t ins_offset =
                (GET_SC_TABLE(rid, cs_native_to_bt_ret_0) - (ADDR)code_ptr - offset) >>
                2;
            latxs_append_ir2_jmp_far(ins_offset, 0);

            /* calculate offset from start to end for sigint check */
            latxs_sigint_prepare_check_intb_lookup(
                    sigint_check_label_start,
                    sigint_check_label_end);
        }
    }

    code_nr = latxs_tr_ir2_assemble(code_ptr);

    latxs_tr_fini();

    return code_nr;
}

#endif
