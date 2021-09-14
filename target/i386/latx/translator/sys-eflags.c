#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_eflags_register_ir1(void)
{
    latxs_register_ir1(X86_INS_CLD);
    latxs_register_ir1(X86_INS_STD);
    latxs_register_ir1(X86_INS_CLC);
    latxs_register_ir1(X86_INS_STC);
    latxs_register_ir1(X86_INS_CMC);
    latxs_register_ir1(X86_INS_CLAC);
    latxs_register_ir1(X86_INS_STAC);
    latxs_register_ir1(X86_INS_PUSHF);
    latxs_register_ir1(X86_INS_PUSHFD);
    latxs_register_ir1(X86_INS_POPF);
    latxs_register_ir1(X86_INS_POPFD);

    latxs_register_ir1(X86_INS_CLTS);

    latxs_register_ir1(X86_INS_SAHF);
    latxs_register_ir1(X86_INS_LAHF);
}

/*
 * always set CPUX86State.cc_src == eflags
 *
 * Only called in :
 * 1. context switch native to bt
 * 2. call to helper prologue
 */
void latxs_tr_save_eflags(void)
{
    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    /* 1. get full eflags */

    /* 1.1 Clear the OF/SF/ZF/AF/PF/CF of eflags */
    IR2_OPND cc_mask = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &cc_mask, zero, 0x8d5);
    latxs_append_ir2_opnd2_(lisa_not, &cc_mask, &cc_mask);
    latxs_append_ir2_opnd3(LISA_AND, eflags, eflags, &cc_mask);
    latxs_ra_free_temp(&cc_mask);

    /* 1.2 Read  OF/SF/ZF/AF/PF/CF from CPU */
    IR2_OPND eflags_temp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &eflags_temp, zero);
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &eflags_temp, 0x3f);

    /* 1.3 Set the OF/SF/ZF/AF/PF/CF of eflags */
    latxs_append_ir2_opnd3(LISA_OR, eflags, eflags, &eflags_temp);
    latxs_ra_free_temp(&eflags_temp);

    /*
     * 2. save eflags into env
     *    env->cc_src : OF/SF/ZF/AF/PF/CF
     *    env->eflags : every thing else except DF
     *    env->df     : if DF: env->df = 1 else env->df = -1
     *    env->cc_op  : CC_OP_EFLAGS = 1
     */
    IR2_OPND cc = latxs_ra_alloc_itemp();
    cc_mask = latxs_ra_alloc_itemp();

    /* 2.1 build mask for OF/SF/ZF/AF/PF/CF */
    latxs_append_ir2_opnd2i(LISA_ORI, &cc_mask, zero, 0x8d5);

    /* 2.2 get them */
    latxs_append_ir2_opnd3(LISA_AND, &cc, eflags, &cc_mask);

    /* 2.3 store them into cc_src */
    latxs_append_ir2_opnd2i(LISA_ST_W, &cc, env,
            lsenv_offset_of_cc_src(lsenv));

    /* 2.4 get other flags : reuse cc_mask */
    latxs_append_ir2_opnd2i(LISA_ORI, &cc_mask, &cc_mask, DF_BIT);
    latxs_append_ir2_opnd3(LISA_NOR, &cc_mask, zero, &cc_mask);
    latxs_append_ir2_opnd3(LISA_AND, &cc_mask, eflags, &cc_mask);

    /* 2.5 store into eflags */
    latxs_append_ir2_opnd2i(LISA_ST_W, &cc_mask, env,
            lsenv_offset_of_eflags(lsenv));

    /* 2.6 set cc_op : reuse cc_mask */
    latxs_append_ir2_opnd2i(LISA_ORI, &cc_mask, zero, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &cc_mask, env,
            lsenv_offset_of_cc_op(lsenv));

    /* 2.7 set env->df according to eflags: reuse cc and cc_mask */
    latxs_append_ir2_opnd2i(LISA_ANDI, &cc_mask, eflags, DF_BIT);

    IR2_OPND df_label = latxs_ir2_opnd_new_label();
    IR2_OPND exit_label = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, &cc_mask, zero, &df_label);
    /* not branch: df = 1 , set env->df = -1 */
    latxs_append_ir2_opnd2i(LISA_ADDI_D, &cc, zero, -1);
    latxs_append_ir2_opnd1(LISA_B, &exit_label);
    /* branch: df = 0 , set env->df = 1 */
    latxs_append_ir2_opnd1(LISA_LABEL, &df_label);
    latxs_append_ir2_opnd2i(LISA_ORI, &cc, zero, 1);
    /* update env->df */
    latxs_append_ir2_opnd1(LISA_LABEL, &exit_label);
    latxs_append_ir2_opnd2i(LISA_ST_W, &cc, env,
            lsenv_offset_of_df(lsenv));

    latxs_ra_free_temp(&cc);
    latxs_ra_free_temp(&cc_mask);
}

/*
 * if (simple) : simply read the env->eflags
 * else        : read through helper_read_eflags
 *
 * Only called in :
 * 1. context switch bt to native : simple = 1
 * 2. call to helper epilogue     : simple = 0
 */
void latxs_tr_load_eflags(int simple)
{
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;
    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    if (simple) {
        latxs_append_ir2_opnd2i(LISA_LD_W, eflags, env,
                lsenv_offset_of_eflags(lsenv));
    } else {
        /* 1. save return value */
        latxs_append_ir2_opnd2i(LISA_ST_D, ret0, env,
                lsenv_offset_of_mips_regs(lsenv,
                    latxs_ir2_opnd_reg(ret0)));
        /* 2. read eflags via helper_read_eflags */
        latxs_append_ir2_opnd2_(lisa_mov, arg0, env);

        IR2_OPND read_eflags_func = latxs_ra_alloc_itemp();
        latxs_load_addr_to_ir2(&read_eflags_func, (ADDR)helper_read_eflags);
        latxs_append_ir2_opnd1_(lisa_call, &read_eflags_func);
        latxs_ra_free_temp(&read_eflags_func);

        latxs_append_ir2_opnd2_(lisa_mov, eflags, ret0);
        /* 3. restore return value */
        latxs_append_ir2_opnd2i(LISA_LD_D, ret0, env,
                lsenv_offset_of_mips_regs(lsenv,
                    latxs_ir2_opnd_reg(ret0)));
    }

    /* Move OF/SF/ZF/AF/PF/CF into CPU */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, eflags, 0x3f);

    /* Clear OF/SF/ZF/AF/PF/CF of elfgas */
    IR2_OPND cc_mask = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&cc_mask, 0x8d5, EXMode_Z);
    latxs_append_ir2_opnd2_(lisa_not, &cc_mask, &cc_mask);
    latxs_append_ir2_opnd3(LISA_AND, eflags, eflags, &cc_mask);
    latxs_ra_free_temp(&cc_mask);
}

void latxs_tr_gen_static_save_eflags(void)
{
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;
    IR2_OPND *stmp2 = &latxs_stmp2_ir2_opnd;

    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    /* 1. get eflags */
    /* Clear the OF/SF/ZF/AF/PF/CF of eflags */
    latxs_append_ir2_opnd2i(LISA_ORI, stmp1, zero, 0x8d5);
    latxs_append_ir2_opnd3(LISA_NOR, stmp1, zero, stmp1);
    latxs_append_ir2_opnd3(LISA_AND, eflags, eflags, stmp1);
    /* Read  OF/SF/ZF/AF/PF/CF from CPU */
    latxs_append_ir2_opnd2_(lisa_mov, stmp1, zero);
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, stmp1, 0x3f);
    /* Set the OF/SF/ZF/AF/PF/CF of eflags */
    latxs_append_ir2_opnd3(LISA_OR, eflags, eflags, stmp1);

    /*
     * 2. save eflags into env
     *    env->cc_src : OF/SF/ZF/AF/PF/CF
     *    env->eflags : every thing else except DF
     *    env->df     : if DF: env->df = 1 else env->df = -1
     *    env->cc_op  : CC_OP_EFLAGS = 1
     */
    IR2_OPND *cc = stmp1;
    IR2_OPND *cc_mask = stmp2;

    /* 2.1 build mask for OF/SF/ZF/AF/PF/CF */
    latxs_append_ir2_opnd2i(LISA_ORI, cc_mask, zero, 0x8d5);

    /* 2.2 get them */
    latxs_append_ir2_opnd3(LISA_AND, cc, eflags, cc_mask);

    /* 2.3 store them into cc_src */
    latxs_append_ir2_opnd2i(LISA_ST_W, cc, env,
            lsenv_offset_of_cc_src(lsenv));

    /* 2.4 get other flags : reuse cc_mask */
    latxs_append_ir2_opnd2i(LISA_ORI, cc_mask, cc_mask, DF_BIT);
    latxs_append_ir2_opnd3(LISA_NOR, cc_mask, zero, cc_mask);
    latxs_append_ir2_opnd3(LISA_AND, cc_mask, eflags, cc_mask);

    /* 2.5 store into eflags */
    latxs_append_ir2_opnd2i(LISA_ST_W, cc_mask, env,
            lsenv_offset_of_eflags(lsenv));

    /* 2.6 set cc_op : reuse cc_mask */
    latxs_append_ir2_opnd2i(LISA_ORI, cc_mask, zero, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, cc_mask, env,
            lsenv_offset_of_cc_op(lsenv));

    /* 2.7 set env->df according to eflags: reuse cc and cc_mask */
    latxs_append_ir2_opnd2i(LISA_ANDI, cc_mask, eflags, DF_BIT);

    IR2_OPND df_label = latxs_ir2_opnd_new_label();
    IR2_OPND exit_label = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3(LISA_BEQ, cc_mask, zero, &df_label);
    /* not branch: df = 1 , set env->df = -1 */
    latxs_append_ir2_opnd2i(LISA_ADDI_D, cc, zero, -1);
    latxs_append_ir2_opnd1(LISA_B, &exit_label);
    /* branch: df = 0 , set env->df = 1 */
    latxs_append_ir2_opnd1(LISA_LABEL, &df_label);
    latxs_append_ir2_opnd2i(LISA_ORI, cc, zero, 1);
    /* update env->df */
    latxs_append_ir2_opnd1(LISA_LABEL, &exit_label);
    latxs_append_ir2_opnd2i(LISA_ST_W, cc, env,
            lsenv_offset_of_df(lsenv));
}

void latxs_tr_gen_static_load_eflags(int simple)
{
    IR2_OPND *stmp1 = &latxs_stmp1_ir2_opnd;

    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *arg0 = &latxs_arg0_ir2_opnd;
    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;

    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND *env = &latxs_env_ir2_opnd;

    if (simple) {
        latxs_append_ir2_opnd2i(LISA_LD_W, eflags, env,
                lsenv_offset_of_eflags(lsenv));
    } else {
        int ret_reg = latxs_ir2_opnd_reg(ret0);

        /* 1. save return value */
        latxs_append_ir2_opnd2i(LISA_ST_D, ret0, env,
                lsenv_offset_of_mips_regs(lsenv, ret_reg));

        /* 2. read eflags via helper_read_eflags */
        latxs_append_ir2_opnd2_(lisa_mov, arg0, env);
        latxs_load_addr_to_ir2(stmp1, (ADDR)helper_read_eflags);
        latxs_append_ir2_opnd1_(lisa_call, stmp1);
        latxs_append_ir2_opnd2_(lisa_mov, eflags, ret0);

        /* 3. restore return value */
        latxs_append_ir2_opnd2i(LISA_LD_D, ret0, env,
                lsenv_offset_of_mips_regs(lsenv, ret_reg));
    }

    /* Move  OF/SF/ZF/AF/PF/CF into CPU */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, eflags, 0x3f);
    /* Clear OF/SF/ZF/AF/PF/CF of elfgas */
    IR2_OPND *cc_mask = stmp1;
    latxs_append_ir2_opnd2i(LISA_ORI, cc_mask, zero, 0x8d5);
    latxs_append_ir2_opnd3(LISA_NOR, cc_mask, zero, cc_mask);
    latxs_append_ir2_opnd3(LISA_AND, eflags, eflags, cc_mask);
}

bool latxs_translate_cld(IR1_INST *pir1)
{
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W,
            &latxs_eflags_ir2_opnd,
            &latxs_zero_ir2_opnd, 10, 10);

    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &tmp,
            &latxs_zero_ir2_opnd, 1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp,
            &latxs_env_ir2_opnd, lsenv_offset_of_df(lsenv));
    latxs_ra_free_temp(&tmp);

    return true;
}

bool latxs_translate_std(IR1_INST *pir1)
{
    latxs_append_ir2_opnd2i(LISA_ORI,
            &latxs_eflags_ir2_opnd,
            &latxs_eflags_ir2_opnd, 0x400);

    IR2_OPND tmp = latxs_ra_alloc_itemp();
    latxs_load_imm64_to_ir2(&tmp, -1);
    latxs_append_ir2_opnd2i(LISA_ST_W, &tmp,
            &latxs_env_ir2_opnd, lsenv_offset_of_df(lsenv));

    return true;
}

bool latxs_translate_clc(IR1_INST *pir1)
{
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &latxs_zero_ir2_opnd, 0x1);
    return true;
}

bool latxs_translate_stc(IR1_INST *pir1)
{
    IR2_OPND cf_opnd = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &cf_opnd, &latxs_zero_ir2_opnd, 0x1);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &cf_opnd, 0x1);
    latxs_ra_free_temp(&cf_opnd);
    return true;
}

bool latxs_translate_cmc(IR1_INST *pir1)
{
    IR2_OPND temp = ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &temp, 0x1);
    latxs_append_ir2_opnd2i(LISA_XORI, &temp, &temp, 0x1);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &temp, 0x1);
    latxs_ra_free_temp(&temp);
    return true;
}

bool latxs_translate_clac(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLAC(pir1);

    IR2_OPND mask = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&mask, AC_MASK, EXMode_Z);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, &mask, &latxs_zero_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_AND, &latxs_eflags_ir2_opnd,
            &latxs_eflags_ir2_opnd, &mask);
    return true;
}

bool latxs_translate_stac(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_STAC(pir1);

    IR2_OPND mask = latxs_ra_alloc_itemp();
    latxs_load_imm32_to_ir2(&mask, AC_MASK, EXMode_Z);
    latxs_append_ir2_opnd3(LISA_OR, &latxs_eflags_ir2_opnd,
            &latxs_eflags_ir2_opnd, &mask);
    return true;
}

/* End of TB in system-mode */
bool latxs_translate_popf(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_POPF(pir1);

    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);

    /*
     * popf
     * ----------------------
     * > eflags <= MEM(SS:ESP)
     * > ESP    <= ESP + 2/4
     * ----------------------
     * 1. tmp   <= MEM(SS:ESP) : softmmu
     * 2. tmp   => eflags : helper_write_eflags
     * 3. ESP   <= ESP + 2/4
     */

    /* 1.1 build MEM(SS:ESP) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0, 0, 0);

    /* 1.2 read data from stack   : might generate exception */
    IR2_OPND tmp = latxs_ra_alloc_itemp();
    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_load_ir1_mem_to_ir2(&tmp,
            &mem_ir1_opnd, EXMode_Z, ss_addr_size);

    /*
     * 2. write into eflags
     *
     * void helper_write_eflags(
     *      CPUX86State *env,
     *      target_ulong t0,
     *      uint32_t update_mask)
     */
    IR2_OPND tmp2 = latxs_ra_alloc_itemp();

    /* 2.0 save native context */
    latxs_tr_gen_call_to_helper_prologue_cfg(default_helper_cfg);

    /* 2.1 tmp2: mask for eflags */
    uint32 eflags_mask = 0;
    if (td->sys.cpl == 0) {
        eflags_mask = TF_MASK | AC_MASK | ID_MASK | NT_MASK |
                      IF_MASK | IOPL_MASK;
    } else {
        if (td->sys.cpl <= td->sys.iopl) {
            eflags_mask = TF_MASK | AC_MASK | ID_MASK | NT_MASK |
                          IF_MASK;
        } else {
            eflags_mask = TF_MASK | AC_MASK | ID_MASK | NT_MASK;
        }
    }
    if (data_size == 16) {
        eflags_mask &= 0xffff;
    }
    latxs_load_imm32_to_ir2(&tmp2, eflags_mask, EXMode_Z);
    /* 2.2 arg1: data to write */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg1_ir2_opnd, &tmp);
    /* 2.3 arg2: eflags mask */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg2_ir2_opnd, &tmp2);
    /* 2.4 arg0: env */
    latxs_append_ir2_opnd2_(lisa_mov, &latxs_arg0_ir2_opnd,
                                      &latxs_env_ir2_opnd);
    /* 2.5 call helper_write_eflags : might generate exception */
    latxs_tr_gen_call_to_helper((ADDR)helper_write_eflags);
    /* 2.6 restore context */
    latxs_tr_gen_call_to_helper_epilogue_cfg(default_helper_cfg);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W,
                &esp_opnd, &esp_opnd, (data_size >> 3));
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D,
                &tmp, &esp_opnd, (data_size >> 3));
        latxs_store_ir2_to_ir1(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

bool latxs_translate_pushf(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_PUSHF(pir1);

    int data_size = latxs_ir1_data_size(pir1);
    lsassert(data_size == 16 || data_size == 32);

    /*
     * pushf
     * ----------------------
     * >  ESP    <= ESP - 2/4
     * >  eflags => MEM(SS:ESP)
     * ----------------------
     * 1. eflags <= eflags : mapping register
     * 2. tmp    => MEM(SS:ESP - 2/4) : softmmu
     * 3. ESP    <= ESP - 2/4
     */

    /*
     * 1. get the eflags
     *    since we sync the eflags in context switch and
     *    x86_to_mips_before_exec_tb(), the eflags mapping
     *    registers always contains the up-to-date eflags
     */
    IR2_OPND *eflags = &latxs_eflags_ir2_opnd;
    IR2_OPND ls_eflags = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2_(lisa_mov, &ls_eflags, &latxs_zero_ir2_opnd);
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &ls_eflags,  0x3f);
    latxs_append_ir2_opnd3(LISA_OR, &ls_eflags, eflags, &ls_eflags);

    /* 2.1 build MEM(SS:ESP - 2/4) */
    IR1_OPND mem_ir1_opnd;
    latxs_ir1_opnd_build_full_mem(&mem_ir1_opnd, data_size,
            X86_REG_SS, X86_REG_ESP, 0 - (data_size >> 3), 0, 0);

    /*
     * 2.2 write eflags into stack : might generate exception
     *     pushf will not write VM(17) and RF(16)
     */
    IR2_OPND eflags_to_push = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd2i(LISA_ORI, &eflags_to_push,
                                      &latxs_zero_ir2_opnd,  0x3);
    latxs_append_ir2_opnd2i(LISA_SLLI_D, &eflags_to_push,
                                         &eflags_to_push, 0x10);
    latxs_append_ir2_opnd2_(lisa_not, &eflags_to_push, &eflags_to_push);

    latxs_append_ir2_opnd3(LISA_AND,
            &eflags_to_push, &ls_eflags, &eflags_to_push);

    int ss_addr_size = latxs_get_sys_stack_addr_size();
    latxs_store_ir2_to_ir1_mem(&eflags_to_push,
            &mem_ir1_opnd, ss_addr_size);
    latxs_ra_free_temp(&eflags_to_push);

    /* 3. update ESP */
    IR2_OPND esp_opnd = latxs_ra_alloc_gpr(esp_index);
    if (lsenv->tr_data->sys.ss32) {
        latxs_append_ir2_opnd2i(LISA_ADDI_W,
                &esp_opnd, &esp_opnd, 0 - (data_size >> 3));
        if (option_by_hand) {
            latxs_ir2_opnd_set_emb(&esp_opnd, EXMode_S, 32);
        }
    } else {
        IR2_OPND tmp = latxs_ra_alloc_itemp();
        latxs_append_ir2_opnd2i(LISA_ADDI_D,
                &tmp, &esp_opnd, 0 - (data_size >> 3));
        latxs_store_ir2_to_ir1(&tmp, &sp_ir1_opnd);
        latxs_ra_free_temp(&tmp);
    }

    return true;
}

/* End of TB in system-mode */
bool latxs_translate_clts(IR1_INST *pir1)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    CHECK_EXCP_CLTS(pir1);

    /* void helper_clts(CPUX86State  *env) */

    IR2_OPND cr0 = latxs_ra_alloc_itemp();
    IR2_OPND hf = latxs_ra_alloc_itemp();

    int off_cr0 = lsenv_offset_of_cr(lsenv, 0);
    int off_hf  = lsenv_offset_of_hflags(lsenv);

    IR2_OPND *env = &latxs_env_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    latxs_append_ir2_opnd2i(LISA_LD_WU, &cr0, env, off_cr0);
    latxs_append_ir2_opnd2i(LISA_LD_WU, &hf, env, off_hf);

    IR2_OPND mask = latxs_ra_alloc_itemp();

    /*env->cr[0] &= ~CR0_TS_MASK;*/
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, CR0_TS_MASK);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, zero, &mask);
    latxs_append_ir2_opnd3(LISA_AND, &cr0, &cr0, &mask);

    /*env->hflags &= ~HF_TS_MASK;*/
    latxs_append_ir2_opnd2i(LISA_ORI, &mask, zero, HF_TS_MASK);
    latxs_append_ir2_opnd3(LISA_NOR, &mask, zero, &mask);
    latxs_append_ir2_opnd3(LISA_AND, &hf, &hf, &mask);

    latxs_append_ir2_opnd2i(LISA_ST_W, &cr0, env, off_cr0);
    latxs_append_ir2_opnd2i(LISA_ST_W, &hf, env, off_hf);

    return true;
}

bool latxs_translate_sahf(IR1_INST *pir1)
{
    IR2_OPND ah = latxs_ra_alloc_itemp();
    latxs_load_ir1_gpr_to_ir2(&ah, &ah_ir1_opnd, EXMode_Z);
    latxs_append_ir2_opnd2i(LISA_ORI, &ah, &ah, 0x2);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &ah, 0x1f);
    latxs_ra_free_temp(&ah);
    return true;
}

bool latxs_translate_lahf(IR1_INST *pir1)
{
    IR2_OPND ah = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd1i(LISA_X86MFFLAG, &ah, 0x1f);
    latxs_append_ir2_opnd2i(LISA_ORI, &ah, &ah, 0x2);
    latxs_store_ir2_to_ir1_gpr(&ah, &ah_ir1_opnd);
    latxs_ra_free_temp(&ah);
    return true;
}
