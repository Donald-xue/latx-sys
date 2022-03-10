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
    env->fastcs_ctx = FASTCS_CTX_NON;

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

void latxs_reset_tb_fastcs_ctx(TranslationBlock *tb)
{
    tb->fastcs_ctx = FASTCS_CTX_NON;
}

void latxs_disasm_tb_fastcs_ctx(TranslationBlock *tb, IR1_INST *pir1)
{
    if (!latxs_fastcs_enabled()) {
        return;
    }

    int i = 0;
    int grp_nr = latxs_ir1_grp_nr(pir1);
    uint8_t *grps = latxs_ir1_get_grps(pir1);

    switch (ir1_opcode(pir1)) {
    case X86_INS_FXSAVE:
    case X86_INS_FXRSTOR:
    case X86_INS_XSAVE:
    case X86_INS_XRSTOR:
    case X86_INS_XSAVEOPT:
        tb->fastcs_ctx |= FASTCS_CTX_FPU;
        tb->fastcs_ctx |= FASTCS_CTX_SIMD;
        break;
    default:
        break;
    }

    for (i = 0; i < grp_nr; ++i) {
        switch (grps[i]) {
            case X86_GRP_FPU:
            case X86_GRP_MMX:
            case X86_GRP_3DNOW: /* MMX */
                tb->fastcs_ctx |= FASTCS_CTX_FPU;
                break;
            case X86_GRP_AES:
            case X86_GRP_ADX:
            case X86_GRP_AVX:
            case X86_GRP_AVX2:
            case X86_GRP_AVX512:
            case X86_GRP_F16C:
            case X86_GRP_FMA:
            case X86_GRP_FMA4:
            case X86_GRP_SHA:
            case X86_GRP_SSE1:
            case X86_GRP_SSE2:
            case X86_GRP_SSE3:
            case X86_GRP_SSE41:
            case X86_GRP_SSE42:
            case X86_GRP_SSE4A:
            case X86_GRP_PCLMUL:
            case X86_GRP_XOP:
            case X86_GRP_CDI: /* AVX512CD  */
            case X86_GRP_ERI: /* AVX512ER  */
            case X86_GRP_DQI: /* AVX512DQ  */
            case X86_GRP_BWI: /* AVX512BW  */
            case X86_GRP_PFI: /* AVX512PF  */
            case X86_GRP_VLX: /* AVX512VL? */
                tb->fastcs_ctx |= FASTCS_CTX_SIMD;
                break;
            case X86_GRP_SSSE3:
                tb->fastcs_ctx |= FASTCS_CTX_FPU;
                tb->fastcs_ctx |= FASTCS_CTX_SIMD;
                break;
            default:
                break;
        }
    }
}
