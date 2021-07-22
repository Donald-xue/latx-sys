#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

/* BPC: Break Point Codes */
ADDR latxs_sc_bpc;

/* HPSCS: HelPer Static Context Swith */
ADDR latxs_sc_hpscs_pro; /* save context before helper */
ADDR latxs_sc_hpscs_epi; /* load context after  helper */

/* NJC: Native Jmp Cache lookup */
ADDR latxs_sc_njc;

/* FCS: Fast Context Switch */
ADDR latxs_sc_fcs_jmp_glue_fpu_0;
ADDR latxs_sc_fcs_jmp_glue_fpu_1;
ADDR latxs_sc_fcs_jmp_glue_xmm_0;
ADDR latxs_sc_fcs_jmp_glue_xmm_1;

/*
 * For other static codes, we use the same entry with LATX-user.
 *
 * Defined in translator/translate.c
 *
 * ADDR context_switch_bt_to_native;
 * ADDR context_switch_native_to_bt_ret_0;
 * ADDR context_switch_native_to_bt;
 *
 * ADDR native_rotate_fpu_by;
 *
 * ADDR native_jmp_glue_0;
 * ADDR native_jmp_glue_1;
 * ADDR native_jmp_glue_2;
 */

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

static int gen_latxs_sc_bpc(void *code_ptr)
{
    int code_nr = 0;
    latxs_tr_init(NULL);

    latxs_append_ir2_opnd0_(lisa_nop);
    latxs_append_ir2_opnd0_(lisa_nop);
    latxs_append_ir2_opnd0_(lisa_return);

    code_nr = latxs_tr_ir2_assemble(code_ptr);

    latxs_tr_fini();

    return code_nr;
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

    /* 2.1 set native code FCSR (#31) */
    latxs_append_ir2_opnd3(LISA_OR, &fcsr_value, &fcsr_value, zero);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &fcsr_value);

    /* 2.2 set f3 = 32 */
    IR2_OPND temp = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&temp, 32, EXMode_S);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &latxs_f32_ir2_opnd, &temp);

    /* 2.3 load x86 mapping registers */
    latxs_tr_load_registers_from_env(0xff, 0xff, 1, 0xff, 0xff, 0x00);
    latxs_tr_load_eflags(1);

    /* 3. jump to native code address (saved in a0) */
    latxs_append_ir2_opnd2i(LISA_JIRL, zero, arg0, 0);

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
     * context_switch_native_to_bt_ret_0 points here
     */
    latxs_append_ir2_opnd3(LISA_OR, ret0, zero, zero);

    /* 1. store the last executed TB ($10) into env */
    IR2_OPND tb_ptr_opnd = latxs_ra_alloc_dbt_arg1();
    latxs_append_ir2_opnd2i(LISA_ST_D, &tb_ptr_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_last_executed_tb(lsenv));

    /* 2. store eip (in $11) into env */
    IR2_OPND eip_opnd = latxs_ra_alloc_dbt_arg2();
    latxs_append_ir2_opnd2i(LISA_ST_W, &eip_opnd,
            &latxs_env_ir2_opnd,
            lsenv_offset_of_eip(lsenv));

    /* 3. save x86 mapping registers */
    int save_top = option_lsfpu ? 1 : 0;
    latxs_tr_save_registers_to_env(0xff, 0xff, save_top, 0xff, 0xff, 0x00);
    latxs_tr_save_eflags();

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

    for (step = 1; step <= 7; ++step) {
        latxs_tr_init(NULL);

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

        /* 1.5 jump to next TB's native code */
        IR2_OPND target_native_code_addr = latxs_ra_alloc_dbt_arg2();
        latxs_append_ir2_opnd2i(LISA_ST_W, &top_bias,
                &latxs_env_ir2_opnd,
                lsenv_offset_of_top_bias(lsenv));
        latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_zero_ir2_opnd,
                &target_native_code_addr, 0);

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

    native_rotate_fpu_by = (ADDR)code_ptr;
    code_nr = __gen_fpu_rotate_dispatch(code_ptr, rotate_step_array);
    code_nr_all += code_nr;
    code_ptr = code_base + (code_nr_all << 2);

    return code_nr_all;
}

int target_latxs_static_codes(void *code_base)
{
    int code_nr = 0;
    int code_nr_all = 0;
    void *code_ptr = code_base;

    latxs_set_lsenv_tmp();

#define LATXS_GEN_STATIC_CODES(genfn, ...) do {     \
    code_nr = genfn(__VA_ARGS__);                   \
    code_nr_all += code_nr;                         \
    code_ptr = code_base + (code_nr_all << 2);      \
} while (0)

    /* prologue */
    context_switch_bt_to_native = (ADDR)code_ptr;
    LATXS_GEN_STATIC_CODES(gen_latxs_sc_prologue, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO("latxs prologue: %p\n",
            (void *)context_switch_bt_to_native);

    /* epilogue */
    context_switch_native_to_bt_ret_0 = (ADDR)code_ptr;
    context_switch_native_to_bt = (ADDR)code_ptr + 4;
    LATXS_GEN_STATIC_CODES(gen_latxs_sc_epilogue, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO("latxs epilogue: %p\n",
            (void *)context_switch_native_to_bt);

    /* fpu rorate */
    LATXS_GEN_STATIC_CODES(gen_latxs_sc_fpu_rotate, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO("latxs fpu rotate: %p\n",
            (void *)native_rotate_fpu_by);

    /* BPC: Break Point Code */
    latxs_sc_bpc = (ADDR)code_ptr;
    LATXS_GEN_STATIC_CODES(gen_latxs_sc_bpc, code_ptr);
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs BPC: break point code %p\n",
            (void *)latxs_sc_bpc);

    return code_nr_all;
}
