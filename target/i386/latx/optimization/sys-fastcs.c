#include "lsenv.h"
#include "common.h"
#include "reg-alloc.h"
#include "flag-lbt.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "qemu/timer.h"
#include "trace.h"
#include <signal.h>
#include <ucontext.h>

int latxs_fastcs_enabled(void)
{
    return option_fastcs;
}

int latxs_fastcs_is_jmp_glue(void)
{
    return option_fastcs == FASTCS_JMP_GLUE;
}

int latxs_fastcs_is_ld_excp(void)
{
    return option_fastcs == FASTCS_LD_EXCP;
}

void latxs_fastcs_env_init(CPUX86State *env)
{
    if (env->fastcs_ptr != NULL) {
        return;
    }

    assert(lsenv->cpu_state == env);

    env->fastcs_ptr = &lsenv->fastcs_data;
    lsenv->fastcs_data.env = env;
}

void latxs_native_printer_cs(lsenv_np_data_t *npd,
        int type, int r1, int r2, int r3, int r4, int r5)
{
    CPUX86State *env = npd->env;
    lsenv_fastcs_t *fcs = env->fastcs_ptr;

    uint64_t cs_type = fcs->cs_type;
    ssize_t r = 0;
    switch(cs_type) {
    case 1:
        r = write(2, "\n1", 2);
        break;
    case 2:
        r = write(2, "2", 1);
        break;
    case 3:
        r = write(2, "3", 1);
        break;
    case 4:
        r = write(2, "4", 1);
        break;
    default:
        break;
    }
    lsassertm(r == 1 || r == 2, "write %d", r);
    (void)r;
}
