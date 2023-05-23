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

#define CCPRO_NOLINK    0x1
#define CCPRO_CHECKTB   0x2
#define CCPRO_CHECKJMP  0x3

#define CCPRO_TYPE_MASK 0xf

int latxs_cc_pro(void)
{
    return option_code_cache_pro;
}

int latxs_cc_pro_nolink(void)
{
    /*
     * <1> cc_flags = 0 : ignoring MP/EM/TS
     * <2> cc_flags = 2 : contains FP/SIMD, but no excp
     * <3> cc_flags = 3 : contains FP/SIMD and generate excp
     *
     * from\to  <1>  <2>  <3>
     *  <1>      O    X    X        O : OK to link
     *  <2>      O    O    X        X : disable link
     *  <3>      -    -    -        - : can not link
     */
    return (option_code_cache_pro & CCPRO_TYPE_MASK) == CCPRO_NOLINK;
}

int latxs_cc_pro_checktb(void)
{
    /*
     * <1> cc_flags = 0 : ignoring MP/EM/TS
     * <2> cc_flags = 2 : contains FP/SIMD, but no excp
     * <3> cc_flags = 3 : contains FP/SIMD and generate excp
     *
     * from\to  <1>  <2>  <3>
     *  <1>     DIR  CHK  CHK      DIR: jump to cc_ok_ptr (no check)
     *  <2>     DIR  DIR  DIR      CHK: jump to cc_ck_ptr (do check)
     *  <3>      -    -    -        - : can not link
     */
    return (option_code_cache_pro & CCPRO_TYPE_MASK) == CCPRO_CHECKTB;
}

int latxs_cc_pro_checkjmp(void)
{
    /* TODO */
    return (option_code_cache_pro & CCPRO_TYPE_MASK) == CCPRO_CHECKJMP;
}

int latxs_cc_pro_tb_flags_cmp(
        const TranslationBlock *tb,
        uint32_t cpu_flags)
{
    uint32_t cc_mask = tb->cc_mask;
    /*
     * tb.cc_flags != 0 : tb.cc_mask = ~0
     * tb.cc_flags == 0 : tb.cc_mask = ~0xe00 (CC_FLAG_MASK)
     */
    uint32_t __tb_flags  = tb->flags & cc_mask;
    uint32_t __cpu_flags = cpu_flags & cc_mask;
    return __tb_flags == __cpu_flags;
}

void latxs_cc_pro_gen_tb_start(void)
{
    if (!latxs_cc_pro_checktb()) return;

    TRANSLATION_DATA *td = lsenv->tr_data;
    TranslationBlock *tb = td->curr_tb;

    if ((tb->flags & 0x3) != 3) return;

    IR2_OPND tbptr = latxs_ra_alloc_itemp();
    IR2_OPND tbflags  = latxs_ra_alloc_itemp();
    IR2_OPND envflags = latxs_ra_alloc_itemp();

    ADDR tb_addr  = (ADDR)tb;
    ADDR code_ptr = (ADDR)tb->tc.ptr;
    ADDR code_off = (ADDR)(lsenv->tr_data->ir2_asm_nr << 2);
    ADDR ins_pc = code_ptr + code_off;

    int offset = (tb_addr - ins_pc) >> 2;
    if (int32_in_int20(offset)) {
        latxs_append_ir2_opnd1i(LISA_PCADDI, &tbptr, offset);
    } else {
        latxs_load_imm64_to_ir2(&tbptr, tb_addr);
    }

    latxs_append_ir2_opnd2i(LISA_LD_WU, &tbflags, &tbptr,
            offsetof(TranslationBlock, flags));
    latxs_append_ir2_opnd2i(LISA_LD_WU, &envflags, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, hflags));
    latxs_append_ir2_opnd3(LISA_BNE, &tbflags, &envflags,
            &(td->exitreq_label));

    latxs_append_ir2_opnd1(LISA_LABEL, &td->cc_pro_label);
}
