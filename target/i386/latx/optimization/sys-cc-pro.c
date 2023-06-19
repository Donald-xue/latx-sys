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
#include "latxs-cc-pro-excp.h"

#define CCPRO_NOLINK    0x1
#define CCPRO_CHECKTB   0x2
#define CCPRO_CHECKJMP  0x3
#define CCPRO_DYINST    0x4

#define CCPRO_TYPE_MASK 0xf
#define CCPRO_MODE_MASK 0xf0

#define CCPRO_DYINST_WITH_FILTER    (0x1 << 4)

int latxs_cc_pro(void)
{
    return option_code_cache_pro && !latxs_cc_pro_dyinst();
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

int latxs_cc_pro_dyinst(void)
{
    return (option_code_cache_pro & CCPRO_TYPE_MASK) == CCPRO_DYINST;
}

int latxs_cc_pro_dyinst_with_filter(void)
{
    return latxs_cc_pro_dyinst() &&
        (option_code_cache_pro & CCPRO_MODE_MASK) ==
                                 CCPRO_DYINST_WITH_FILTER;
}

int latxs_cc_pro_tb_flags_cmp(
        const TranslationBlock *tb,
        uint32_t cpu_flags)
{
    uint32_t cc_mask = tb->cc_mask;
    /*
     * ============== CPL3 ============== 
     * tb.cc_flags != 0 : tb.cc_mask = ~0
     * tb.cc_flags == 0 : tb.cc_mask = ~0xe00 (CC_FLAG_MASK)
     * ============== CPL0 ============== 
     * tb.cc_flags = 0  , tb.cc_mask = ~0
     */
    uint32_t __tb_flags  = tb->flags & cc_mask;
    uint32_t __cpu_flags = cpu_flags & cc_mask;
    return __tb_flags == __cpu_flags;
}

int latxs_cc_pro_for_tb(void *_tb)
{
    if (latxs_cc_pro()) {
        TranslationBlock *tb = _tb;
        return (tb->flags & 0x3) == 3;
    }
    return 0;
}

void latxs_cc_pro_init_tb(void *_tb)
{
    TranslationBlock *tb = _tb;

    tb->cc_flags = 0;

    if (latxs_cc_pro_for_tb(tb)) {
        tb->cc_mask  = ~CC_FLAG_MASK;
    } else {
        tb->cc_mask  = ~0;
    }
}

uint32_t latxs_cc_pro_reset_flags_for_hash(uint32_t flags)
{
    if (latxs_cc_pro() && (flags & 0x3) == 3) {
        return flags & ~CC_FLAG_MASK;
    }
    return flags;
}
uint32_t latxs_cc_pro_get_tb_flags_for_hash(void *_tb)
{
    TranslationBlock *tb = _tb;

    return latxs_cc_pro_reset_flags_for_hash(tb->flags);
}

void *latxs_cc_pro_get_next_ptr(void *_tb, void *_nexttb)
{
    TranslationBlock *tb = _tb;
    TranslationBlock *nexttb = _nexttb;

    if (!latxs_cc_pro() ||
        !latxs_cc_pro_for_tb(tb)) {
        return (void *)nexttb->tc.ptr;
    }

    void *next_ptr = NULL;

    if (latxs_cc_pro_checktb()) {
        if (nexttb->cc_flags && !(tb->cc_flags)) {
            next_ptr = (void *)nexttb->cc_ck_ptr;
        } else {
            next_ptr = (void *)nexttb->cc_ok_ptr;
        }
    } else {
        next_ptr = (void *)nexttb->tc.ptr;
    }

    assert(next_ptr);
    return next_ptr;
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



static void __excp_dy_check_em_or_ts_gen_prex(IR1_INST *pir1)
{
    /* TS | EM : prex exception */
    assert(!((HF_TS_MASK | HF_EM_MASK) >> 12));

    IR2_OPND no_excp = latxs_ir2_opnd_new_label();
    IR2_OPND flags = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_LD_WU, &flags, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, hflags));
    latxs_append_ir2_opnd2i(LISA_ANDI, &flags, &flags,
            HF_TS_MASK | HF_EM_MASK);
    latxs_append_ir2_opnd2(LISA_BEQZ, &flags, &no_excp);

    latxs_tr_gen_excp_prex(pir1, 0);

    latxs_append_ir2_opnd1(LISA_LABEL, &no_excp);
    latxs_ra_free_temp(&flags);
}



#define DY_CHECK_FP     (1<<0)
#define DY_CHECK_SSE    (1<<1)
#define DY_CHECK_MXCSR  (1<<2)
#define DY_CHECK_FX     (1<<3)
#define DY_CHECK_WAIT   (1<<4)

#define DY_CHECK_FILTER(td, n) do {             \
    if (latxs_cc_pro_dyinst_with_filter()) {    \
        if (td->cc_pro_dycheck_filter & n)      \
            return 1;                           \
        td->cc_pro_dycheck_filter |= n;         \
    }                                           \
} while (0)

int latxs_cc_pro_excp_check_fp(IR1_INST *pir1)
{
    if (!latxs_cc_pro_dyinst()) return 0;
    DY_CHECK_FILTER(lsenv->tr_data, DY_CHECK_FP);

    __excp_dy_check_em_or_ts_gen_prex(pir1);

    return 1;
}

int latxs_cc_pro_excp_check_sse(IR1_INST *pir1)
{
    if (!latxs_cc_pro_dyinst()) return 0;
    DY_CHECK_FILTER(lsenv->tr_data, DY_CHECK_SSE);

    /* TS : prex exception    */
    /* EM : illegal operation */
    assert(!((HF_TS_MASK | HF_EM_MASK) >> 12));

    IR2_OPND no_excp = latxs_ir2_opnd_new_label();
    IR2_OPND flags = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_LD_WU, &flags, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, hflags));
    latxs_append_ir2_opnd2i(LISA_ANDI, &flags, &flags,
            HF_TS_MASK | HF_EM_MASK);
    latxs_append_ir2_opnd2(LISA_BEQZ, &flags, &no_excp);

    IR2_OPND ill_excp = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2i(LISA_ANDI, &flags, &flags, HF_TS_MASK);
    latxs_append_ir2_opnd2(LISA_BEQZ, &flags, &ill_excp);

    latxs_tr_gen_excp_prex(pir1, 0); /* TS */

    latxs_append_ir2_opnd1(LISA_LABEL, &ill_excp);
    latxs_tr_gen_excp_illegal_op(pir1, 0); /* EM */

    latxs_append_ir2_opnd1(LISA_LABEL, &no_excp);

    latxs_ra_free_temp(&flags);

    return 1;
}

int latxs_cc_pro_excp_check_ldst_mxcsr(IR1_INST *pir1)
{
    if (!latxs_cc_pro_dyinst()) return 0;
    DY_CHECK_FILTER(lsenv->tr_data, DY_CHECK_MXCSR);

    TRANSLATION_DATA *td = lsenv->tr_data;

    assert(!(HF_EM_MASK >> 12));

    IR2_OPND no_excp = latxs_ir2_opnd_new_label();
    IR2_OPND flags = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_LD_WU, &flags, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, hflags));
    latxs_append_ir2_opnd2i(LISA_ANDI, &flags, &flags, HF_EM_MASK);
    latxs_append_ir2_opnd2(LISA_BEQZ, &flags, &no_excp);

    latxs_tr_gen_excp_illegal_op(pir1, 0); /* EM */

    latxs_append_ir2_opnd1(LISA_LABEL, &no_excp);

    latxs_ra_free_temp(&flags);

    if (!(td->sys.flags & HF_OSFXSR_MASK)) {
        latxs_tr_gen_excp_illegal_op(pir1, 1);
        return 2;
    }

    return 1;
}

int latxs_cc_pro_excp_check_fxsave(IR1_INST *pir1)
{
    if (!latxs_cc_pro_dyinst()) return 0;
    DY_CHECK_FILTER(lsenv->tr_data, DY_CHECK_FX);

    __excp_dy_check_em_or_ts_gen_prex(pir1);

    return 1;
}

int latxs_cc_pro_excp_check_wait(IR1_INST *pir1)
{
    if (!latxs_cc_pro_dyinst()) return 0;
    DY_CHECK_FILTER(lsenv->tr_data, DY_CHECK_WAIT);

    /* TS & MP : illegal operation */
    assert(!((HF_TS_MASK | HF_MP_MASK) >> 12));

    IR2_OPND no_excp = latxs_ir2_opnd_new_label();
    IR2_OPND flags = latxs_ra_alloc_itemp();
    IR2_OPND tmp = latxs_ra_alloc_itemp();

    latxs_append_ir2_opnd2i(LISA_LD_WU, &flags, &latxs_env_ir2_opnd,
            offsetof(CPUX86State, hflags));
    latxs_append_ir2_opnd2_(lisa_mov, &tmp, &flags);

    latxs_append_ir2_opnd2i(LISA_SRLI_D, &flags, &flags, HF_TS_SHIFT);
    latxs_append_ir2_opnd2i(LISA_SRLI_D, &tmp,   &tmp,   HF_MP_SHIFT);

    latxs_append_ir2_opnd3(LISA_AND,   &tmp, &tmp, &flags);
    latxs_append_ir2_opnd2i(LISA_ANDI, &tmp, &tmp, 0x1);

    latxs_append_ir2_opnd2(LISA_BEQZ, &tmp, &no_excp);

    latxs_tr_gen_excp_prex(pir1, 0);

    latxs_append_ir2_opnd1(LISA_LABEL, &no_excp);
    latxs_ra_free_temp(&flags);
    latxs_ra_free_temp(&tmp);

    return 1;
}
