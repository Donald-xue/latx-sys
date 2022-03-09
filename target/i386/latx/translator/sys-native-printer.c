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

    /* reset env = &CPUX86State */
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
}

