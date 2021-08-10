#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>

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
