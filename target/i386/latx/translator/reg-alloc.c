#include "common.h"
#include "reg-alloc.h"
#include "env.h"
#include <string.h>
#include "latx-options.h"

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
    int itemp_num = ++(lsenv->tr_data->itemp_num);
    IR2_OPND ir2_opnd;

    lsassert(itemp_num < IR2_ITEMP_MAX);
    lsenv->tr_data->ireg_em[itemp_num] = UNKNOWN_EXTENSION;
    lsenv->tr_data->ireg_eb[itemp_num] = 32;

    ir2_opnd_build(&ir2_opnd, IR2_OPND_IREG, itemp_num);
    return ir2_opnd;
}

IR2_OPND ra_alloc_ftemp(void)
{
    int ftemp_num = ++(lsenv->tr_data->ftemp_num);
    return ir2_opnd_new(IR2_OPND_FREG, ftemp_num);
}

IR2_OPND ra_alloc_itemp_internal(void) { return ra_alloc_itemp(); }

IR2_OPND ra_alloc_ftemp_internal(void) { return ra_alloc_ftemp(); }

void ra_free_temp(IR2_OPND opnd) {}
void ra_free_all_internal_temp(void) {}
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

static int rai_allocate_itemp_physical_id(int virtual_id, IR2_INST *curr_ir2)
{
    int i = 0;
    int ret = 0;

    TEMP_REG_STATUS *p = lsenv->tr_data->itemp_status;
    /* 1. already allocated a physical id */
    for (i = 0; i < itemp_status_num; ++i) {
        if (p[i].virtual_id == virtual_id) {
            ret = p[i].physical_id;
            goto _FIND_OUT_;
        }
    }
    /* 2. try to allocate a physical id */
    for (i = 0; i < itemp_status_num; ++i) {
        if (p[i].virtual_id == 0) {
            p[i].virtual_id = virtual_id;
            ret = p[i].physical_id;
            goto _FIND_OUT_;
        }
    }

    /* 3. if cannot allocate physical id, determine live virtual id */
    ds_set live_virtual_ids = {0, 0, NULL};       /* int16 */
    ds_set labels_need_checking = {0, 0, NULL};   /* int */
    ds_set labels_already_checked = {0, 0, NULL}; /* int */

    ds_set *p_live_virtual_ids = &live_virtual_ids;             /* int16 */
    ds_set *p_labels_need_checking = &labels_need_checking;     /* int */
    ds_set *p_labels_already_checked = &labels_already_checked; /* int */

    DSSET_INIT(p_live_virtual_ids, 64, sizeof(int16));
    DSSET_INIT(p_labels_need_checking, 64, sizeof(int));
    DSSET_INIT(p_labels_already_checked, 64, sizeof(int));

    /* 3.1 check the following ir2 */
    IR2_INST *pir2 = curr_ir2;
    while (pir2 != NULL) {
        if (ir2_opcode(pir2) == LISA_LABEL) {
            lsassert(ir2_opnd_is_label(&pir2->_opnd[0]));
            DSSET_INSERT(p_labels_already_checked, int,
                         ir2_opnd_addr(&pir2->_opnd[0]));
        } else {
            for (i = 0; i < 3; ++i) {
                IR2_OPND *popnd = &pir2->_opnd[i];
                if (ir2_opnd_is_itemp(popnd) ||
                    ir2_opnd_is_mem_base_itemp(popnd)) {
                    DSSET_INSERT(p_live_virtual_ids, int16,
                                 ir2_opnd_base_reg_num(popnd));
                } else if (ir2_opnd_is_label(popnd)) {
                    DSSET_INSERT(p_labels_need_checking, int,
                                 ir2_opnd_addr(popnd));
                }
            }
        }

        pir2 = ir2_next(pir2);
    }

    /* 3.2 check the previous ir2 */
    int is_subset = 1;
    DSSET_ISSUBSET(is_subset, int, p_labels_already_checked,
                   p_labels_need_checking);
    if (!is_subset) {
        IR2_INST *pir2 = curr_ir2;
        while (pir2 != NULL) {
            if (ir2_opcode(pir2) == LISA_LABEL) {
                lsassert(ir2_opnd_is_label(&pir2->_opnd[0]));
                DSSET_INSERT(p_labels_already_checked, int,
                             ir2_opnd_addr(&pir2->_opnd[0]));
                DSSET_ISSUBSET(is_subset, int, p_labels_already_checked,
                               p_labels_need_checking);
                if (is_subset)
                    break;
            } else {
                for (i = 0; i < 3; ++i) {
                    IR2_OPND *popnd = &pir2->_opnd[i];
                    if (ir2_opnd_is_itemp(popnd) ||
                        ir2_opnd_is_mem_base_itemp(popnd)) {
                        DSSET_INSERT(p_live_virtual_ids, int16,
                                     ir2_opnd_base_reg_num(popnd));
                    } else if (ir2_opnd_is_label(popnd)) {
                        DSSET_INSERT(p_labels_need_checking, int,
                                     ir2_opnd_addr(popnd));
                    }
                }
            }

            pir2 = ir2_prev(pir2);
        }
    }

    /* 4. release dead virtual id from the table */
    int vid_count = 0;
    for (i = 0; i < itemp_status_num; ++i) {
        vid_count = 0;
        DSSET_COUNT(vid_count, p_live_virtual_ids, int16, p[i].virtual_id);
        if (vid_count == 0) {
            p[i].virtual_id = 0;
        }
    }

    /* 5. try to allocate a physical id again */
    for (i = 0; i < itemp_status_num; ++i) {
        if (p[i].virtual_id == 0) {
            p[i].virtual_id = virtual_id;
            ret = p[i].physical_id;
            goto _FIND_OUT_CLEAR_;
        }
    }

    lsassertm(0, "cannot alloate temp register\n");
_FIND_OUT_CLEAR_:
    DSSET_FINI(p_live_virtual_ids);
    DSSET_FINI(p_labels_need_checking);
    DSSET_FINI(p_labels_already_checked);
_FIND_OUT_:
    return ret;
}

static int rai_allocate_ftemp_physical_id(int virtual_id, IR2_INST *curr_ir2)
{
    int i = 0;
    int8 ret = 0;

    TEMP_REG_STATUS *p = lsenv->tr_data->ftemp_status;

    /* 1. already allocated a physical id */
    for (i = 0; i < ftemp_status_num; ++i) {
        if (p[i].virtual_id == virtual_id) {
            ret = p[i].physical_id;
            goto _FIND_OUT_;
        }
    }

    /* 2. try to allocate a physical id */
    for (i = 0; i < ftemp_status_num; ++i) {
        if (p[i].virtual_id == 0) {
            p[i].virtual_id = virtual_id;
            ret = p[i].physical_id;
            goto _FIND_OUT_;
        }
    }

    /* 3. if cannot allocate physical id, determine live virtual id */
    ds_set live_virtual_ids = {0, 0, NULL};       /* int16 */
    ds_set labels_need_checking = {0, 0, NULL};   /* int */
    ds_set labels_already_checked = {0, 0, NULL}; /* int */

    ds_set *p_live_virtual_ids = &live_virtual_ids;             /* int16 */
    ds_set *p_labels_need_checking = &labels_need_checking;     /* int */
    ds_set *p_labels_already_checked = &labels_already_checked; /* int */

    DSSET_INIT(p_live_virtual_ids, 64, sizeof(int16));
    DSSET_INIT(p_labels_need_checking, 64, sizeof(int));
    DSSET_INIT(p_labels_already_checked, 64, sizeof(int));

    /* 3.1 check the following ir2 */
    IR2_INST *pir2 = curr_ir2;
    while (pir2 != NULL) {
        if (ir2_opcode(pir2) == LISA_LABEL) {
            lsassert(ir2_opnd_is_label(&pir2->_opnd[0]));
            DSSET_INSERT(p_labels_already_checked, int,
                         ir2_opnd_addr(&pir2->_opnd[0]));
        } else {
            for (i = 0; i < 3; ++i) {
                IR2_OPND *popnd = &pir2->_opnd[i];
                if (ir2_opnd_is_ftemp(popnd)) {
                    DSSET_INSERT(p_live_virtual_ids, int16,
                                 ir2_opnd_base_reg_num(popnd));
                } else if (ir2_opnd_is_label(popnd)) {
                    DSSET_INSERT(p_labels_need_checking, int,
                                 ir2_opnd_addr(popnd));
                }
            }
        }

        pir2 = ir2_next(pir2);
    }

    /* 3.2 check the previous ir2 */
    int is_subset = 1;
    DSSET_ISSUBSET(is_subset, int, p_labels_already_checked,
                   p_labels_need_checking);
    if (!is_subset) {
        IR2_INST *pir2 = curr_ir2;
        while (pir2 != NULL) {
            if (ir2_opcode(pir2) == LISA_LABEL) {
                lsassert(ir2_opnd_is_label(&pir2->_opnd[0]));
                DSSET_INSERT(p_labels_already_checked, int,
                             ir2_opnd_addr(&pir2->_opnd[0]));
                DSSET_ISSUBSET(is_subset, int, p_labels_already_checked,
                               p_labels_need_checking);
                if (is_subset)
                    break;
            } else {
                for (i = 0; i < 3; ++i) {
                    IR2_OPND *popnd = &pir2->_opnd[i];
                    if (ir2_opnd_is_ftemp(popnd)) {
                        DSSET_INSERT(p_live_virtual_ids, int16,
                                     ir2_opnd_base_reg_num(popnd));
                    } else if (ir2_opnd_is_label(popnd)) {
                        DSSET_INSERT(p_labels_need_checking, int,
                                     ir2_opnd_addr(popnd));
                    }
                }
            }

            pir2 = ir2_prev(pir2);
        }
    }

    /* 4. release dead virtual id from the table */
    int vid_count = 0;
    for (i = 0; i < ftemp_status_num; ++i) {
        vid_count = 0;
        DSSET_COUNT(vid_count, p_live_virtual_ids, int16, p[i].virtual_id);
        if (vid_count == 0) {
            p[i].virtual_id = 0;
        }
    }

    /* 5. try to allocate a physical id again */
    for (i = 0; i < ftemp_status_num; ++i) {
        if (p[i].virtual_id == 0) {
            p[i].virtual_id = virtual_id;
            ret = p[i].physical_id;
            goto _FIND_OUT_CLEAR_;
        }
    }

    lsassertm(0, "cannot alloate temp register\n");
_FIND_OUT_CLEAR_:
    DSSET_FINI(p_live_virtual_ids);
    DSSET_FINI(p_labels_need_checking);
    DSSET_FINI(p_labels_already_checked);
_FIND_OUT_:
    return ret;
}

bool ra_temp_register_allocation(void)
{
    TRANSLATION_DATA *t = lsenv->tr_data;
    IR2_INST *pir2 = t->first_ir2;

    int i = 0;

    while (pir2 != NULL) {
        for (i = 0; i < 4; ++i) {
            IR2_OPND *popnd = &pir2->_opnd[i];
            int reg_num = ir2_opnd_base_reg_num(popnd);

            if (ir2_opnd_is_itemp(popnd) || ir2_opnd_is_mem_base_itemp(popnd)) {
                int8 physical_id =
                    rai_allocate_itemp_physical_id(reg_num, pir2);

                if (ir2_opnd_is_itemp(popnd))
                    ir2_opnd_build(popnd, IR2_OPND_IREG, physical_id);
                else
                    ir2_opnd_build2(popnd, IR2_OPND_MEM, physical_id,
                                    ir2_opnd_imm(popnd));
            } else if (ir2_opnd_is_ftemp(popnd)) {
                int8 physical_id =
                    rai_allocate_ftemp_physical_id(reg_num, pir2);
                ir2_opnd_build(popnd, IR2_OPND_FREG, physical_id);
            }
        }
        pir2 = ir2_next(pir2);
    }

    return true;
}
