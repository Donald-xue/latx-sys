#include "common.h"
#include "reg_alloc.h"
#include "env.h"
#include "latx-options.h"

/* bool translate_pcmpeqb(IR1_INST * pir1) { return false; } */
/* bool translate_pcmpeqw(IR1_INST * pir1) { return false; } */
/* bool translate_pcmpeqd(IR1_INST * pir1) { return false; } */
/* bool translate_pcmpgtb(IR1_INST * pir1) { return false; } */
/* bool translate_pcmpgtw(IR1_INST * pir1) { return false; } */
/* bool translate_pcmpgtd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpeqpd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpneqpd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpunordpd(IR1_INST * pir1) { return false; } */
/* bool translate_cmplepd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpltpd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpleps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpltps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpeqps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpordpd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnlepd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnltpd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpltsd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpeqsd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpordps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnleps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnltps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpneqps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpunordps(IR1_INST * pir1) { return false; } */
/* bool translate_cmpordsd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnlesd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnltsd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpneqsd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpunordsd(IR1_INST * pir1) { return false; } */
/* bool translate_cmplesd(IR1_INST * pir1) { return false; } */
/* bool translate_cmpneqss(IR1_INST * pir1) { return false; } */
/* bool translate_cmpunordss(IR1_INST * pir1) { return false; } */
/* bool translate_cmpless(IR1_INST * pir1) { return false; } */
/* bool translate_cmpltss(IR1_INST * pir1) { return false; } */
/* bool translate_cmpeqss(IR1_INST * pir1) { return false; } */
/* bool translate_cmpordss(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnless(IR1_INST * pir1) { return false; } */
/* bool translate_cmpnltss(IR1_INST * pir1) { return false; } */
/* bool translate_comiss(IR1_INST * pir1) { return false; } */
/* bool translate_comisd(IR1_INST * pir1) { return false; } */
/* bool translate_ucomiss(IR1_INST * pir1) { return false; } */
/* bool translate_ucomisd(IR1_INST * pir1) { return false; } */

bool translate_pcmpeqb(IR1_INST *pir1)
{
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSEQ_B, dest, dest, src);
    } else { //mmx
        IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 1), false, true);
        la_append_ir2_opnd3(LISA_VSEQ_B, dest_lo, dest_lo, src_lo);
    }
    return true;
}

bool translate_pcmpeqw(IR1_INST *pir1)
{
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSEQ_H, dest, dest, src);
    } else { //mmx
        IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 1), false, true);
        la_append_ir2_opnd3(LISA_VSEQ_H, dest_lo, dest_lo, src_lo);
    }
    return true;
}

bool translate_pcmpeqd(IR1_INST *pir1)
{
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSEQ_W, dest, dest, src);
    } else { //mmx
        IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false, true);
        IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 1), false, true);
        la_append_ir2_opnd3(LISA_VSEQ_W, dest_lo, dest_lo, src_lo);
    }
    return true;
}

bool translate_pcmpgtb(IR1_INST *pir1)
{
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSLT_B, dest, src, dest);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSLT_B, dest_lo, src_lo, dest_lo);
    return true;
}

bool translate_pcmpgtw(IR1_INST *pir1)
{
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSLT_H, dest, src, dest);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSLT_H, dest_lo, src_lo, dest_lo);
    return true;
}

bool translate_pcmpgtd(IR1_INST *pir1)
{
    if(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0))){
        IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
        IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
        la_append_ir2_opnd3(LISA_VSLT_W, dest, src, dest);
        return true;
    }
    IR2_OPND dest_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0), false,
                                            true); /* fill default parameter */
    IR2_OPND src_lo = load_freg_from_ir1_1(ir1_get_opnd(pir1, 0) + 1, false,
                                           true); /* fill default parameter */
    la_append_ir2_opnd3(LISA_VSLT_W, dest_lo, src_lo, dest_lo);
    return true;
}

bool translate_cmpeqpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CEQ);
    return true;
}

bool translate_cmpltpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CLT);
    return true;
}

bool translate_cmplepd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CLE);
    return true;
}

bool translate_cmpunordpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CUN);
    return true;
}

bool translate_cmpneqpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CNE);
    return true;
}

bool translate_cmpnltpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CLE);
    return true;
}

bool translate_cmpnlepd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_CLT);
    return true;
}

bool translate_cmpordpd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, dest, dest, src, FCMP_COND_COR);
    return true;
}

bool translate_cmpeqps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, dest, src, FCMP_COND_CEQ);
    return true;
}

bool translate_cmpltps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, dest, src, FCMP_COND_CLT);
    return true;
}

bool translate_cmpleps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, dest, src, FCMP_COND_CLE);
    return true;
}

bool translate_cmpunordps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, dest, src, FCMP_COND_CUN);
    return true;
}

bool translate_cmpneqps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, dest, src, FCMP_COND_CNE);
    return true;
}

bool translate_cmpnltps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, src, dest, FCMP_COND_CLE);
    return true;
}

bool translate_cmpnleps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, src, dest, FCMP_COND_CLT);
    return true;
}

bool translate_cmpordps(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, dest, dest, src, FCMP_COND_COR);
    return true;
}

bool translate_cmpeqsd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CEQ);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpltsd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CLT);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmplesd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CLE);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpunordsd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CUN);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpneqsd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CNE);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpnltsd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CLT);
    IR2_OPND zero = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VXOR_V, zero, zero, zero);
    la_append_ir2_opnd3(LISA_VNOR_V, temp, temp, zero);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpnlesd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_CLE);
    IR2_OPND zero = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VXOR_V, zero, zero, zero);
    la_append_ir2_opnd3(LISA_VNOR_V, temp, temp, zero);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpordsd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_D, temp, dest, src, FCMP_COND_COR);
    la_append_ir2_opnd2i(LISA_XVINSVE0_D, dest, temp, 0);
    return true;
}

bool translate_cmpeqss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CEQ);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpltss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CLT);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpless(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CLE);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpunordss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CUN);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpneqss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CNE);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpnltss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CLT);
    IR2_OPND zero = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VXOR_V, zero, zero, zero);
    la_append_ir2_opnd3(LISA_VNOR_V, temp, temp, zero);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpnless(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_CLE);
    IR2_OPND zero = ra_alloc_ftemp();
    la_append_ir2_opnd3(LISA_VXOR_V, zero, zero, zero);
    la_append_ir2_opnd3(LISA_VNOR_V, temp, temp, zero);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_cmpordss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_is_xmm(ir1_get_opnd(pir1, 0)));
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND temp = ra_alloc_ftemp();
    la_append_ir2_opnd3i(LISA_VFCMP_COND_S, temp, dest, src, FCMP_COND_COR);
    la_append_ir2_opnd2i(LISA_XVINSVE0_W, dest, temp, 0);
    return true;
}

bool translate_comisd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_num(pir1) == 2);
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0)); /* fill default parameter */
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1)); /* fill default parameter */
    IR2_OPND label_nun = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_neq = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL); 

    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x3f);

    /*  case 1: are they unordered? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_nun);
    /* at least one of the operands is NaN */
    // set zf,pf,cf = 111
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0xb);
    la_append_ir2_opnd1(LISA_B, label_exit); 

    /* case 2: not unordered. are they equal? */
    la_append_ir2_opnd1(LISA_LABEL, label_nun);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_neq);
    /* two operands are equal */
    // set zf,pf,cf = 100
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);

    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 3: not unordered, not equal. less than? */
    la_append_ir2_opnd1(LISA_LABEL, label_neq);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_exit);
    /* less than */
    // set zf.pf.cf = 001
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);

    la_append_ir2_opnd1(LISA_B, label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    /* set zf,pf,cf = 000, as we have set all eflags to 0, so just exit */

    /* exit */
    //set of,sf,af = 000
    la_append_ir2_opnd1(LISA_LABEL,label_exit);

    return true;
}

bool translate_comiss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_num(pir1) == 2);
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND label_nun = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_neq = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL);

    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x3f);

    /*  case 1: are they unordered? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_S, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_nun);
    /* at least one of the operands is NaN */
    // set zf,pf,cf = 111
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0xb);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 2: not unordered. are they equal? */
    la_append_ir2_opnd1(LISA_LABEL, label_nun);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_S, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_neq);
    /* two operands are equal */
    // set zf,pf,cf = 100
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 3: not unordered, not equal. less than? */
    la_append_ir2_opnd1(LISA_LABEL, label_neq);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_S, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_exit);
    /* less than */
    // set zf.pf.cf = 001
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    /* set zf,pf,cf = 000, as we have set all eflags to 0, so just exit */

    /* exit */
    //set of,sf,af = 000
    la_append_ir2_opnd1(LISA_LABEL,label_exit);

    return true;
}

bool translate_ucomisd(IR1_INST *pir1)
{
    lsassert(ir1_opnd_num(pir1) == 2);
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND label_nun = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_neq = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_gt = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL); 

    /*  case 1: are they unordered? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_nun);
    /* at least one of the operands is NaN */
    // set zf,pf,cf = 111
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0xb);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 2: not unordered. are they equal? */
    la_append_ir2_opnd1(LISA_LABEL, label_nun);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_neq);
    /* two operands are equal */
    // set zf,pf,cf = 100
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x3);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 3: not unordered, not equal. less than? */
    la_append_ir2_opnd1(LISA_LABEL, label_neq);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_D, 
        fcc0_ir2_opnd, dest, src, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_gt);
    /* less than */
    // set zf.pf.cf = 001
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0xa);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    //set zf,pf,cf = 000
    la_append_ir2_opnd1(LISA_LABEL,label_gt);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0xb);

    /* exit */
    //set of,sf,af = 000
    la_append_ir2_opnd1(LISA_LABEL,label_exit);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x34);

    return true;    
}

bool translate_ucomiss(IR1_INST *pir1)
{
    lsassert(ir1_opnd_num(pir1) == 2);
    IR2_OPND dest = load_freg128_from_ir1(ir1_get_opnd(pir1, 0));
    IR2_OPND src = load_freg128_from_ir1(ir1_get_opnd(pir1, 1));
    IR2_OPND label_nun = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_neq = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_gt = ir2_opnd_new_type(IR2_OPND_LABEL); 
    IR2_OPND label_exit = ir2_opnd_new_type(IR2_OPND_LABEL); 

    /*  case 1: are they unordered? */
    la_append_ir2_opnd3i(LISA_FCMP_COND_S,
        fcc0_ir2_opnd, dest, src, FCMP_COND_CUN);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_nun);
    /* at least one of the operands is NaN */
    // set zf,pf,cf = 111
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0xb);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 2: not unordered. are they equal? */
    la_append_ir2_opnd1(LISA_LABEL, label_nun);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_S,
        fcc0_ir2_opnd, dest, src, FCMP_COND_CEQ);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_neq);
    /* two operands are equal */
    // set zf,pf,cf = 100
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x8);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x3);
    la_append_ir2_opnd1(LISA_B, label_exit);

    /* case 3: not unordered, not equal. less than? */
    la_append_ir2_opnd1(LISA_LABEL, label_neq);  
    la_append_ir2_opnd3i(LISA_FCMP_COND_S,
        fcc0_ir2_opnd, dest, src, FCMP_COND_CLT);
    la_append_ir2_opnd2(LISA_BCEQZ, fcc0_ir2_opnd, label_gt);
    /* less than */
    // set zf.pf.cf = 001
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0xa);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, n1_ir2_opnd, 0x1);

    la_append_ir2_opnd1(LISA_B, label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    //set zf,pf,cf = 000
    la_append_ir2_opnd1(LISA_LABEL, label_gt);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0xb);

    /* exit*/
    //set of,sf,af = 000
    la_append_ir2_opnd1(LISA_LABEL,label_exit);
    la_append_ir2_opnd1i(LISA_X86MTFLAG, zero_ir2_opnd, 0x34);

    return true;   
}

