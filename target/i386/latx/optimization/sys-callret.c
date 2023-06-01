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

#ifdef LATX_JR_RA_DEBUG
    printf("%s jrraptr %p from %x to %x\n",
            __func__, tb->jr_ra_ptr, tb->pc, tb2->pc);
#endif

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

    tb->jr_ra_call_to = tb2;
    tb2->jr_ra_call_from = tb;
}

void latxs_jr_ra_reset_call(TranslationBlock *tb)
{
    TranslationBlock *from = tb->jr_ra_call_from;
    if (from) {
#ifdef LATX_JR_RA_DEBUG
        printf("%s from %x to %x\n",
                __func__, from->pc, tb->pc);
#endif

        lsassertm(from->jr_ra_call_to == tb,
                "%s from %p from.to %p tb %p tb.from %p\n",
                __func__,
                from, from->jr_ra_call_to,
                tb, tb->jr_ra_call_from);
        lsassertm(from->jr_ra_ptr, "%s jr ra ptr is %p\n",
                __func__, from->jr_ra_ptr);

        uint32_t *jr_ra_ptr = from->jr_ra_ptr;

        jr_ra_ptr[0] = 0x03400000; /* andi   zero, zero, 0 */
        jr_ra_ptr[1] = 0x03400000; /* andi   zero, zero, 0 */
        jr_ra_ptr[2] = 0x00000801; /* gr2scr scr1, zero    */

        from->jr_ra_call_to = NULL;
        tb->jr_ra_call_from = NULL;
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
