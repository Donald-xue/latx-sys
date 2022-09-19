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
#include "latxs-cc-pro.h"

int latxs_cc_pro(void)
{
    return option_code_cache_pro;
}

int latxs_cc_pro_tb_flags_cmp(
        const TranslationBlock *tb,
        uint32_t cpu_flags)
{
    if (latxs_cc_pro() && tb->cc_flags == 0) {
        uint32_t __tb_flags  = tb->flags & ~0xe00;
        uint32_t __cpu_flags = cpu_flags & ~0xe00;
        if (__tb_flags == __cpu_flags) return 1;
    } else {
        if (tb->flags == cpu_flags) return 1;
    }
    
    return 0;
}
