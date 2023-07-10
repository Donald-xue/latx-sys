#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "latx-config.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "latxs-fastcs-cfg.h"
#include "latx-perfmap.h"
#include "latx-bpc-sys.h"
#include "latx-np-sys.h"
#include "latx-sigint-sys.h"
#include "latx-intb-sys.h"
#include "latx-cross-page.h"
#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

latx_sc_table_t *latx_sc_table;
static latx_sc_table_t __latx_sc_table[LATX_REGION_N];
static void __attribute__((__constructor__)) latxs_sc_init(void)
{
    latx_sc_table = __latx_sc_table;
}



#define LATXS_DUMP_STATIC_CODES_INFO(str, ...) do {    \
    if (option_dump) {                                  \
        fprintf(stderr, str, __VA_ARGS__);              \
    }                                                   \
} while (0)

/*
 * When generating prologue/epilogue in system-mode,
 * the vCPU thread is not created yet.
 *
 * The lsenv is initialized at the begining of vCPU thread.
 *
 * But here we need the lsenv to generate some static
 * native codes. So we use a temp lsenv.
 */

static ENV latxs_lsenv_tmp;
static TRANSLATION_DATA latxs_tr_data_tmp;
static CPUX86State latxs_env_tmp;

static void latxs_set_lsenv_tmp(void)
{
    memset(&latxs_lsenv_tmp, 0, sizeof(ENV));
    memset(&latxs_tr_data_tmp, 0, sizeof(TRANSLATION_DATA));
    memset(&latxs_env_tmp, 0, sizeof(CPUX86State));

    lsenv = &latxs_lsenv_tmp;
    lsenv->cpu_state = &latxs_env_tmp;
    lsenv->tr_data = &latxs_tr_data_tmp;
}

static int gen_latxs_sc_prologue(void *code_ptr)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *sp = &latxs_sp_ir2_opnd;
    IR2_OPND *fp = &latxs_fp_ir2_opnd;
    IR2_OPND *ra = &latxs_ra_ir2_opnd;
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;
    IR2_OPND *fcsr1 = &latxs_fcsr1_ir2_opnd; /* enable */
    IR2_OPND *fcsr2 = &latxs_fcsr2_ir2_opnd; /* Flags Cause */
    IR2_OPND *fcsr3 = &latxs_fcsr3_ir2_opnd; /* RM */

    TRANSLATION_DATA *td = lsenv->tr_data;

    int i = 0;
    const int extra_space = 40;

    latxs_tr_init(NULL);

    /* 1. save DBT's context in stack, so allocate space on the stack */
    latxs_append_ir2_opnd2i(LISA_ADDI_D, sp, sp, -256);

    /* 1.1 save callee-saved registers. s0-s7 ($23-$31) */
    IR2_OPND reg = latxs_ir2_opnd_new_inv();
    for (i = 0; i <= 8; ++i) {
        reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i + 23);
        latxs_append_ir2_opnd2i(LISA_ST_D, &reg, sp, extra_space + (i << 3));
    }

    /* 1.2 save fp($22), and ra($1) */
    latxs_append_ir2_opnd2i(LISA_ST_D, fp, sp, extra_space + 72);
    latxs_append_ir2_opnd2i(LISA_ST_D, ra, sp, extra_space + 80);

    /* 1.3. save DBT FCSR (#31) */
    IR2_OPND fcsr_value = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &fcsr_value, fcsr);
    latxs_append_ir2_opnd2i(LISA_ST_D, &fcsr_value, sp, extra_space + 88);

    /* 2. set up native context */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_env_ir2_opnd, arg1, zero);

    /* print context switch type */
    latxs_np_tr_cs_prologue();

    /* 2.1 set native code FCSR (#31) */
#if defined(LATX_SYS_FCSR)
#if defined(FASTCS_INCLUDE_FCSR)
    IR2_OPND label_no_fastcs = latxs_ir2_opnd_new_label();
    if (latxs_fastcs_enabled()) {
        IR2_OPND fastcsctx = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_LD_BU, &fastcsctx,
                &latxs_env_ir2_opnd,
                offsetof(CPUX86State, fastcs_ctx));
        latxs_append_ir2_opnd3(LISA_BEQ, &fastcsctx,
                &latxs_zero_ir2_opnd,
                &label_no_fastcs);
        latxs_ra_free_temp(&fastcsctx);
    }
#endif
    latxs_append_ir2_opnd2i(LISA_LD_W, &fcsr_value,
            &latxs_env_ir2_opnd, lsenv_offset_of_fcsr(lsenv));
    /* set RM */
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr3, &fcsr_value);
    /* disable exception TODO support fpu exception */
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr1, zero);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr2, zero);
#if defined(FASTCS_INCLUDE_FCSR)
    if (latxs_fastcs_enabled()) {
        latxs_append_ir2_opnd1(LISA_LABEL, &label_no_fastcs);
    }
#endif
#else /* no LATX_SYS_FCSR */
    latxs_append_ir2_opnd3(LISA_OR, &fcsr_value, &fcsr_value, zero);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &fcsr_value);
#endif

    /* 2.2 set f3 = 32 */
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&temp, 32, EXMode_S);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &latxs_f32_ir2_opnd, &temp);

    /* 2.3 load x86 mapping registers */
    int load_top = !option_soft_fpu;
    if (latxs_fastcs_enabled()) {
        latxs_fastcs_load_registers(0xffffffff, 0xff, load_top,
                                    0xffffffff, 0x00);
    } else {
        latxs_tr_load_registers_from_env(0xffffffff, 0xff, load_top,
                                         0xffffffff, 0x00);
    }
    latxs_tr_load_eflags();

    IR2_OPND sigint_label = latxs_ir2_opnd_new_label();
    if (sigint_enabled() == 1) {
        sigint_label = latxs_ir2_opnd_new_label();

        latxs_append_ir2_opnd2i(LISA_LD_W, &temp,
                &latxs_env_ir2_opnd,
                (int32_t)offsetof(X86CPU, neg.icount_decr.u32) -
                (int32_t)offsetof(X86CPU, env));
        latxs_append_ir2_opnd3(LISA_BLT, &temp, &latxs_zero_ir2_opnd,
                &sigint_label);
        latxs_append_ir2_opnd0_(lisa_nop);
    }

    if (sigint_enabled() == 1) {
        /* 3.0 set sigint_flag in ENV to 0 */
        latxs_append_ir2_opnd2i(LISA_ST_W, zero,
                &latxs_env_ir2_opnd,
                offsetof(CPUX86State, sigint_flag));
    }

    /* 3. jump to native code address (saved in a0) */
    latxs_append_ir2_opnd2i(LISA_JIRL, zero, arg0, 0);

    latxs_append_ir2_opnd0_(lisa_nop);
    if (sigint_enabled() == 1) {
        latxs_append_ir2_opnd1(LISA_LABEL, &sigint_label);

        latxs_append_ir2_opnd2i(LISA_ORI, &latxs_ret0_ir2_opnd, zero,
                TB_EXIT_REQUESTED);

        IR2_OPND tb_ptr_opnd = latxs_ra_alloc_dbt_arg1();

        latxs_append_ir2_opnd2_(lisa_mov, &tb_ptr_opnd, zero);

        int inst_size = td->ir2_asm_nr << 2;
        /*
         * native_to_bt -> 0x00 : xxxx
         *                 0x04 : xxxx
         *                 0x08 : xxxx
         * bt_to_native -> 0x0c : xxxx
         *                 0x10 : xxxx # inst_num = 2
         *                 0x14 : xxxx <- LISA_B
         *
         * offset of branch   = 0x00 - 0x0c - 0x8  = 0xffec = -20 = - 0x14
         * branch destiantion = 0x14 - 0x14 = 0x00
         */

        int rid = lsenv->tr_data->region_id;
#ifndef LATX_USE_MULTI_REGION
        lsassert(rid == 0);
#endif
        int64_t ins_offset = (GET_SC_TABLE(rid, cs_native_to_bt) -
                              GET_SC_TABLE(rid, cs_bt_to_native) - inst_size) >>
                              2;
        latxs_append_ir2_jmp_far(ins_offset, 1);
        latxs_append_ir2_opnd0_(lisa_nop);
    }

    i = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return i;
}

static int gen_latxs_sc_epilogue(void *code_ptr)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *sp = &latxs_sp_ir2_opnd;
    IR2_OPND *fp = &latxs_fp_ir2_opnd;
    IR2_OPND *ra = &latxs_ra_ir2_opnd;
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;
    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;

    int i = 0;
    const int extra_space = 40;

    latxs_tr_init(NULL);

    /*
     * by default set v0 to zero.
     * sc_table.cs_native_to_bt_ret_0 points here
     */
    latxs_append_ir2_opnd3(LISA_OR, ret0, zero, zero);

    if (sigint_enabled() == 1) {
        /* 5.0 set sigint_flag in ENV to 1 */
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, zero, 1);
        latxs_append_ir2_opnd2i(LISA_ST_W, &tmp,
                &latxs_env_ir2_opnd,
                offsetof(CPUX86State, sigint_flag));
        latxs_ra_free_temp(&tmp);
    }

    /* 1. store the last executed TB ($10) into env */
    IR2_OPND tb_ptr_opnd = latxs_ra_alloc_dbt_arg1();
    latxs_append_ir2_opnd2i(LISA_ST_D, &tb_ptr_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_last_executed_tb(lsenv));

#if defined(LATX_SYS_FCSR)
#if defined(FASTCS_INCLUDE_FCSR)
    IR2_OPND label_no_fastcs = latxs_ir2_opnd_new_label();
    if (latxs_fastcs_enabled()) {
        IR2_OPND fastcsctx = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_LD_BU, &fastcsctx,
                &latxs_env_ir2_opnd,
                offsetof(CPUX86State, fastcs_ctx));
        latxs_append_ir2_opnd3(LISA_BEQ, &fastcsctx,
                &latxs_zero_ir2_opnd,
                &label_no_fastcs);
        latxs_ra_free_temp(&fastcsctx);
    }
#endif
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &tmp, fcsr);
#if defined(FCSR_SAVE_RM_ONLY)
    latxs_append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0x300);
#endif
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp,
            &latxs_env_ir2_opnd, lsenv_offset_of_fcsr(lsenv));
    latxs_ra_free_temp(&tmp);
#if defined(FASTCS_INCLUDE_FCSR)
    if (latxs_fastcs_enabled()) {
        latxs_append_ir2_opnd1(LISA_LABEL, &label_no_fastcs);
    }
#endif
#endif

    /* 3. save x86 mapping registers */
    int save_top = (option_lsfpu && !option_soft_fpu) ? 1 : 0;
    if (latxs_fastcs_enabled()) {
        latxs_fastcs_save_registers(0xffffffff, 0xff, save_top,
                                    0xffffffff, 0x00);
    } else {
        latxs_tr_save_registers_to_env(0xffffffff, 0xff, save_top,
                                       0xffffffff, 0x00);
    }
    latxs_tr_save_eflags();

    /* print context switch type */
    latxs_np_tr_cs_epilogue();

    /* 4. restore bt's context */
    /* 4.1. restore DBT FCSR (#31) */
    IR2_OPND fcsr_value = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_D, &fcsr_value, sp, extra_space + 88);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &fcsr_value);
    /* 4.2 restore fp($22) and ra($1) */
    latxs_append_ir2_opnd2i(LISA_LD_D, ra, sp, extra_space + 80);
    latxs_append_ir2_opnd2i(LISA_LD_D, fp, sp, extra_space + 72);
    /* 4.3 restore callee-saved registers. s0-s7 ($16-$23) */
    IR2_OPND reg = latxs_ir2_opnd_new_inv();
    for (i = 0; i <= 8; ++i) {
        reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i + 23);
        latxs_append_ir2_opnd2i(LISA_LD_D, &reg, sp, extra_space + (i << 3));
    }
    /* 4.4 restore sp */
    latxs_append_ir2_opnd2i(LISA_ADDI_D, sp, sp, 256);

    /* 5 return */
    latxs_append_ir2_opnd2i(LISA_JIRL, zero, ra, 0);

    i = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return i;
}

static int __gen_fpu_rotate_step(void *code_base,
        ADDR *rotate_step_array)
{
    int code_nr = 0;
    int code_nr_all = 0;
    void *code_ptr = code_base;

    int i = 0;
    int step = 0;

//    TRANSLATION_DATA *td = lsenv->tr_data;

    IR2_OPND *env  = &latxs_env_ir2_opnd;  /* $23 s0   */
//    IR2_OPND *zero = &latxs_zero_ir2_opnd; /* $0  zero */

    for (step = 1; step <= 7; ++step) {
        latxs_tr_init(NULL);

//        int start = (td->ir2_asm_nr << 2);






        /* 2.1 load top_bias early. It will be modified later */
        IR2_OPND top_bias = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_LD_WU, &top_bias,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_top_bias(lsenv));

        /* 2.2 prepare for the rotation */
        IR2_OPND fpr[8];
        for (i = 0; i < 8; ++i) {
            fpr[i] = latxs_ra_alloc_st(i);
        }

        /* 2.3 rotate! */
        IR2_OPND spilled_data = latxs_ra_alloc_ftemp();
        int spilled_index = 0;
        int number_of_moved_fpr = 0;
        while (number_of_moved_fpr < 8) {
            /* 2.3.1 spill out a register */
            latxs_append_ir2_opnd2(LISA_FMOV_D,
                    &spilled_data, &fpr[spilled_index]);
            /* 2.3.2 rotate, until moving from the spilled register */
            int target_index = spilled_index;
            int source_index = (target_index + step) & 7;
            while (source_index != spilled_index) {
                latxs_append_ir2_opnd2(LISA_FMOV_D,
                        &fpr[target_index], &fpr[source_index]);
                number_of_moved_fpr++;
                target_index = source_index;
                source_index = (target_index + step) & 7;
            };
            /* 2.3.3 move from the spilled data */
            latxs_append_ir2_opnd2(LISA_FMOV_D,
                    &fpr[target_index], &spilled_data);
            number_of_moved_fpr++;
            /* 2.3.4 when step is 2, 4, or 6, rotate from the next index; */
            spilled_index++;
        }

        /* 1.4 adjust the top_bias */
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &top_bias, &top_bias, step);
        latxs_append_ir2_opnd2i(LISA_ANDI,   &top_bias, &top_bias, 0x7);
        latxs_append_ir2_opnd2i(LISA_ST_W,   &top_bias, env,
                lsenv_offset_of_top_bias(lsenv));

        /* TODO  SIGINT check */
//      if (sigint_enabled() == 1) {
//          IR2_OPND tmp = latxs_ra_alloc_itemp();
//          latxs_append_ir2_opnd2i(LISA_LD_W, &tmp, env,
//                  (int32_t)offsetof(X86CPU, neg.icount_decr.u32) -
//                  (int32_t)offsetof(X86CPU, env));

//          int offset = (td->ir2_asm_nr << 2) - start;
//          int64_t ins_offset =
//              (context_switch_native_to_bt_ret_0 - (ADDR)code_ptr - offset) >>
//              2;
//          latxs_append_ir2_opnd2i(LISA_BLT, &tmp, zero, ins_offset);
//      }

        /* 1.5 jump to next TB's native code, saved in ra by jump glue */
        latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_zero_ir2_opnd,
                &latxs_ra_ir2_opnd, 0);





        rotate_step_array[step] = (ADDR)code_ptr;
        rotate_step_array[step - 8] = (ADDR)code_ptr;
        code_nr = latxs_tr_ir2_assemble((void *)rotate_step_array[step - 8]);
        code_nr += 1;

        latxs_tr_fini();

        if (option_dump) {
            fprintf(stderr,
                    "[fpu rotate] rotate step(%d,%d) at %p, size = %d\n",
                    step, step - 8, code_ptr, code_nr);
        }

        code_ptr += (code_nr << 2);
        code_nr_all += code_nr;
    }

    return code_nr_all;
}

static int __gen_fpu_rotate_dispatch(void *code_ptr,
        ADDR *rotate_step_array)
{
    int code_nr = 0;

    latxs_tr_init(NULL);

    IR2_OPND rotate_fpu = latxs_ra_alloc_itemp();
    latxs_load_addr_to_ir2(&rotate_fpu, (ADDR)(rotate_step_array));

    IR2_OPND rotate_step = latxs_ra_alloc_dbt_arg1();

    latxs_append_ir2_opnd2i(LISA_SLLI_W, &rotate_step, &rotate_step, 3);
    latxs_append_ir2_opnd3(LISA_ADD_D, &rotate_fpu, &rotate_fpu, &rotate_step);
    latxs_append_ir2_opnd2i(LISA_LD_D, &rotate_fpu, &rotate_fpu, 0);
    latxs_append_ir2_opnd2i(LISA_JIRL, &zero_ir2_opnd, &rotate_fpu, 0);

    code_nr = latxs_tr_ir2_assemble((void *)code_ptr);
    code_nr += 1;

    latxs_tr_fini();

    return code_nr;
}

static int gen_latxs_sc_fpu_rotate(void *code_base)
{
    int code_nr = 0;
    int code_nr_all = 0;
    void *code_ptr = code_base;

    int rid = lsenv->tr_data->region_id;
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    /*
     * rotate -7 ~ 7
     *
     * array: [0][1] ... [6][7][8] ... [13][14]
     * step : -7 -6      -1  0  1        6   7
     */
    static ADDR fpu_rotate_by_step_array[15];
    ADDR *rotate_step_array = &fpu_rotate_by_step_array[7];
    rotate_step_array[0] = 0;
    code_nr = __gen_fpu_rotate_step(code_ptr, rotate_step_array);
    code_nr_all += code_nr;
    code_ptr = code_base + (code_nr_all << 2);

    SET_SC_TABLE(rid, fpu_rotate, (ADDR)code_ptr);
    code_nr = __gen_fpu_rotate_dispatch(code_ptr, rotate_step_array);
    code_nr_all += code_nr;
    code_ptr = code_base + (code_nr_all << 2);

    return code_nr_all;
}

/* direct jmp glue for FPU rotate */
static int __gen_latxs_jmp_glue(void *code_ptr, int n)
{
    int code_nr = 0;

    latxs_tr_init(NULL);

    TRANSLATION_DATA *td = lsenv->tr_data;
    int rid = td->region_id;
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    int start = (td->ir2_asm_nr << 2);
    int offset = 0;

    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;

    IR2_OPND tb  = latxs_ra_alloc_dbt_arg1();

    IR2_OPND tmp0 = latxs_ra_alloc_itemp();
    IR2_OPND tmp1 = latxs_ra_alloc_itemp();

    lsassert(n == 0 || n == 1);

    /* load tb->next_tb[n] into a0/v0 */
    latxs_append_ir2_opnd2i(LISA_LD_D, ret0, &tb,
            offsetof(TranslationBlock, next_tb) +
            n * sizeof(void *));

    if (!option_lsfpu && !option_soft_fpu) {
        IR2_OPND step_opnd = latxs_ra_alloc_dbt_arg1(); /* $10 a6 */

        /* 1. tb->_top_out from @tb */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp1, &tb,
                offsetof(TranslationBlock, _top_out));
        /* 2. next_tb->_top_in from @ret0 */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &tmp0, ret0,
                offsetof(TranslationBlock, _top_in));
        /* 3. calculate top_bias, store rotate step in arg1 */
        latxs_append_ir2_opnd3(LISA_SUB_D, &step_opnd, &tmp1, &tmp0);

        /*
         * For direct jmps, if no need to rotate,
         * we will make direct link without this glue
         */

        /* top_bias != 0, need to rotate, step is in arg1 */
        /* fetch native address of next_tb to ra */
        latxs_append_ir2_opnd2i(LISA_LD_D, &latxs_ra_ir2_opnd, ret0,
                offsetof(TranslationBlock, tc) +
                offsetof(struct tb_tc, ptr));
        offset = (lsenv->tr_data->ir2_asm_nr << 2) - start;

        int64_t ins_offset =
            (GET_SC_TABLE(rid, fpu_rotate) - (ADDR)code_ptr - offset) >> 2;
        latxs_append_ir2_jmp_far(ins_offset, 0);
    }

    code_nr = latxs_tr_ir2_assemble(code_ptr);

    latxs_tr_fini();

    return code_nr;
}

static int gen_latxs_jmp_glue_all(void *code_base)
{
    int code_nr = 0;
    int code_nr_all = 0;
    void *code_ptr = code_base;

    int region_id = lsenv->tr_data->region_id;

    SET_SC_TABLE(region_id, jmp_glue_0, (ADDR)code_ptr);
    code_nr = __gen_latxs_jmp_glue(code_ptr, 0);
    code_nr_all += code_nr;
    code_ptr += code_nr << 2;
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs TBLink : native jmp glue 0 at %p\n",
            (void *)GET_SC_TABLE(region_id, jmp_glue_0));

    SET_SC_TABLE(region_id, jmp_glue_1, (ADDR)code_ptr);
    code_nr = __gen_latxs_jmp_glue(code_ptr, 1);
    code_nr_all += code_nr;
    code_ptr += code_nr << 2;
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs TBLink : native jmp glue 1 at %p\n",
            (void *)GET_SC_TABLE(region_id, jmp_glue_1));

    return code_nr_all;
}

int target_latxs_static_codes(void *code_base, int region_id)
{
    int code_nr = 0;
    int code_nr_all = 0;
    void *code_ptr = code_base;

    latxs_set_lsenv_tmp();
#ifdef LATX_USE_MULTI_REGION
    if (latx_multi_region_enable()) {
        lsenv->tr_data->region_id = region_id;
    } else {
        if (region_id != LATX_REGION_ID_DISABLE) {
            /* only use one region */
            return 0;
        }
    }
#else
    lsassert(region_id == 0);
    lsenv->tr_data->region_id = 0;
#endif

#define LATXS_GEN_STATIC_CODES(name, genfn, ...) do {   \
    code_nr = genfn(__VA_ARGS__);                       \
    code_nr_all += code_nr;                             \
    latx_perfmap_insert(code_ptr, code_nr << 2, \
            region_id ? name "_R1" : name "_R0");  \
    code_ptr = code_base + (code_nr_all << 2);          \
} while (0)

#ifdef LATXS_NP_ENABLE
    /* print something in native codes */
    if (latxs_np_enabled()) {
        SET_SC_TABLE(region_id, nprint, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_native_printer",
                gen_latxs_native_printer, code_ptr);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs do something %p\n",
                (void *)GET_SC_TABLE(region_id, nprint));
    }
#endif

    /* epilogue */
    SET_SC_TABLE(region_id, cs_native_to_bt_ret_0, (ADDR)code_ptr);
    SET_SC_TABLE(region_id, cs_native_to_bt, (ADDR)code_ptr + 4);
    LATXS_GEN_STATIC_CODES("latxs_epilogue",
            gen_latxs_sc_epilogue, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO("latxs epilogue: %p\n",
            (void *)GET_SC_TABLE(region_id, cs_native_to_bt));

    /* prologue */
    /*
     * For signal interrupt optimization, we need to check interrupt
     * pending before jump to TB's native codes. If there is an
     * interrupt pending, we should jump to epilogue to return
     * back to BT context. So we need to know the address of
     * context switch native to bt here.
     */
    SET_SC_TABLE(region_id, cs_bt_to_native, (ADDR)code_ptr);
    LATXS_GEN_STATIC_CODES("latxs_prologue",
            gen_latxs_sc_prologue, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO("latxs prologue: %p\n",
            (void *)GET_SC_TABLE(region_id, cs_bt_to_native));

    if (!option_soft_fpu) {
        /* fpu rorate */
        LATXS_GEN_STATIC_CODES("latxs_rotate_fpu",
                gen_latxs_sc_fpu_rotate, code_ptr);
        LATXS_DUMP_STATIC_CODES_INFO("latxs fpu rotate: %p\n",
                (void *)GET_SC_TABLE(region_id, fpu_rotate));
    }

#ifdef LATX_BPC_ENABLE
    /* BPC: Break Point Code */
    SET_SC_TABLE(region_id, bpc, (ADDR)code_ptr);
    LATXS_GEN_STATIC_CODES("latxs_break_point_code",
            gen_latxs_sc_bpc, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs BPC: break point code %p\n",
            (void *)GET_SC_TABLE(region_id, bpc));
#endif

    if (scs_enabled()) {
        SET_SC_TABLE(region_id, scs_prologue, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_static_prologue",
                gen_latxs_scs_prologue_cfg, code_ptr, default_helper_cfg);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs SCS: static CS prologue %p\n",
                (void *)GET_SC_TABLE(region_id, scs_prologue));

        SET_SC_TABLE(region_id, scs_epilogue, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_static_epilogue",
                gen_latxs_scs_epilogue_cfg, code_ptr, default_helper_cfg);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs SCS: static CS epilogue %p\n",
                (void *)GET_SC_TABLE(region_id, scs_epilogue));
    }

    if (latxs_fastcs_enabled())
    {

        if (latxs_fastcs_is_jmp_glue() &&
            !latxs_fastcs_is_jmp_glue_direct()) {
        SET_SC_TABLE(region_id, fcs_F_0, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_fpu0",
                gen_latxs_sc_fcs_jmp_glue, code_ptr, 0x1, 0);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS FPU 0: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_F_0));

        SET_SC_TABLE(region_id, fcs_F_1, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_fpu1",
                gen_latxs_sc_fcs_jmp_glue, code_ptr, 0x1, 1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS FPU 1: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_F_1));

        SET_SC_TABLE(region_id, fcs_S_0, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_simd0",
                gen_latxs_sc_fcs_jmp_glue, code_ptr, 0x2, 0);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS SIMD 0: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_S_0));

        SET_SC_TABLE(region_id, fcs_S_1, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_simd1",
                gen_latxs_sc_fcs_jmp_glue, code_ptr, 0x2, 1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS SIMD 1: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_S_1));

        SET_SC_TABLE(region_id, fcs_FS_0, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_both0",
                gen_latxs_sc_fcs_jmp_glue, code_ptr, 0x3, 0);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS FPU/SIMD 0: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_FS_0));

        SET_SC_TABLE(region_id, fcs_FS_1, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_both1",
                gen_latxs_sc_fcs_jmp_glue, code_ptr, 0x3, 1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS FPU/SIMD 1: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_FS_1));
        }

        if (latxs_fastcs_is_jmp_glue_direct()) {
        SET_SC_TABLE(region_id, fcs_F_0, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_fpu0_ret",
                gen_latxs_sc_fcs_jmp_glue_return, code_ptr, 0x1, 0);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS Ret FPU 0: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_F_0));

        SET_SC_TABLE(region_id, fcs_F_1, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_fpu1_ret",
                gen_latxs_sc_fcs_jmp_glue_return, code_ptr, 0x1, 1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS Ret FPU 1: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_F_1));

        SET_SC_TABLE(region_id, fcs_S_0, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_simd0_ret",
                gen_latxs_sc_fcs_jmp_glue_return, code_ptr, 0x2, 0);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS Ret SIMD 0: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_S_0));

        SET_SC_TABLE(region_id, fcs_S_1, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_simd1_ret",
                gen_latxs_sc_fcs_jmp_glue_return, code_ptr, 0x2, 1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS Ret SIMD 1: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_S_1));

        SET_SC_TABLE(region_id, fcs_FS_0, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_both0_ret",
                gen_latxs_sc_fcs_jmp_glue_return, code_ptr, 0x3, 0);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS Ret FPU/SIMD 0: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_FS_0));

        SET_SC_TABLE(region_id, fcs_FS_1, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_both1_ret",
                gen_latxs_sc_fcs_jmp_glue_return, code_ptr, 0x3, 1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS Ret FPU/SIMD 1: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_FS_1));
        }


        if (latxs_fastcs_is_jmp_glue() ||
            latxs_fastcs_is_ld_branch()) {
        SET_SC_TABLE(region_id, fcs_check_load_F, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_fpu_check",
                gen_latxs_sc_fcs_check_load, code_ptr, 0x1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS check load FPU: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_check_load_F));

        SET_SC_TABLE(region_id, fcs_check_load_S, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_simd_check",
                gen_latxs_sc_fcs_check_load, code_ptr, 0x2);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS check load SIMD: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_check_load_S));

        SET_SC_TABLE(region_id, fcs_check_load_FS, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_both_check",
                gen_latxs_sc_fcs_check_load, code_ptr, 0x3);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS check load FPU/SIMD: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_check_load_FS));

        SET_SC_TABLE(region_id, fcs_load_F, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_fpu_load",
                gen_latxs_sc_fcs_load, code_ptr, 0x1);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS load FPU: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_load_F));

        SET_SC_TABLE(region_id, fcs_load_S, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_simd_load",
                gen_latxs_sc_fcs_load, code_ptr, 0x2);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS load SIMD: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_load_S));

        SET_SC_TABLE(region_id, fcs_load_FS, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_fcs_both_load",
                gen_latxs_sc_fcs_load, code_ptr, 0x3);
        LATXS_DUMP_STATIC_CODES_INFO("latxs FastCS load FPU/SIMD: %p\n",
                (void *)GET_SC_TABLE(region_id, fcs_load_FS));
        }
    }

    /* Native Jmp Cache Lookup */
    if (intb_njc_enabled()) {
        SET_SC_TABLE(region_id, intb_njc, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_intb_njc",
                gen_latxs_intb_njc_lookup, code_ptr);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs NJC : native jmp cache lookup %p\n",
                (void *)GET_SC_TABLE(region_id, intb_njc));
    }

    /* cross-page jmp check */
    SET_SC_TABLE(region_id, jmp_glue_cpc_0, (ADDR)code_ptr);
    LATXS_GEN_STATIC_CODES("latxs_cpc_0",
            gen_latxs_jmp_glue_cpc, code_ptr, 0);
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs CPC: jmp glue cpc 0 %p\n",
            (void *)GET_SC_TABLE(region_id, jmp_glue_cpc_0));

    SET_SC_TABLE(region_id, jmp_glue_cpc_1, (ADDR)code_ptr);
    LATXS_GEN_STATIC_CODES("latxs_cpc_1",
            gen_latxs_jmp_glue_cpc, code_ptr, 1);
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs CPC: jmp glue cpc 1 %p\n",
            (void *)GET_SC_TABLE(region_id, jmp_glue_cpc_1));

    /* jmp glue for tb-link */
    if (option_tb_link) {
        LATXS_GEN_STATIC_CODES("latxs_jmp_glue",
                gen_latxs_jmp_glue_all, code_ptr);
    }

    /* indirect branch lookup */
#ifdef LATXS_INTB_LINK_ENABLE
    if (intb_link_enable()) {
        SET_SC_TABLE(region_id, intb_lookup, (ADDR)code_ptr);
        LATXS_GEN_STATIC_CODES("latxs_intb_lookup",
                gen_latxs_intb_lookup, code_ptr);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs indirect TB link at %p\n",
                (void *)GET_SC_TABLE(region_id, intb_lookup));
    }
#endif

    latx_perfmap_flush();

    latx_multi_region_save(region_id);

    return code_nr_all;
}
