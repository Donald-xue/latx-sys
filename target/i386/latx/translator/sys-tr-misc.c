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
    latxs_register_ir1(X86_INS_LIDT);
    latxs_register_ir1(X86_INS_SIDT);
    latxs_register_ir1(X86_INS_LGDT);
    latxs_register_ir1(X86_INS_SGDT);
    latxs_register_ir1(X86_INS_LLDT);
    latxs_register_ir1(X86_INS_SLDT);
    latxs_register_ir1(X86_INS_LTR);
    latxs_register_ir1(X86_INS_STR);
    latxs_register_ir1(X86_INS_PUSH);
    latxs_register_ir1(X86_INS_POP);
    latxs_register_ir1(X86_INS_RET);
    latxs_register_ir1(X86_INS_CPUID);
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

bool latxs_translate_lidt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LIDT(pir1);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid LIDT insn: pir1 = %p\n", (void *)pir1);

    /*
     * In i386: always load 6 bytes
     *  >  ir1_opnd_size(opnd) == 6, not a regular load operation
     *
     * 2 bytes for limit, and limit is always 16-bits long
     *
     * 4 bytes for base address, and base assress is 24-bits long
     * in real-address mode and vm86 mode, 32-bits long in PE mode.
     */
    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    IR2_OPND limit = latxs_ra_alloc_itemp();
    IR2_OPND base  = latxs_ra_alloc_itemp();

    int save_temp = 1;

    /* 1. load 2 bytes for limit at MEM(addr)*/
    gen_ldst_softmmu_helper(LISA_LD_HU, &limit, &mem_opnd, save_temp);

    /* 2. load 4 bytes for base address at MEM(addr + 2)*/
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
    gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);

    /* 3. 24-bits long base address in Real-Address mode and vm86 mode */
    if (!td->sys.pe || td->sys.vm86) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND tmp1 = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &tmp, &base, 0x10);
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0xff);
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp, &tmp, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &tmp1, &base);
        latxs_append_ir2_opnd3(LISA_OR, &tmp, &tmp, &tmp1);
        base = tmp;
    }

    /* 4. store limit/base into IDTR */
    latxs_append_ir2_opnd2i(LISA_ST_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_base(lsenv));
    latxs_append_ir2_opnd2i(LISA_ST_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_limit(lsenv));

    return true;
}

bool latxs_translate_sidt(IR1_INST *pir1)
{
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid SIDT insn: pir1 = %p\n", (void *)pir1);

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    /* 1. load limit/base into temp register */
    IR2_OPND base = latxs_ra_alloc_itemp();
    IR2_OPND limit = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_LD_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_base(lsenv));
    latxs_append_ir2_opnd2i(LISA_LD_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_idtr_limit(lsenv));

    int save_temp = 1;
    /* 2. store 2 bytes limit at MEM(addr) */
    gen_ldst_softmmu_helper(LISA_ST_H, &limit, &mem_opnd, save_temp);

    /* 3. store 4 bytes base at MEM(addr + 2) */
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
    if (latxs_ir1_addr_size(pir1) == 2) {
        latxs_append_ir2_opnd2_(lisa_mov24z, &base, &base);
    }
    gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);

    latxs_ra_free_temp(&base);
    latxs_ra_free_temp(&limit);
    return true;
}

bool latxs_translate_lgdt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LGDT(pir1);

    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid LGDT insn: pir1 = %p\n", (void *)pir1);

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    IR2_OPND limit = latxs_ra_alloc_itemp();
    IR2_OPND base = latxs_ra_alloc_itemp();

    int save_temp = 1;

    /* 1. load 2 bytes for limit at MEM(addr)*/
    gen_ldst_softmmu_helper(LISA_LD_HU, &limit, &mem_opnd, save_temp);

    /* 2. load 4 bytes for base address at MEM(addr + 2)*/
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
    gen_ldst_softmmu_helper(LISA_LD_WU, &base, &mem, save_temp);

    /* 3. 24-bits long base address in Real-Address mode and vm86 mode */
    if (!td->sys.pe || td->sys.vm86) {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        IR2_OPND tmp1 = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_SRAI_D, &tmp, &base, 0x10);
        latxs_append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0xff);
        latxs_append_ir2_opnd2i(LISA_SLLI_D, &tmp, &tmp, 0x10);
        latxs_append_ir2_opnd2_(lisa_mov16z, &tmp1, &base);
        latxs_append_ir2_opnd3(LISA_OR, &tmp, &tmp, &tmp1);
        base = tmp;
    }

    /* 4. store limit/base into GDTR */
    latxs_append_ir2_opnd2i(LISA_ST_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_base(lsenv));
    latxs_append_ir2_opnd2i(LISA_ST_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_limit(lsenv));

    return true;
}

bool latxs_translate_sgdt(IR1_INST *pir1)
{
    IR1_OPND *opnd = ir1_get_opnd(pir1, 0);
    lsassertm_illop(ir1_addr(pir1),
            ir1_opnd_is_mem(opnd),
            "not a valid SGDT insn: pir1 = %p\n", (void *)pir1);

    IR2_OPND mem_opnd;
    latxs_convert_mem_opnd(&mem_opnd, opnd, -1);

    /* 0. mem_opnd might be temp register */
    int save_temp = 1;

    /* 1. load gdtr.limit from env */
    IR2_OPND limit = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &limit, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_limit(lsenv));
    /* 2. store 16-bits limit at MEM(addr) */
    gen_ldst_softmmu_helper(LISA_ST_H, &limit, &mem_opnd, save_temp);
    latxs_ra_free_temp(&limit);

    /* 3. load gdtr.base  from env */
    IR2_OPND base = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &base, &latxs_env_ir2_opnd,
            lsenv_offset_of_gdtr_base(lsenv));

    /* 4. store 32-bit base at MEM(addr + 2) */
    IR2_OPND mem = latxs_convert_mem_ir2_opnd_plus_2(&mem_opnd);
    gen_ldst_softmmu_helper(LISA_ST_W, &base, &mem, save_temp);
    latxs_ra_free_temp(&base);

    return true;
}

static void latxs_translate_lldt_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lldt(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    latxs_load_ir1_gpr_to_ir2(&latxs_arg1_ir2_opnd, opnd0, EXMode_Z);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_lldt);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void latxs_translate_lldt_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* 0. load selector value */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&selector, opnd0, EXMode_Z, false, -1);

    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_lldt(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg1_ir2_opnd,
            &selector, &latxs_zero_ir2_opnd);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_lldt);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

bool latxs_translate_lldt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LLDT(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_lldt_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LLDT insn: pir1 = %p\n", (void *)pir1);

        latxs_translate_lldt_mem(pir1, opnd0);
    }

    return true;
}

bool latxs_translate_sldt(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_SLDT(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    /* load ldtr.selector from env */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &selector, &latxs_env_ir2_opnd,
            lsenv_offset_of_ldtr_selector(lsenv));

    latxs_store_ir2_to_ir1(&selector, opnd0, false);

    return true;
}

static void latxs_translate_ltr_gpr(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_ltr(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    latxs_load_ir1_gpr_to_ir2(&latxs_arg1_ir2_opnd, opnd0, EXMode_Z);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_ltr);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

static void latxs_translate_ltr_mem(IR1_INST *pir1, IR1_OPND *opnd0)
{
    /* helper might cause exception, save complete CPUX86State */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* void helper_ltr(CPUX86State  *env, int selector) */

    /* 1. get selector into arg1 (a1) */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_load_ir1_mem_to_ir2(&selector, opnd0, EXMode_Z, false, -1);
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg1_ir2_opnd,
            &selector, &latxs_zero_ir2_opnd);

    /* 2. call helper */
    latxs_append_ir2_opnd3(LISA_OR, &latxs_arg0_ir2_opnd,
            &latxs_env_ir2_opnd, &latxs_zero_ir2_opnd);
    latxs_tr_gen_call_to_helper((ADDR)helper_ltr);

    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);
}

bool latxs_translate_ltr(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_LTR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0); /* GPR or MEM */

    if (ir1_opnd_is_gpr(opnd0)) {
        latxs_translate_ltr_gpr(pir1, opnd0);
    } else {
        lsassertm_illop(ir1_addr(pir1),
                ir1_opnd_is_mem(opnd0),
                "not a valid LTR insn: pir1 = %p\n", (void *)pir1);
        latxs_translate_ltr_mem(pir1, opnd0);
    }

    return true;
}

bool latxs_translate_str(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_STR(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    /* load tr.selector from env */
    IR2_OPND selector = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_LD_W, &selector, &latxs_env_ir2_opnd,
            lsenv_offset_of_tr_selector(lsenv));

    latxs_store_ir2_to_ir1(&selector, opnd0, false);

    return true;
}

/*
 * push/pop
 *
 * In system-mode, for precise exception, the effect of instruction
 * should be executed at the end of this instruction's translated code.
 *
 * For pop, these two things need to be done at the end:
 *  1> save the read value into destination
 *  2> update esp register
 *
 * For push, since the write is done by softmmu, and the exception can
 * only be generated by the softmmu helper, only one thing need to be
 * done at the end:
 *  1> update esp register
 */

/* End of TB in system-mode : pop es/ss/ds */
bool latxs_translate_pop(IR1_INST *pir1)
{
    /*
     * pop
     * ----------------------
     * >  dest <= MEM(SS:ESP)
     * >  ESP  <= ESP + 2/4
     * ----------------------
     * 1. tmp  <= MEM(SS:ESP) : softmmu
     * 2. ESP  <= ESP + 2/4
     * 3. tmp  => dest : gpr/mem/seg
     */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    TRANSLATION_DATA *td = lsenv->tr_data;

    bool is_gpr_esp = 0;
    bool is_mem_esp = 0;
    bool ss32 = lsenv->tr_data->sys.ss32;

    if (ir1_opnd_is_gpr(opnd0)) {
        if (ir1_opnd_base_reg_num(opnd0) == esp_index) {
            is_gpr_esp = 1;
        }
    }

    if (ir1_opnd_is_mem(opnd0)) {
        if (ir1_opnd_base_reg_num(opnd0) == esp_index) {
            is_mem_esp = 1;
        }
    }

    int data_size = latxs_ir1_data_size(pir1);
    int esp_inc   = data_size >> 3;

    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    if (!ir1_opnd_is_seg(opnd0)) {
        lsassert(data_size == opnd_size);
    }

    lsassert(data_size >= opnd_size);

    /*
     * TODO
    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
        ir1_opnd_num(pir1) == 1 && !is_gpr_esp && data_size == 32) {
        IR2_OPND esp_mem_opnd;
        IR2_OPND esp = ra_alloc_gpr(esp_index);
        ir2_opnd_build2(&esp_mem_opnd, IR2_OPND_MEM, esp._reg_num, 0);
        IR2_OPND dest_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        gen_ldst_softmmu_helper(
            ir1_opnd_default_em(opnd0) == SIGN_EXTENSION ? mips_lw : mips_lwu,
            &dest_reg, &esp_mem_opnd, 0);
        IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_inc);
        return true;
    }
    */

    /* 1.1 build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 1.2 read data from stack   : might generate exception */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_load_ir1_mem_to_ir2(&tmp, &mem_ir1_opnd, EXMode_Z,
            false, ss_addr_size);

    /* 2. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (is_gpr_esp) {
        if (ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_W,
                    &esp_opnd, &esp_opnd, esp_inc);
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
            }
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D,
                    &tmp, &esp_opnd, esp_inc);
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    }

    /* 3. write into destination  : might generate exception */
    if (is_mem_esp) {
        td->sys.popl_esp_hack = data_size >> 3;
        latxs_store_ir2_to_ir1_mem(&tmp, opnd0, false, ss_addr_size);
        td->sys.popl_esp_hack = 0;
    } else {
        latxs_store_ir2_to_ir1(&tmp, opnd0, false);
    }

    /*
     * Order is important for pop %esp
     * The %esp is increased first. Then the pop value is loaded into %esp.
     * This executing order is the same as that in TCG.
     */
    if (!is_gpr_esp) {
        if (ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_D,
                    &esp_opnd, &esp_opnd, esp_inc);
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_inc);
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    }

    return true;
}

bool latxs_translate_push(IR1_INST *pir1)
{
    /*
     * push
     * ----------------------
     * >  ESP  <= ESP - 2/4
     * >  src  => MEM(SS:ESP)
     * ----------------------
     * 1. tmp  <= source : gpr/mem/seg
     * 2. tmp  => MEM(SS:ESP - 2/4) : softmmu
     * 3. ESP  <= ESP - 2/4
     */
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);

    bool ss32 = lsenv->tr_data->sys.ss32;

    int data_size = latxs_ir1_data_size(pir1);
    int esp_dec   = 0 - (data_size >> 3);

    int opnd_size = ir1_opnd_size(opnd0);
    lsassert(opnd_size == 16 || opnd_size == 32);
    if (!ir1_opnd_is_seg(opnd0)) {
        lsassert(data_size == opnd_size);
    }

    /*
     * TODO
    if (option_by_hand && lsenv->tr_data->sys.ss32 &&
        !lsenv->tr_data->sys.addseg && ir1_opnd_is_gpr(opnd0) &&
        ir1_opnd_num(pir1) == 1 && data_size == 32) {
        IR2_OPND esp_mem_opnd;
        IR2_OPND esp = ra_alloc_gpr(esp_index);
        ir2_opnd_build2(&esp_mem_opnd, IR2_OPND_MEM, esp._reg_num, esp_dec);
        IR2_OPND src_reg = ra_alloc_gpr(ir1_opnd_base_reg_num(opnd0));
        gen_ldst_softmmu_helper(mips_sw, &src_reg, &esp_mem_opnd, 0);
        IR2_OPND esp_opnd = ra_alloc_gpr(esp_index);
        append_ir2_opnd2i(mips_addi_addrx, &esp_opnd, &esp_opnd, esp_dec);
        return true;
    }
    */

    /* 1. load source data   : might generate exception */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&tmp, opnd0, EXMode_N, false);

    /* 2.1 build MEM(SS:ESP - 2/4)*/
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, esp_dec, 0, 0);

    /* 2.2 write data into stack   : might generate exception */
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_store_ir2_to_ir1_mem(&tmp, &mem_ir1_opnd,
            false, ss_addr_size);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd, esp_dec);
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd, esp_dec);
        latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

bool latxs_translate_ret(IR1_INST *pir1)
{
    /*
     * ESP update   according to dflag(opnd size)
     * Value size   according to dflag(opnd size)
     * Address size according to aflag(addr size) : load ret addr
     *
     * code32 || (code16 && prefix_data) : dflag = 4
     * code16 || (code32 && prefix_data) : dflag = 2
     *
     * aflag = ir1_addr_size(pir1) : provided by capstone
     * code32 || (code16 && prefix_addr) : aflag = 4
     * code16 || (code32 && prefix_addr) : aflag = 2
     *
     * Usually the dflag(opnd size) stores in inst's operand.
     * But 'ret' is allowed to have zero operand. And if it has
     * one operand, it must be 'imm16' which must has 16-bit opnd size.
     * So there is no way to get the dflag from pir1 itself.
     *
     * In normal situation, the capstone will translate zero operand with
     * different opnd size into serival instruction, such as 'ins'.
     * But near 'ret' has only one instruction in capstone......
     */

    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);
    int addr_size = latxs_ir1_addr_size(pir1);
    lsassert(addr_size == 2 || addr_size == 4);

    /* 1. load ret_addr into $25 from MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);
    IR2_OPND return_addr_opnd = latxs_ra_alloc_dbt_arg2();
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_load_ir1_mem_to_ir2(&return_addr_opnd,
            &mem_ir1_opnd, EXMode_Z, false, ss_addr_size);

    /* 2. apply address size */
    if (data_size == 32 && addr_size == 2) {
        latxs_append_ir2_opnd2_(lisa_mov16z, &return_addr_opnd,
                                             &return_addr_opnd);
    }

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (pir1 != NULL && ir1_opnd_num(pir1) &&
            ir1_opnd_is_imm(ir1_get_opnd(pir1, 0))) {
        if (lsenv->tr_data->sys.ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
            }
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                ir1_opnd_uimm(ir1_get_opnd(pir1, 0)) + (data_size >> 3));
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    } else {
        if (lsenv->tr_data->sys.ss32) {
            latxs_append_ir2_opnd2i(LISA_ADDI_W, &esp_opnd, &esp_opnd,
                              (data_size >> 3));
            if (option_by_hand) {
                latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
            }
        } else {
            IR2_OPND tmp = latxs_ra_alloc_itemp();
            latxs_append_ir2_opnd2i(LISA_ADDI_D, &tmp, &esp_opnd,
                              (data_size >> 3));
            latxs_store_ir2_to_ir1_gpr(&tmp, &sp_ir1_opnd);
            latxs_ra_free_temp(&tmp);
        }
    }

    latxs_tr_generate_exit_tb(pir1, 0);

    return true;
}

bool latxs_translate_cpuid(IR1_INST *pir1)
{
    /* 0. save next instruciton's EIP to env */
    latxs_tr_gen_save_curr_eip();

    /*
     * 1. call helper_cpuid
     *
     * void helper_cpuid(
     *      CPUX86State *env)
     * >> load new EAX/ECX/EDX/EBX
     */
    helper_cfg_t cfg = default_helper_cfg;
    latxs_tr_gen_call_to_helper1_cfg((ADDR)helper_cpuid, cfg);

    return true;
}
