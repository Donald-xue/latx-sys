#include "env.h"
#include "shadow_stack.h"
#include "latx-options.h"
#include "profile.h"
#include "ibtc.h"

__thread ENV *lsenv;
QHT * etb_qht;

__thread ENV lsenv_real;
__thread TRANSLATION_DATA tr_data_real;
__thread FLAG_PATTERN_DATA fp_data_real;
static QHT etb_qht_real;
SS ss;//shadow stack


extern void xtm_capstone_init(void);

static void __attribute__((__constructor__)) latx_init(void)
{
    context_switch_bt_to_native = 0;
    context_switch_native_to_bt_ret_0 = 0;
    context_switch_native_to_bt = 0;
    context_switch_is_init = 0;
    native_rotate_fpu_by = 0;

    options_init();
    xtm_capstone_init();

    etb_qht = &etb_qht_real;
    etb_qht_init();
    ss_init(&ss);//init shadow stack
}

void latx_exit(void)
{
    if (option_profile) {
        profile_generate();
        profile_dump(10);
    }
}

void latx_lsenv_init(CPUArchState *env)
{
    lsenv = &lsenv_real;
    lsenv->cpu_state = env;
    lsenv->tr_data = &tr_data_real;
    lsenv->fp_data = &fp_data_real;

    env->vregs[4] = (uint64_t)ss._ssi_current;
    if (option_dump) {
        fprintf(stderr, "[LATX] env init : %p\n", lsenv->cpu_state);
    }
}

void latx_set_tls_ibtc_table(CPUArchState *env)
{
    env->ibtc_table_p = &ibtc_table;
}
