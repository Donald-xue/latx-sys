#include "common.h"
#include "reg-alloc.h"
#include "lsenv.h"
#include <string.h>
#include "latx-options.h"
#include "translate.h"

IR2_OPND ra_alloc_gpr(int gpr_num)
{
//    static const int8 rai_imap_gpr[8] = {15, 3, 1, 19, 20, 21, 22, 23};
    /*
     * Mapping to LA regs.
     */
    static const int8 rai_imap_gpr[8] = {15, 18, 19, 26, 27, 28, 29, 30};

    IR2_OPND opnd;
    ir2_opnd_build(&opnd, IR2_OPND_GPR, rai_imap_gpr[gpr_num]);

    return opnd;
}

IR2_OPND ra_alloc_mda(void)
{
    IR2_OPND opnd;
    /*MIPS R31 ---> LA R1*/
    ir2_opnd_build(&opnd, IR2_OPND_IREG, 1);
    return opnd;
}

/* to improve efficiency, we map some frequently used variables
 * to host registers:
 *   bit 0: guest base, mapped to $14, t2; should be valid for all native code
 *   bit 1: last executed tb, mapped to $24, t8; register not always valid
 *   bit 2: next x86 addr, mapped to $25; registerk not always valid
 *   bit 3: top_bias, not mapped to register
 *   bit 4: shadow_stack
 * they can be saved to and loaded from lsenv->cpu_state->vregs
 */
IR2_OPND ra_alloc_vreg(int vr_num)
{
    //static const int8 rai_imap_vr[5] = {14, 24, 25, 28, 17};
    /*
     * Mapping to LA directly.
     */
    static const int8 rai_imap_vr[5] = {14, 20, 16, 31, 24};

    IR2_OPND opnd;

    lsassert(vr_num >= 0 && vr_num <= 4);

    ir2_opnd_build(&opnd, IR2_OPND_IREG, rai_imap_vr[vr_num]);

    return opnd;
}

IR2_OPND ra_alloc_guest_base(void) { return ra_alloc_vreg(0); }

IR2_OPND ra_alloc_dbt_arg1(void) { return ra_alloc_vreg(1); }

IR2_OPND ra_alloc_dbt_arg2(void) { return ra_alloc_vreg(2); }

IR2_OPND ra_alloc_flag_pattern_saved_opnd0(void) { return ra_alloc_vreg(1); }

IR2_OPND ra_alloc_flag_pattern_saved_opnd1(void) { return ra_alloc_vreg(2); }

/* Mapping to LA 17->24*/
IR2_OPND ra_alloc_ss(void)
{
    IR2_OPND ir2_ss;
    ir2_opnd_build(&ir2_ss, IR2_OPND_IREG, 24);
    return ir2_ss;
}
/* Mapping to LA 18->25*/
IR2_OPND ra_alloc_env(void) { return ir2_opnd_new(IR2_OPND_IREG, 25); }
/* Mapping to LA 28->31*/
IR2_OPND ra_alloc_top(void) { return ir2_opnd_new(IR2_OPND_IREG, 31); }

/* Mapping to LA 30->22*/
IR2_OPND ra_alloc_eflags(void) { return ir2_opnd_new(IR2_OPND_IREG, 22); }
/*
 * FIXME: What is the freg mapping rules for LA?
 */
IR2_OPND ra_alloc_f32(void) { return ir2_opnd_new(IR2_OPND_FREG, 8); }

IR2_OPND ra_alloc_st(int st_num)
{
    if (option_lsfpu) {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[st_num]);
    } else {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        int fpr_num = (lsenv->tr_data->curr_top + st_num) & 7;
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[fpr_num]);
    }
}

IR2_OPND ra_alloc_mmx(int mmx_num)
{
    if (option_lsfpu) {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[mmx_num]);
    } else {
        static const int8 rai_fmap_mmx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_mmx[mmx_num]);
    }
}

IR2_OPND ra_alloc_xmm_lo(int xmm_lo_num)
{
    lsassert(!option_xmm128map);
    static const int8 rai_fmap_xmm_lo[8] = {16, 18, 20, 22, 24, 26, 28, 30};
    return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_xmm_lo[xmm_lo_num]);
}

IR2_OPND ra_alloc_xmm_hi(int xmm_hi_num)
{
    lsassert(!option_xmm128map);
    static const int8 rai_fmap_xmm_hi[8] = {17, 19, 21, 23, 25, 27, 29, 31};
    return ir2_opnd_new(IR2_OPND_FREG, rai_fmap_xmm_hi[xmm_hi_num]);
}

IR2_OPND ra_alloc_xmm(int num) {
    lsassert(option_xmm128map);
    lsassert(0 <= num && num <=7);
    return ir2_opnd_new(IR2_OPND_FREG, 16 + num);//replace by shell script
}

IR2_OPND ra_alloc_itemp(void)
{
    int itemp_index = (lsenv->tr_data->itemp_num)++;
    IR2_OPND ir2_opnd;

    lsassert(itemp_index < 10);

    int itemp_num = itemp_status_default[itemp_index%10].physical_id;

    lsenv->tr_data->ireg_em[itemp_num] = UNKNOWN_EXTENSION;
    lsenv->tr_data->ireg_eb[itemp_num] = 32;

    ir2_opnd_build(&ir2_opnd, IR2_OPND_IREG, itemp_num);
    return ir2_opnd;
}

IR2_OPND ra_alloc_ftemp(void)
{
    int ftemp_index = (lsenv->tr_data->ftemp_num)++;

    lsassert(ftemp_index < 7);

    int ftemp_num = ftemp_status_default[ftemp_index%7].physical_id;
    return ir2_opnd_new(IR2_OPND_FREG, ftemp_num);
}

IR2_OPND ra_alloc_itemp_internal(void) { return ra_alloc_itemp(); }

IR2_OPND ra_alloc_ftemp_internal(void) { return ra_alloc_ftemp(); }

void ra_free_temp(IR2_OPND opnd) {}
void ra_free_all_internal_temp(void) {
    /* reset itemp_num*/
    TRANSLATION_DATA *tr_data = lsenv->tr_data;

    tr_data->itemp_num = 0;
    tr_data->ftemp_num = 0;
}
void ra_free_itemp(int i) {}
void ra_free_ftemp(int i) {}

EXTENSION_MODE ir2_opnd_default_em(IR2_OPND *opnd)
{
    lsassert(ir2_opnd_is_ireg(opnd));
#ifdef N64 /* validate address */
    IR2_OPND ir2_esp = ra_alloc_gpr(esp_index);
    IR2_OPND ir2_ebp = ra_alloc_gpr(ebp_index);
    if (ir2_opnd_cmp(opnd, &ir2_esp))
        return EM_X86_ADDRESS;
    else if (ir2_opnd_cmp(opnd, &ir2_ebp))
        return ZERO_EXTENSION;
    else
        return SIGN_EXTENSION;
#else
    return SIGN_EXTENSION;
#endif
}

void ra_init(void)
{
    memcpy(lsenv->tr_data->itemp_status, itemp_status_default,
           sizeof(itemp_status_default));
    memcpy(lsenv->tr_data->ftemp_status, ftemp_status_default,
           sizeof(ftemp_status_default));
}
