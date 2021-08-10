#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_misc_register_ir1(void)
{
    latxs_register_ir1(X86_INS_LJMP);
    latxs_register_ir1(X86_INS_CALL);
    latxs_register_ir1(X86_INS_JMP);
    latxs_register_ir1(X86_INS_CLI);
    latxs_register_ir1(X86_INS_STI);
}

int latxs_get_sys_stack_addr_size(void)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    if (td->sys.ss32) {
        return 4;
    } else {
        return 2;
    }
}

static void translate_jmp_far_pe_imm(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /*
     * void helper_ljmp_protected(
     *     CPUX86State     *env,
     *     int             new_cs,
     *     target_ulong    new_eip,
     *     target_ulong    next_eip)
     */
    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *arg1 = &latxs_arg1_ir2_opnd;
    IR2_OPND *arg2 = &latxs_arg2_ir2_opnd;
    IR2_OPND *arg3 = &latxs_arg3_ir2_opnd;

    int   new_cs   = 0;
    ADDRX new_eip  = 0;
    ADDRX next_eip = ir1_addr_next(pir1);

    /*
     *                  ptr16:16 ptr16:32
     * opnd[0]:selector    16       16
     * opnd[1]:offset      16       32
     */
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    int opnd1_size  = ir1_opnd_size(opnd1);

    new_cs  = ir1_opnd_simm(opnd0);
    new_eip = ir1_opnd_uimm(opnd1);

    /* 1. save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2. parameters */

    /* 2.1 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, arg0, env);

    /* 2.2 arg1: new_cs, 16-bits */
    latxs_load_imm32_to_ir2(arg1, new_cs & 0xffff, EXMode_Z);

    /* 2.3 arg2: new_eip, 16/32-bits */
    if (opnd1_size == 16) {
        latxs_load_imm32_to_ir2(arg2, new_eip & 0xffff, EXMode_Z);
    } else { /* opnd1_size == 32 */
        latxs_load_imm32_to_ir2(arg2, new_eip & 0xffffffff, EXMode_Z);
    }

    /* 2.4 arg3: next_eip, 32-bits */
    latxs_load_imm32_to_ir2(arg3, next_eip, EXMode_Z);

    /* 3. call helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_ljmp_protected);

    /* 4. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 5. context switch */
    lsenv->tr_data->ignore_eip_update = 1;
    latxs_tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_pe_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /*
     * void helper_ljmp_protected(
     *     CPUX86State     *env,
     *     int             new_cs,
     *     target_ulong    new_eip,
     *     target_ulong    next_eip)
     */

    /*
     *           m16:16     m16:32
     * offset      16         32     MEM(addr)
     * selector    16         16     MEM(addr+2/4)
     */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);

    /* 1. load offset and selector */
    IR2_OPND offset = latxs_ra_alloc_itemp();
    IR2_OPND selector = latxs_ra_alloc_itemp();

    int opnd_size = ir1_opnd_size(opnd0);

    switch (opnd_size) {
    case 32: /* m16:16 */
        gen_ldst_softmmu_helper(LISA_LD_WU, &offset, &mem_opnd, 0);
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &selector, &offset, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &offset, &offset);
        break;
    case 48: /* m16:32 */
        /* load 32-bit offset */
        gen_ldst_softmmu_helper(LISA_LD_WU, &offset, &mem_opnd, 1);
        /* memory address += 4 */
        IR2_OPND mem_opnd_4 =
            latxs_convert_mem_ir2_opnd_plus_4(&mem_opnd);
        /* load 16-bit selector */
        gen_ldst_softmmu_helper(LISA_LD_HU, &selector, &mem_opnd_4, 1);
        break;
    default:
        lsassertm_illop(ir1_addr(pir1), 0,
                "unsupported opnd size %d in ljmp mem.\n", opnd_size);
        break;
    }

    latxs_ra_free_temp(&mem_opnd);

    /* 2. save native context here */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 3. parameters */
    /* 3.1 arg1: new_cs, 16-bits */
    /* 3.2 arg2: new_eip, 16/32-bits */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &selector);
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &offset);
    /* 3.4 arg3: next_eip, 32-bits */
    ADDRX next_eip = ir1_addr_next(pir1);
    latxs_load_imm32_to_ir2(&latxs_arg3_ir2_opnd, next_eip, EXMode_Z);
    /* 3.5 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);

    /* 4. call helper */
    latxs_tr_gen_call_to_helper((ADDR)helper_ljmp_protected);

    /* 5. restore native context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 6. context switch */
    lsenv->tr_data->ignore_eip_update = 1;
    latxs_tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_real_imm(IR1_INST *pir1, IR1_OPND *opnd0)
{
    uint32_t selector;
    ADDRX base;

    IR2_OPND tmp0 = latxs_ra_alloc_itemp();
    IR2_OPND tmp1 = latxs_ra_alloc_itemp();

    /*                  ptr16:16 ptr16:32 */
    /* opnd[0]:selector    16       16    */
    /* opnd[1]:offset      16       32    */
    selector = ir1_opnd_uimm(opnd0);
    base     = selector << 0x4;

    latxs_load_imm32_to_ir2(&tmp0, selector, EXMode_Z);
    latxs_load_addrx_to_ir2(&tmp1, base);

    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp0, &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_selector(lsenv, R_CS));
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp1, &latxs_env_ir2_opnd,
            lsenv_offset_of_seg_base(lsenv, R_CS));

    latxs_tr_generate_exit_tb(pir1, 1);
}

static void translate_jmp_far_real_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* m16:16/m16:32/m16:64 */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd0, -1);

    IR2_OPND dest = latxs_ra_alloc_itemp();
    IR2_OPND next_eip = latxs_ra_alloc_itemp();

    /* m16:16 */
    if (ir1_opnd_size(opnd0) == 32) {
        IR2_OPND tmp0 = latxs_ra_alloc_itemp();

        gen_ldst_softmmu_helper(LISA_LD_W, &dest, &mem_opnd, 1);

        latxs_append_ir2_opnd2_(lisa_mov16z, &next_eip, &dest);

        latxs_append_ir2_opnd2i(LISA_SRLI_W, &tmp0, &dest, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &tmp0, &tmp0);
        latxs_append_ir2_opnd2i(LISA_ST_W, &tmp0, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_selector(lsenv, R_CS));

        latxs_append_ir2_opnd2i(LISA_SLLI_W, &tmp0, &tmp0, 0x4);
        latxs_append_ir2_opnd2i(LISA_ST_W, &tmp0, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, R_CS));

        latxs_ra_free_temp(&tmp0);
    } else if (ir1_opnd_size(opnd0) == 48) { /* m16:32 */
        /* load 32-bit offset */
        gen_ldst_softmmu_helper(LISA_LD_WU, &dest, &mem_opnd, 1);
        latxs_append_ir2_opnd2_(lisa_mov, &next_eip, &dest);

        /* load 16-bit selector */
        IR2_OPND mem_opnd_adjusted =
            latxs_convert_mem_ir2_opnd_plus_4(&mem_opnd);

        gen_ldst_softmmu_helper(LISA_LD_HU, &dest, &mem_opnd_adjusted, 1);
        latxs_append_ir2_opnd2i(LISA_ST_W, &dest, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_selector(lsenv, R_CS));

        latxs_append_ir2_opnd2i(LISA_SLLI_D, &dest, &dest, 0x4);
        latxs_append_ir2_opnd2i(LISA_ST_W, &dest, &latxs_env_ir2_opnd,
                lsenv_offset_of_seg_base(lsenv, R_CS));
    } else { /* m16:64 */
        lsassertm_illop(ir1_addr(pir1), 0,
            "unsupported opnd size %d in ljmp mem.\n", ir1_opnd_size(opnd0));
    }

    latxs_ra_free_temp(&dest);

    IR2_OPND next_eip_opnd = latxs_ra_alloc_dbt_arg2();
    latxs_append_ir2_opnd2_(lisa_mov, &next_eip_opnd, &next_eip);
    latxs_tr_generate_exit_tb(pir1, 1);
}

bool latxs_translate_jmp_far(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* protected mode && not vm86 mode */
    if (td->sys.pe && !td->sys.vm86) {
        if (ir1_opnd_is_imm(opnd0)) {
            translate_jmp_far_pe_imm(pir1, opnd0);
        } else if (ir1_opnd_is_mem(opnd0)) {
            translate_jmp_far_pe_mem(pir1, opnd0);
        } else {
            lsassert(0);
        }
        return true;
    }

    /* Real-Address mode || vm86 mode */
    if (ir1_opnd_is_imm(opnd0)) {
        translate_jmp_far_real_imm(pir1, opnd0);
    } else if (ir1_opnd_is_mem(opnd0)) {
        translate_jmp_far_real_mem(pir1, opnd0);
    }

    return true;
}

bool latxs_translate_call(IR1_INST *pir1)
{
    if (ir1_is_indirect_call(pir1)) {
        return latxs_translate_callin(pir1);
    } else if (ir1_addr_next(pir1) == ir1_target_addr(pir1)) {
        return latxs_translate_callnext(pir1);
    }

    bool ss32 = lsenv->tr_data->sys.ss32;
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = latxs_ir1_data_size(pir1);
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);

    int ss_addr_size = latxs_get_sys_stack_addr_size();

    /* 1. get return address */
    IR2_OPND return_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));

    /* 2. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    latxs_store_ir2_to_ir1_mem(&return_addr_opnd,
            &mem_ir1_opnd, false, ss_addr_size);
    latxs_ra_free_temp(&return_addr_opnd);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd,
                          0 - (opnd_size >> 3));
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    /* 4. exit TB */
    latxs_tr_generate_exit_tb(pir1, 0);

    return true;
}

bool latxs_translate_callnext(IR1_INST *pir1)
{
    bool ss32 = lsenv->tr_data->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = latxs_ir1_data_size(pir1);
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);

    int ss_addr_size = latxs_get_sys_stack_addr_size();

    /* 1. get return address */
    IR2_OPND next_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&next_addr_opnd, ir1_addr_next(pir1));

    /* 2. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size,
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    latxs_store_ir2_to_ir1_mem(&next_addr_opnd,
            &mem_ir1_opnd, false, ss_addr_size);
    latxs_ra_free_temp(&next_addr_opnd);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd,
                &esp_opnd, 0 - (opnd_size >> 3));
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &tmp,
                &esp_opnd, 0 - (opnd_size >> 3));
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

bool latxs_translate_callin(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;

    bool ss32 = td->sys.ss32;

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    int data_size = latxs_ir1_data_size(pir1);
    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    lsassert(data_size == opnd_size);

    int ss_addr_size = latxs_get_sys_stack_addr_size();

    /* 1. prepare successor x86 address */
    IR2_OPND next_eip_opnd = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&next_eip_opnd, opnd0, EXMode_Z, false);
    if (opnd_size == 16) {
        latxs_append_ir2_opnd2_(lisa_mov16z, &next_eip_opnd,
                                             &next_eip_opnd);
    }

    /* 2. get return address */
    IR2_OPND return_addr_opnd = latxs_ra_alloc_itemp();
    latxs_load_addrx_to_ir2(&return_addr_opnd, ir1_addr_next(pir1));

    /* 3. save return address at MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, opnd_size, /* 16 or 32 */
            X86_REG_SS, X86_REG_ESP, 0 - (opnd_size >> 3), 0, 0);
    latxs_store_ir2_to_ir1_mem(&return_addr_opnd,
            &mem_ir1_opnd, false, ss_addr_size);
    latxs_ra_free_temp(&return_addr_opnd);

    /* 4. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd,
                &esp_opnd, 0 - (opnd_size >> 3));
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &tmp, &esp_opnd,
                          0 - (opnd_size >> 3));
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    /* 5. go to next TB */
    IR2_OPND succ_x86_addr_opnd = latxs_ra_alloc_dbt_arg2();
    latxs_append_ir2_opnd3(LISA_OR, &succ_x86_addr_opnd,
            &latxs_zero_ir2_opnd, &next_eip_opnd);

    latxs_tr_generate_exit_tb(pir1, 0);

    return true;
}

bool latxs_translate_jmp(IR1_INST *pir1)
{
    if (ir1_is_indirect_jmp(pir1)) {
        return latxs_translate_jmpin(pir1);
    }

    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_jmpin(IR1_INST *pir1)
{
    /* 1. set successor x86 address */
    IR2_OPND next_eip = latxs_ra_alloc_dbt_arg2();
    latxs_load_ir1_to_ir2(&next_eip,
            ir1_get_opnd(pir1, 0), EXMode_Z, false);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 16) {
        latxs_append_ir2_opnd2_(lisa_mov16z, &next_eip, &next_eip);
    }

    latxs_tr_generate_exit_tb(pir1, 1);
    return true;
}

bool latxs_translate_cli(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLI(pir1);

    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /* helper_cli */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, IF_BIT);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, zero, &mask);
    latxs_append_ir2_opnd3(LISA_AND, eflags, eflags, &mask);

    latxs_ra_free_temp(&mask);

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_sti(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLI(pir1);

    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /* helper_sti */
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, IF_BIT);
    latxs_append_ir2_opnd3(LISA_OR, eflags, eflags, &mask);

    latxs_ra_free_temp(&mask);

    /* sti is EOB in system-mode */
    lsenv->tr_data->inhibit_irq = 1;

    return true;
}
