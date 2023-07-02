#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "latxs-fastcs-cfg.h"
#include "latx-perfmap.h"
#include "latx-bpc-sys.h"
#include "latx-np-sys.h"
#include "latx-intb-sys.h"
#include "latx-sigint-sys.h"
#include "latxs-cc-pro.h"
#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#include "latx-callret-internal.h"

//#define LATX_JR_RA_DEBUG

#ifdef LATX_JR_RA_DEBUG
#define FROMTO_PRINT(fmt, ...)  do {    \
    printf(fmt, __VA_ARGS__);           \
} while (0)
#define FROMTO_PRINT_LINE()     do {    \
    printf("\n");                       \
} while (0)
#else
#define FROMTO_PRINT(fmt, ...)
#define FROMTO_PRINT_LINE()
#endif

#define FROMTO_PTR(p)   ((void *)(((uint64_t)(p)) & (~0x1)))
#define FROMTO_NEXT(p)  (((uint64_t)(p)) & 0x1)
#define FROMTO_BUILD(p) ((void *)(((uint64_t)(p)) | (0x1)))

int latxs_jr_ra(void)
{
    return option_jr_ra;
}

int latxs_jr_ra_for_tb(TranslationBlock *tb)
{
    return (tb->flags & 0x3) == 3;
}

void latxs_jr_ra_gen_call(IR1_INST *pir1,
        IR2_OPND *nextpc, IR2_OPND *tmp)
{
    TranslationBlock *tb = lsenv->tr_data->curr_tb;
    /* SCR0 = next PC */
    latxs_append_ir2_opnd2(LISA_GR2SCR, &latxs_scr0_ir2_opnd, nextpc);
    /* SCR1 = next TB.tc.ptr */
    latxs_append_ir2_opnd1(LISA_LABEL, &lsenv->tr_data->jr_ra_label);
    latxs_append_ir2_opnd0_(lisa_nop); /* PCALAU12I */
    latxs_append_ir2_opnd0_(lisa_nop); /* ORI       */
    latxs_append_ir2_opnd2(LISA_GR2SCR, /* gr2scr    */
            &latxs_scr1_ir2_opnd, &latxs_zero_ir2_opnd);

    tb->scr_reg = latxs_ir2_opnd_reg(tmp);
    tb->nextpc = ir1_addr_next(pir1);
}

void latxs_jr_ra_finish_call(TranslationBlock *tb, TranslationBlock *tb2)
{
    int reg = tb->scr_reg;
    uint32_t *jr_ra_ptr = tb->jr_ra_ptr;

    FROMTO_PRINT("%s from tb.pc %lx to tb.pc %lx\n",
            __func__, (uint64_t)tb->pc, (uint64_t)tb2->pc);

    uint32_t i1_pcalau12i = 0;
    uint32_t i2_ori = 0;
    uint32_t i3_gr2scr = 0;

    uint64_t from = (uint64_t)jr_ra_ptr;
    uint64_t to   = (uint64_t)tb2->tc.ptr;

    lsassertm(12 <= reg && reg <= 19, "%s reg %d not tmp\n",
            __func__, reg);

    uint64_t off_high = ((to >> 12) - (from >> 12)) & 0xfffff;
    uint64_t off_low  = to & 0xfff;

    i1_pcalau12i = 0x1a000000 | reg | (off_high << 5);
    i2_ori       = 0x03800000 | (off_low << 10) | (reg << 5) | reg;
    i3_gr2scr    = 0x801      | (reg << 5);

    jr_ra_ptr[0] = i1_pcalau12i;
    jr_ra_ptr[1] = i2_ori;
    jr_ra_ptr[2] = i3_gr2scr;

    if (tb2->jr_ra_call_from == NULL) {
        tb->jr_ra_call_to    = tb2;
        tb2->jr_ra_call_from = tb;
    } else {
        TranslationBlock *_tb = tb2->jr_ra_call_from;
        tb2->jr_ra_call_from = tb;
        tb->jr_ra_call_to = FROMTO_BUILD(_tb);
    }

    FROMTO_PRINT("%s tb %p tb.to   %p\n", __func__, tb, tb->jr_ra_call_to);
    FROMTO_PRINT("%s tb %p tb.from %p\n", __func__, tb2, tb2->jr_ra_call_from);
    FROMTO_PRINT_LINE();
}

void latxs_jr_ra_reset_call(TranslationBlock *tb)
{
    TranslationBlock *from = tb->jr_ra_call_from;
    TranslationBlock *to   = tb->jr_ra_call_to;

    tb->jr_ra_call_from = NULL;
    tb->jr_ra_call_to   = NULL;

    TranslationBlock *ptb = NULL;
    uint32_t *jr_ra_ptr = NULL;

    int fromto_done = 0;

    while (from) {
        FROMTO_PRINT("%s from %p to %p\n", __func__, from, tb);

        lsassertm(from->jr_ra_ptr, "%s jr ra ptr is null\n", __func__);

        jr_ra_ptr = from->jr_ra_ptr;
        jr_ra_ptr[0] = 0x03400000; /* andi   zero, zero, 0 */
        jr_ra_ptr[1] = 0x03400000; /* andi   zero, zero, 0 */
        jr_ra_ptr[2] = 0x00000801; /* gr2scr scr1, zero    */

        if (FROMTO_PTR(from->jr_ra_call_to) != tb) {
            ptb = FROMTO_PTR(from->jr_ra_call_to);
            from->jr_ra_call_to = NULL;

            from = ptb;
        } else {
            from->jr_ra_call_to = NULL;
            from = NULL;
        }

        fromto_done = 1;
    }

    if (to) {
        if (FROMTO_NEXT(to)) {
            FROMTO_PRINT("%s tb %p to %p next\n", __func__, tb, to);
            ptb = FROMTO_PTR(to);
            while (FROMTO_NEXT(ptb->jr_ra_call_to)) {
                FROMTO_PRINT("%s tb %p to %p next\n", __func__, tb, ptb->jr_ra_call_to);
                ptb = FROMTO_PTR(ptb->jr_ra_call_to);
            }
            ptb = ptb->jr_ra_call_to;
            FROMTO_PRINT("%s tb %p to %p next find\n", __func__, tb, ptb);

            from = ptb->jr_ra_call_from;
            if (from == tb) {
                FROMTO_PRINT("%s tb.to %p from %p find tb-self\n", __func__, ptb, from);
                ptb->jr_ra_call_from = FROMTO_PTR(to);
            } else {
                FROMTO_PRINT("%s tb.to %p from search %p\n", __func__, ptb, from);
                while (FROMTO_PTR(from->jr_ra_call_to) != tb) {
                    from = FROMTO_PTR(from->jr_ra_call_to);
                    FROMTO_PRINT("%s tb.to %p from search %p\n", __func__, ptb, from);
                }
                FROMTO_PRINT("%s tb.to %p from search %p find\n", __func__, ptb, from);
                from->jr_ra_call_to = to;
                FROMTO_PRINT("%s set %p.to = %p\n", __func__, from, to);
            }
        } else {
            FROMTO_PRINT("%s tb %p to %p\n", __func__, tb, to);
            from = to->jr_ra_call_from;
            if (from == tb) {
                FROMTO_PRINT("%s tb %p to %p to.from %p find tb-self\n", __func__, tb, to, from);
                to->jr_ra_call_from = NULL;
            } else {
                FROMTO_PRINT("%s tb %p to %p to.from %p search\n", __func__, tb, to, from);
                while (FROMTO_PTR(from->jr_ra_call_to)!= tb) {
                    from = FROMTO_PTR(from->jr_ra_call_to);
                    FROMTO_PRINT("%s tb %p to %p to.from %p search\n", __func__, tb, to, from);
                }
                FROMTO_PRINT("%s tb %p to %p to.from %p search find to.from.to %p\n", __func__, tb, to, from, FROMTO_PTR(from->jr_ra_call_to));
                from->jr_ra_call_to = to;
                FROMTO_PRINT("%s set %p.to = %p\n", __func__, from, to);
            }
        }

        fromto_done = 1;
    }

    if (fromto_done) {
        FROMTO_PRINT_LINE();
    }
}

void latxs_jr_ra_gen_ret(IR1_INST *pir1,
        IR2_OPND *retpc, IR2_OPND *tmp)
{
    IR2_OPND miss = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd2(LISA_SCR2GR, tmp, &latxs_scr0_ir2_opnd);
    latxs_append_ir2_opnd3(LISA_BNE, retpc, tmp, &miss);
    /* hit */
    latxs_append_ir2_opnd2(LISA_SCR2GR, tmp, &latxs_scr1_ir2_opnd);
    latxs_append_ir2_opnd2(LISA_BEQZ, tmp, &miss);
    latxs_append_ir2_opnd2i(LISA_JIRL, &latxs_zero_ir2_opnd, tmp, 0);
    /* miss */
    latxs_append_ir2_opnd1(LISA_LABEL, &miss);
}

void latxs_clear_scr(void)
{
    __asm__ __volatile__ (
            ".word 0x800\n" /* gr2scr scr0, zero */
            ".word 0x801\n" /* gr2scr scr1, zero */
            );
}
