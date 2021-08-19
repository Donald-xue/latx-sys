#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

/* BPC: Break Point Codes */
ADDR latxs_sc_bpc;

/* NJC: Native Jmp Cache lookup */
ADDR latxs_sc_njc;

/* FCS: Fast Context Switch */
ADDR latxs_sc_fcs_jmp_glue_fpu_0;
ADDR latxs_sc_fcs_jmp_glue_fpu_1;
ADDR latxs_sc_fcs_jmp_glue_xmm_0;
ADDR latxs_sc_fcs_jmp_glue_xmm_1;

ADDR latxs_native_printer;

/* SCS: Static Context Switch */
ADDR latxs_sc_scs_prologue;
ADDR latxs_sc_scs_epilogue;

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

static void latxs_native_printer_helper(CPUX86State *env,
        int type, int r1, int r2, int r3, int r4, int r5)
{
    /* Up to now, only SoftTLB Compare is supported */
    lsassert(type == LATXS_NP_TLBCMP);

    /* r1: reg number of cmp_opnd */
    uint64_t addr_cmp = env->mips_regs[r1];
    /* r2: reg number of tag_opnd */
    uint64_t addr_tag = env->mips_regs[r2];
    /* r3: reg number of address */
    uint64_t addr_x86 = env->mips_regs[r3];
    /* r4: mmu index */
    int mmu_index = r4;
    /* r5: load or store */
    int is_load = r5;

    CPUState *cpu = env_cpu(env);
    X86CPU *xcpu = (X86CPU *)cpu;

    uint64_t mask = xcpu->neg.tlb.f[mmu_index].mask;
    int tlb_index = (addr_x86 >> TARGET_PAGE_BITS) &
                    (mask >> CPU_TLB_ENTRY_BITS);
    CPUTLBEntry *tlb = &xcpu->neg.tlb.f[mmu_index].table[tlb_index];

    int is_hit = addr_cmp == addr_tag;

    if (is_load) {
        fprintf(stderr,
                "load  tlb cmp %x,%x,%x mmu=%d. TLB:%x,%x,%x val=%x\n",
                addr_cmp, addr_tag, addr_x86, mmu_index,
                tlb->addr_code, tlb->addr_read, tlb->addr_write,
                is_hit ? *((uint32_t *)((uint32_t)(addr_x86) + tlb->addend)) :
                         -1);
        if (is_hit) {
            fprintf(stderr, "tlb %p fast hit addend %x\n", tlb, tlb->addend);
        }
    } else {
        fprintf(stderr,
                "store tlb cmp %x,%x,%x mmu=%d. TLB:%x,%x,%x val=%x\n",
                addr_cmp, addr_tag, addr_x86, mmu_index,
                tlb->addr_code, tlb->addr_read, tlb->addr_write,
                is_hit ? *((uint32_t *)((uint32_t)(addr_x86) + tlb->addend)) :
                        -1);
    }
}

static int gen_latxs_native_printer(void *code_ptr)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;

    IR2_OPND reg = latxs_zero_ir2_opnd;

    int i = 0;

    latxs_tr_init(NULL);

    /* save all native registers */
    for (i = 1; i < 32; ++i) {
        reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i);
        latxs_append_ir2_opnd2i(LISA_ST_D, &reg, env,
                lsenv_offset_of_mips_regs(lsenv, i));
    }

    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
    latxs_tr_gen_call_to_helper((ADDR)latxs_native_printer_helper);

    /* read all native registers */
    reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i);
    for (i = 1; i < 32; ++i) {
        reg = latxs_ir2_opnd_new(IR2_OPND_GPR, i);
        latxs_append_ir2_opnd2i(LISA_LD_D, &reg, env,
                lsenv_offset_of_mips_regs(lsenv, i));
    }
    latxs_append_ir2_opnd0_(lisa_return);

    i = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return i;
}

static int __gen_latxs_jmp_glue(void *code_ptr, int n)
{
    int code_nr = 0;

    latxs_tr_init(NULL);

    TRANSLATION_DATA *td = lsenv->tr_data;

    int start = (td->real_ir2_inst_num << 2);
    int offset = 0;

    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *env  = &latxs_env_ir2_opnd;

    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *ra = &latxs_ra_ir2_opnd;

    IR2_OPND tb  = latxs_ra_alloc_dbt_arg1(); /* $10 a6 */
    IR2_OPND eip = latxs_ra_alloc_dbt_arg2(); /* $11 a7 */

    IR2_OPND tmp    = latxs_ra_alloc_itemp();
    IR2_OPND param0 = latxs_ra_alloc_itemp();
    IR2_OPND param1 = latxs_ra_alloc_itemp();

    /*
     * arguments passed to native rotate FPU
     * DBT arg 1 : FPU rotate step
     * DBT arg 2 : next TB's native codes
     */
    IR2_OPND step_opnd = latxs_ra_alloc_dbt_arg1();
    IR2_OPND tb_nc_opnd = latxs_ra_alloc_dbt_arg2();

    if (n == 0 || n == 1) {
        /* load tb->next_tb[n] into a0/v0 */
        latxs_append_ir2_opnd2i(LISA_LD_D, ret0, &tb,
                offsetof(TranslationBlock, next_tb) +
                n * sizeof(void *));
    } else {
        /* Indirect jmp lookup TB */

        IR2_OPND label_next_tb_exist = latxs_ir2_opnd_new_label();

        /*
         * $a6: prev TB           @tb         @step_opnd
         * $a7: EIP for next TB   @eip
         *
         * tmp: free to use here  @param0
         * tmp: free to use here  @param1
         * tmp: free to use here  @tmp
         *      > used to verify next TB
         *      > used to record monitor data
         *
         * $a0/v0: next TB (lookup result)       @ret0
         *         > valid after TB lookup
         */

        /* TODO : NJC Lookup TB */

        /* LL: helper_lookup_tb */
        if (option_lsfpu) {
            latxs_tr_gen_save_curr_top();
            latxs_tr_fpu_disable_top_mode();
        }
        latxs_tr_save_registers_to_env(0xff, 0xff, 0, 0xff, 0xff, 0x3);
        latxs_tr_save_eflags();

        latxs_load_addr_to_ir2(&tmp, (ADDR)helper_lookup_tb);
        latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
        latxs_append_ir2_opnd2i(LISA_JIRL, ra, &tmp, 0);

        latxs_tr_load_registers_from_env(0xff, 0xff, 1, 0xff, 0xff, 0x3);
        latxs_append_ir2_opnd3(LISA_BNE, ret0, zero, &label_next_tb_exist);

        /* if next_tb == NULL, jump to epilogue */
        latxs_append_ir2_opnd2i(LISA_LD_WU, &eip, env,
                lsenv_offset_of_eip(lsenv));
        offset = (td->real_ir2_inst_num << 2) - start;
        latxs_append_ir2_opnda(LISA_B,
                (context_switch_native_to_bt_ret_0
                 - (ADDR)code_ptr - offset) >> 2);

        /* else compare tb->top_out and next_tb->top_in */
        latxs_append_ir2_opnd1(LISA_LABEL, &label_next_tb_exist);
    }

    if (!option_lsfpu) {
        /* 1. tb->_top_out from @tb */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &param1, &tb,
                offsetof(TranslationBlock, _top_out));
        /* 2. next_tb->_top_in from @ret0 */
        latxs_append_ir2_opnd2i(LISA_LD_BU, &param0, ret0,
                offsetof(TranslationBlock, _top_in));
        /* 3. calculate top_bias, store rotate step in arg1 */
        latxs_append_ir2_opnd3(LISA_SUB_D, &step_opnd, &param1, &param0);

        /*
         * For direct jmps, if no need to rotate,
         * we will make direct link without this glue
         *
         * For indirect jmps, we will check step to
         * decide wehter to rotate FPU or not
         */
        if (n == 2) {
            IR2_OPND label_rotate = latxs_ir2_opnd_new_label();
            latxs_append_ir2_opnd3(LISA_BNE, &step_opnd, zero,
                    &label_rotate);
            /* bias ==0, no need to ratate */
            /* fetch native address of next_tb to arg2 */
            latxs_append_ir2_opnd2i(LISA_LD_D, &tmp, ret0,
                    offsetof(TranslationBlock, tc) +
                    offsetof(struct tb_tc, ptr));
            latxs_append_ir2_opnd2i(LISA_JIRL, zero, &tmp, 0);
            latxs_append_ir2_opnd1(LISA_LABEL, &label_rotate);
        }

        /* top_bias != 0, need to rotate, step is in arg1 */
        /* fetch native address of next_tb to arg2 */
        latxs_append_ir2_opnd2i(LISA_LD_D, &tb_nc_opnd, ret0,
                offsetof(TranslationBlock, tc) +
                offsetof(struct tb_tc, ptr));
        offset = (lsenv->tr_data->real_ir2_inst_num << 2) - start;
        latxs_append_ir2_opnda(LISA_B, (native_rotate_fpu_by
                 - (ADDR)code_ptr - offset) >> 2);
    } else {
        /* With LSFPU enabled, we could jmp to next TB directly  */
        latxs_append_ir2_opnd2i(LISA_LD_D, &tmp, ret0,
                offsetof(TranslationBlock, tc) +
                offsetof(struct tb_tc, ptr));
        latxs_append_ir2_opnd2i(LISA_JIRL, zero, &tmp, 0);
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

    native_jmp_glue_0 = (ADDR)code_ptr;
    code_nr = __gen_latxs_jmp_glue(code_ptr, 0);
    code_nr_all += code_nr;
    code_ptr += code_nr << 2;
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs TBLink : native jmp glue 0 at %p\n",
            (void *)native_jmp_glue_0);

    native_jmp_glue_1 = (ADDR)code_ptr;
    code_nr = __gen_latxs_jmp_glue(code_ptr, 1);
    code_nr_all += code_nr;
    code_ptr += code_nr << 2;
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs TBLink : native jmp glue 1 at %p\n",
            (void *)native_jmp_glue_1);

    native_jmp_glue_2 = (ADDR)code_ptr;
    code_nr = __gen_latxs_jmp_glue(code_ptr, 2);
    code_nr_all += code_nr;
    code_ptr += code_nr << 2;
    LATXS_DUMP_STATIC_CODES_INFO(
            "latxs TBLink : native jmp glue 2 at %p\n",
            (void *)native_jmp_glue_2);

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

    if (scs_enabled()) {
        latxs_sc_scs_prologue = (ADDR)code_ptr;
        LATXS_GEN_STATIC_CODES(gen_latxs_scs_prologue_cfg,
                code_ptr, default_helper_cfg);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs SCS: static CS prologue %p\n",
                (void *)latxs_sc_scs_prologue);

        latxs_sc_scs_epilogue = (ADDR)code_ptr;
        LATXS_GEN_STATIC_CODES(gen_latxs_scs_epilogue_cfg,
                code_ptr, default_helper_cfg);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs SCS: static CS epilogue %p\n",
                (void *)latxs_sc_scs_epilogue);
    }

    /* do something */
    if (option_native_printer) {
        latxs_native_printer = (ADDR)code_ptr;
        LATXS_GEN_STATIC_CODES(gen_latxs_native_printer, code_ptr);
        LATXS_DUMP_STATIC_CODES_INFO(
                "latxs do something %p\n",
                (void *)latxs_native_printer);
    }

    /* jmp glue for tb-link */
    if (option_tb_link) {
        LATXS_GEN_STATIC_CODES(gen_latxs_jmp_glue_all, code_ptr);
    }

    return code_nr_all;
}
