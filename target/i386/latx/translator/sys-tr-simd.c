#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include "sys-excp.h"
#include <string.h>

void latxs_sys_simd_register_ir1(void)
{
    latxs_register_ir1(X86_INS_POR);
    latxs_register_ir1(X86_INS_PXOR);

    latxs_register_ir1(X86_INS_PACKUSWB);

    latxs_register_ir1(X86_INS_PADDB);
    latxs_register_ir1(X86_INS_PADDW);
    latxs_register_ir1(X86_INS_PADDD);
    latxs_register_ir1(X86_INS_PADDQ);

    latxs_register_ir1(X86_INS_PADDUSB);
    latxs_register_ir1(X86_INS_PADDUSW);

    latxs_register_ir1(X86_INS_PAND);
    latxs_register_ir1(X86_INS_PANDN);

    latxs_register_ir1(X86_INS_PCMPEQB);
    latxs_register_ir1(X86_INS_PCMPEQW);
    latxs_register_ir1(X86_INS_PCMPEQD);
    latxs_register_ir1(X86_INS_PCMPGTB);
    latxs_register_ir1(X86_INS_PCMPGTW);
    latxs_register_ir1(X86_INS_PCMPGTD);

    latxs_register_ir1(X86_INS_PMULLW);

    latxs_register_ir1(X86_INS_PSLLW);
    latxs_register_ir1(X86_INS_PSLLD);
    latxs_register_ir1(X86_INS_PSLLQ);
    latxs_register_ir1(X86_INS_PSRLW);
    latxs_register_ir1(X86_INS_PSRLD);
    latxs_register_ir1(X86_INS_PSRLQ);
    latxs_register_ir1(X86_INS_PSRLDQ);

    latxs_register_ir1(X86_INS_PSUBB);
    latxs_register_ir1(X86_INS_PSUBW);
    latxs_register_ir1(X86_INS_PSUBD);
    latxs_register_ir1(X86_INS_PSUBQ);

    latxs_register_ir1(X86_INS_PUNPCKHBW);
    latxs_register_ir1(X86_INS_PUNPCKHWD);
    latxs_register_ir1(X86_INS_PUNPCKHDQ);
    latxs_register_ir1(X86_INS_PUNPCKLBW);
    latxs_register_ir1(X86_INS_PUNPCKLWD);
    latxs_register_ir1(X86_INS_PUNPCKLDQ);

    latxs_register_ir1(X86_INS_ADDSD);
    latxs_register_ir1(X86_INS_ADDSS);

    latxs_register_ir1(X86_INS_ANDPS);

    latxs_register_ir1(X86_INS_CMPEQPD);
    latxs_register_ir1(X86_INS_CMPLTPD);
    latxs_register_ir1(X86_INS_CMPLEPD);
    latxs_register_ir1(X86_INS_CMPUNORDPD);
    latxs_register_ir1(X86_INS_CMPNEQPD);
    latxs_register_ir1(X86_INS_CMPNLTPD);
    latxs_register_ir1(X86_INS_CMPNLEPD);
    latxs_register_ir1(X86_INS_CMPORDPD);
    latxs_register_ir1(X86_INS_CMPEQPS);
    latxs_register_ir1(X86_INS_CMPLTPS);
    latxs_register_ir1(X86_INS_CMPLEPS);
    latxs_register_ir1(X86_INS_CMPUNORDPS);
    latxs_register_ir1(X86_INS_CMPNEQPS);
    latxs_register_ir1(X86_INS_CMPNLTPS);
    latxs_register_ir1(X86_INS_CMPNLEPS);
    latxs_register_ir1(X86_INS_CMPORDPS);
    latxs_register_ir1(X86_INS_CMPEQSD);
    latxs_register_ir1(X86_INS_CMPLTSD);
    latxs_register_ir1(X86_INS_CMPLESD);
    latxs_register_ir1(X86_INS_CMPUNORDSD);
    latxs_register_ir1(X86_INS_CMPNEQSD);
    latxs_register_ir1(X86_INS_CMPNLTSD);
    latxs_register_ir1(X86_INS_CMPNLESD);
    latxs_register_ir1(X86_INS_CMPORDSD);
    latxs_register_ir1(X86_INS_CMPEQSS);
    latxs_register_ir1(X86_INS_CMPLTSS);
    latxs_register_ir1(X86_INS_CMPLESS);
    latxs_register_ir1(X86_INS_CMPUNORDSS);
    latxs_register_ir1(X86_INS_CMPNEQSS);
    latxs_register_ir1(X86_INS_CMPNLTSS);
    latxs_register_ir1(X86_INS_CMPNLESS);
    latxs_register_ir1(X86_INS_CMPORDSS);

    latxs_register_ir1(X86_INS_COMISD);
    latxs_register_ir1(X86_INS_COMISS);
    latxs_register_ir1(X86_INS_UCOMISD);
    latxs_register_ir1(X86_INS_UCOMISS);

    latxs_register_ir1(X86_INS_CVTDQ2PD);
    latxs_register_ir1(X86_INS_CVTDQ2PS);
    latxs_register_ir1(X86_INS_CVTPD2PS);
    latxs_register_ir1(X86_INS_CVTSD2SS);
    latxs_register_ir1(X86_INS_CVTSI2SD);
    latxs_register_ir1(X86_INS_CVTSS2SD);
    latxs_register_ir1(X86_INS_CVTTSS2SI);

    latxs_register_ir1(X86_INS_DIVSD);
    latxs_register_ir1(X86_INS_DIVSS);

    latxs_register_ir1(X86_INS_MAXPD);
    latxs_register_ir1(X86_INS_MAXPS);
    latxs_register_ir1(X86_INS_MAXSD);
    latxs_register_ir1(X86_INS_MAXSS);
    latxs_register_ir1(X86_INS_MINPD);
    latxs_register_ir1(X86_INS_MINPS);
    latxs_register_ir1(X86_INS_MINSD);
    latxs_register_ir1(X86_INS_MINSS);

    latxs_register_ir1(X86_INS_MOVNTPD);
    latxs_register_ir1(X86_INS_MOVNTPS);
    latxs_register_ir1(X86_INS_MOVNTQ);

    latxs_register_ir1(X86_INS_MULPD);
    latxs_register_ir1(X86_INS_MULPS);
    latxs_register_ir1(X86_INS_MULSD);
    latxs_register_ir1(X86_INS_MULSS);

    latxs_register_ir1(X86_INS_ORPD);
    latxs_register_ir1(X86_INS_ORPS);

    latxs_register_ir1(X86_INS_PEXTRW);

    latxs_register_ir1(X86_INS_PMAXSW);
    latxs_register_ir1(X86_INS_PMAXUB);
    latxs_register_ir1(X86_INS_PMINSW);
    latxs_register_ir1(X86_INS_PMINUB);

    latxs_register_ir1(X86_INS_PMOVMSKB);
    latxs_register_ir1(X86_INS_PMULUDQ);

    latxs_register_ir1(X86_INS_PSHUFD);
    latxs_register_ir1(X86_INS_PSHUFW);
    latxs_register_ir1(X86_INS_PSHUFLW);
}

#define XMM_REG(o) ir1_opnd_base_reg_num(o)
#define XMM_LOAD128(o) latxs_load_freg128_from_ir1(o)
#define XMM_LOADFP1(o) latxs_load_freg_from_ir1_1(o, false, true)

#define XMM_EXCP(pir1) do {                             \
    if (latxs_tr_gen_sse_common_excp_check(pir1)) {     \
        return true;                                    \
    }                                                   \
} while (0)

bool latxs_translate_por(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VOR_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VOR_V, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_pxor(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0) && ir1_opnd_is_mem(opnd1)) {
        IR2_OPND temp = latxs_ra_alloc_ftemp();
        latxs_load_freg128_from_ir1_mem(&temp, opnd1);
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(opnd0));
        latxs_append_ir2_opnd3(LISA_VXOR_V, &xmm_dest, &xmm_dest, &temp);
        return true;
    } else if (ir1_opnd_is_xmm(opnd0) && ir1_opnd_is_xmm(opnd1)) {
        IR2_OPND xmm_dest = latxs_ra_alloc_xmm(XMM_REG(opnd0));
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(opnd1));
        latxs_append_ir2_opnd3(LISA_VXOR_V, &xmm_dest, &xmm_dest, &xmm_src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);

    latxs_append_ir2_opnd3(LISA_VXOR_V, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_packuswb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        if (ir1_opnd_is_xmm(opnd1) &&
                (XMM_REG(opnd0) == XMM_REG(opnd1))) {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &dest, &dest, 0);
            latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                    VEXTRINS_IMM_4_0(1, 0));
        } else {
            latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &dest, &dest, 0);
            IR2_OPND temp = latxs_ra_alloc_ftemp();
            latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &temp, &src, 0);
            latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp,
                    VEXTRINS_IMM_4_0(1, 0));
        }
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);

        latxs_append_ir2_opnd3(LISA_VPACKEV_D, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VSSRANI_BU_H, &dest, &dest, 0);
    }

    return true;
}

bool latxs_translate_paddb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_B, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VADD_B, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_paddw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_H, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VADD_H, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_paddd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_W, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VADD_W, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_paddusb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSADD_BU, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VSADD_BU, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_paddusw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSADD_HU, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VSADD_HU, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_pand(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VAND_V, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_pandn(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VANDN_V, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VANDN_V, &dest_lo, &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_pcmpeqb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_B, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_B, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpeqw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_H, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_H, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpeqd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_W, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSEQ_W, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpgtb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_B, &dest, &src, &dest);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_B, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpgtw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_H, &dest, &src, &dest);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_H, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pcmpgtd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_W, &dest, &src, &dest);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSLT_W, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_pmullw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMUL_H, &dest, &dest, &src);
    } else {
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VMUL_H, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_psllw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        if (ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1)) {
            IR2_OPND src = XMM_LOAD128(opnd1);
            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(1, 16));
            latxs_append_ir2_opnd3(LISA_VSLT_HU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSLL_H, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        } else if (ir1_opnd_is_imm(opnd1)) {
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 15) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSLLI_H, &dest, &dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { /* mmx */
        if (ir1_opnd_is_imm(opnd0 + 1)) {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            uint8_t imm = ir1_opnd_uimm(opnd0 + 1);
            if (imm > 15) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSLLI_H, &dest, &dest, imm);
            }
        } else {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            IR2_OPND src  = XMM_LOADFP1(opnd1);

            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(1, 16));
            latxs_append_ir2_opnd3(LISA_VSLT_HU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSLL_H, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        }
    }
    return true;
}

bool latxs_translate_pslld(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        if (ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1)) {
            IR2_OPND src = XMM_LOAD128(opnd1);
            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(2, 32));
            latxs_append_ir2_opnd3(LISA_VSLT_WU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSLL_W, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        } else if (ir1_opnd_is_imm(opnd1)) {
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 31) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSLLI_W, &dest, &dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { /* mmx */
        if (ir1_opnd_is_imm(opnd0 + 1)) {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            uint8_t imm = ir1_opnd_uimm(opnd0 + 1);
            if (imm > 31) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSLLI_W, &dest, &dest, imm);
            }
        } else {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            IR2_OPND src  = XMM_LOADFP1(opnd1);

            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(2, 32));
            latxs_append_ir2_opnd3(LISA_VSLT_WU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSLL_W, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        }
    }
    return true;
}

bool latxs_translate_psllq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        if (ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1)) {
            IR2_OPND src = XMM_LOAD128(opnd1);
            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(3, 64));
            latxs_append_ir2_opnd3(LISA_VSLT_DU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSLL_D, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        } else if (ir1_opnd_is_imm(opnd1)) {
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 63) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSLLI_D, &dest, &dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { /* mmx */
        if (ir1_opnd_is_imm(opnd0 + 1)) {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            uint8_t imm = ir1_opnd_uimm(opnd0 + 1);
            if (imm > 63) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSLLI_D, &dest, &dest, imm);
            }
        } else {
            IR2_OPND temp_imm = latxs_ra_alloc_itemp();
            IR2_OPND temp     = latxs_ra_alloc_itemp();
            IR2_OPND target_label = latxs_ir2_opnd_new_label();

            IR2_OPND dest = XMM_LOADFP1(opnd0);
            IR2_OPND src  = XMM_LOADFP1(opnd1);

            IR2_OPND *zero = &latxs_zero_ir2_opnd;
            latxs_load_imm64_to_ir2(&temp_imm, 0xffffffffffffff80ULL);
            latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &temp, &src);
            latxs_append_ir2_opnd3(LISA_AND, &temp_imm, &temp_imm, &temp);
            latxs_append_ir2_opnd3(LISA_BEQ, &temp_imm, zero, &target_label);
            latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &dest, zero);
            latxs_append_ir2_opnd1(LISA_LABEL, &target_label);
            latxs_append_ir2_opnd3(LISA_VSLL_D, &dest, &dest, &src);

            latxs_ra_free_temp(&temp_imm);
            latxs_ra_free_temp(&temp);
        }
    }
    return true;
}

bool latxs_translate_psrlw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        if (ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1)) {
            IR2_OPND src = XMM_LOAD128(opnd1);
            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(1, 16));
            latxs_append_ir2_opnd3(LISA_VSLT_HU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSRL_H, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        } else if (ir1_opnd_is_imm(opnd1)) {
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 15) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSRLI_H, &dest, &dest, imm);
            }
        } else {
            lsassert(0);
        }
        return true;
    } else {
        if (ir1_opnd_is_imm(opnd1)) {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            uint8_t imm = ir1_opnd_uimm(opnd0 + 1);
            if (imm > 15) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSRLI_H, &dest, &dest, imm);
            }
        } else {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            IR2_OPND src  = XMM_LOADFP1(opnd1);

            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(1, 16));
            latxs_append_ir2_opnd3(LISA_VSLT_HU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_H, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSRL_H, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        }
    }
    return true;
}

bool latxs_translate_psrld(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        if (ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1)) {
            IR2_OPND src = XMM_LOAD128(opnd1);
            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(2, 32));
            latxs_append_ir2_opnd3(LISA_VSLT_WU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSRL_W, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        } else if (ir1_opnd_is_imm(opnd1)) {
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 31) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSRLI_W, &dest, &dest, imm);
            }
        } else {
            lsassert(0);
        }
        return true;
    } else {
        if (ir1_opnd_is_imm(opnd1)) {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 31) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VXOR_V, &dest, &dest, imm);
            }
       } else {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            IR2_OPND src  = XMM_LOADFP1(opnd1);

            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(2, 32));
            latxs_append_ir2_opnd3(LISA_VSLT_WU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_W, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSRL_W, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
       }
    }

    return true;
}

bool latxs_translate_psrlq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        if (ir1_opnd_is_xmm(opnd1) || ir1_opnd_is_mem(opnd1)) {
            IR2_OPND src = XMM_LOAD128(opnd1);
            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2, VLDI_IMM_TYPE0(3, 64));
            latxs_append_ir2_opnd3(LISA_VSLT_DU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd3(LISA_VSRL_D, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        } else if (ir1_opnd_is_imm(opnd1)) {
            uint8_t imm = ir1_opnd_uimm(opnd1);
            if (imm > 63) {
                latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
            } else {
                latxs_append_ir2_opnd2i(LISA_VSRLI_D, &dest, &dest, imm);
            }
        } else {
            lsassert(0);
        }
    } else { /* mmx */
        if (ir1_opnd_is_imm(opnd0 + 1)) {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            uint8_t imm = ir1_opnd_uimm(opnd0 + 1);
            if (imm > 63) {
                latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &dest,
                        &latxs_zero_ir2_opnd);
            } else {
                IR2_OPND itemp = latxs_ra_alloc_itemp();
                latxs_load_imm32_to_ir2(&itemp, imm, EXMode_N);
                latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &itemp, &dest);
                latxs_append_ir2_opnd2i(LISA_SRLI_D, &itemp, &itemp, imm);
                latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &dest, &itemp);
                latxs_ra_free_temp(&itemp);
            }
        } else {
            IR2_OPND dest = XMM_LOADFP1(opnd0);
            IR2_OPND src  = XMM_LOADFP1(opnd1);

            IR2_OPND temp1 = latxs_ra_alloc_ftemp();
            IR2_OPND temp2 = latxs_ra_alloc_ftemp();
            IR2_OPND temp3 = latxs_ra_alloc_ftemp();

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd1i(LISA_VLDI, &temp2,
                                         VLDI_IMM_TYPE0(3, 64));
            latxs_append_ir2_opnd3(LISA_VSLT_DU, &temp3, &temp1, &temp2);

            latxs_append_ir2_opnd2i(LISA_VREPLVEI_D, &temp1, &src, 0);
            latxs_append_ir2_opnd3(LISA_VSRL_D, &dest, &dest, &temp1);
            latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &temp3);
        }
    }

    return true;
}

bool latxs_translate_psubb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_B, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_B, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_psubw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_H, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_H, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_psubd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_W, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_W, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_punpckhbw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKHBW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVH_B, &dest, &src, &dest);
    } else {
        /* PUNPCKHBW mm, mm/m64 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                                     VEXTRINS_IMM_4_0(0, 1));
    }

    return true;
}

bool latxs_translate_punpckhwd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKHWD xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVH_H, &dest, &src, &dest);
    } else {
        /* PUNPCKHWD mm, mm/m64 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_H, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                                     VEXTRINS_IMM_4_0(0, 1));
    }

    return true;
}

bool latxs_translate_punpckhdq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKHDQ xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVH_W, &dest, &src, &dest);
    } else {
        /* PUNPCKHDQ mm, mm/m64 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &src, &dest);
        latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &dest,
                                     VEXTRINS_IMM_4_0(0, 1));
    }

    return true;
}

bool latxs_translate_punpcklbw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKLBW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
    } else {
        /* PUNPCKLBW mm, mm/m32 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
        latxs_append_ir2_opnd3(LISA_VILVL_B, &dest, &src, &dest);
    }

    return true;
}

bool latxs_translate_punpcklwd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKLWD xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_H, &dest, &src, &dest);
    } else {
        /* PUNPCKLWD mm, mm/m32 */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
        latxs_append_ir2_opnd3(LISA_VILVL_H, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_punpckldq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PUNPCKLQDQ xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &src, &dest);
    } else {
        /*PUNPCKLDQ mm, mm/m32  */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src  = load_freg_from_ir1_1(opnd1, false, false);
        latxs_append_ir2_opnd3(LISA_VILVL_W, &dest, &dest, &src);
    }

    return true;
}

bool latxs_translate_addsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd3(LISA_FADD_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_addss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd3(LISA_FADD_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_andps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VAND_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_cmpeqpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CEQ);
    return true;
}

bool latxs_translate_cmpltpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CEQ);
    return true;
}

bool latxs_translate_cmplepd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CLE);
    return true;
}

bool latxs_translate_cmpunordpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CUN);
    return true;
}

bool latxs_translate_cmpneqpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CNE);
    return true;
}

bool latxs_translate_cmpnltpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CLE);
    return true;
}

bool latxs_translate_cmpnlepd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_CLT);
    return true;
}

bool latxs_translate_cmpordpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &dest, &dest, &src,
                                  FCMP_COND_COR);
    return true;
}

bool latxs_translate_cmpeqps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CEQ);
    return true;
}

bool latxs_translate_cmpltps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CEQ);
    return true;
}

bool latxs_translate_cmpleps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CLE);
    return true;
}

bool latxs_translate_cmpunordps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CUN);
    return true;
}

bool latxs_translate_cmpneqps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CNE);
    return true;
}

bool latxs_translate_cmpnltps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CLE);
    return true;
}

bool latxs_translate_cmpnleps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_CLT);
    return true;
}

bool latxs_translate_cmpordps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &dest, &dest, &src,
                                  FCMP_COND_COR);
    return true;
}

bool latxs_translate_cmpeqsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CEQ);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpltsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CLT);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmplesd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CLE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpunordsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CUN);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpneqsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CNE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnltsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CLT);

    IR2_OPND zero = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VXOR_V, &zero, &zero, &zero);
    latxs_append_ir2_opnd3(LISA_VNOR_V, &temp, &temp, &zero);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnlesd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CLE);

    IR2_OPND zero = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VXOR_V, &zero, &zero, &zero);
    latxs_append_ir2_opnd3(LISA_VNOR_V, &temp, &temp, &zero);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpordsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_COR);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpeqss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_D, &temp, &dest, &src,
                                  FCMP_COND_CEQ);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpltss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CLT);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpless(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CLE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpunordss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CUN);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpneqss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CNE);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnltss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CLT);

    IR2_OPND zero = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VXOR_V, &zero, &zero, &zero);
    latxs_append_ir2_opnd3(LISA_VNOR_V, &temp, &temp, &zero);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpnless(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_CLE);

    IR2_OPND zero = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VXOR_V, &zero, &zero, &zero);
    latxs_append_ir2_opnd3(LISA_VNOR_V, &temp, &temp, &zero);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_cmpordss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3i(LISA_VFCMP_COND_S, &temp, &dest, &src,
                                  FCMP_COND_COR);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_comisd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    lsassert(ir1_opnd_num(pir1) == 2);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND label_nun  = latxs_ir2_opnd_new_label();
    IR2_OPND label_neq  = latxs_ir2_opnd_new_label();
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();

    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND allone = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_NOR, &allone, zero, zero);

    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x3f);

    /*  case 1: are they unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                    &dest, &src, FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_nun);
    /* at least one of the operands is NaN */
    /* set zf,pf,cf = 111 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0xb);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 2: not unordered. are they equal? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_nun);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                    &dest, &src, FCMP_COND_CEQ);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_neq);
    /* two operands are equal */
    /* set zf,pf,cf = 100 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x8);

    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 3: not unordered, not equal. less than? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_neq);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0,
                    &dest, &src, FCMP_COND_CLT);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_exit);
    /* less than */
    /* set zf.pf.cf = 001 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x1);

    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    /* set zf,pf,cf = 000, as we have set all eflags to 0, */
    /* so just exit */

    /* exit */
    /* set of,sf,af = 000 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    latxs_ra_free_temp(&allone);
    return true;
}

bool latxs_translate_comiss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    lsassert(ir1_opnd_num(pir1) == 2);
    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND label_nun  = latxs_ir2_opnd_new_label();
    IR2_OPND label_neq  = latxs_ir2_opnd_new_label();
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();

    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND allone = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_NOR, &allone, zero, zero);

    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x3f);

    /*  case 1: are they unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, fcc0,
                    &dest, &src, FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_nun);
    /* at least one of the operands is NaN */
    /* set zf,pf,cf = 111 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0xb);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 2: not unordered. are they equal? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_nun);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, fcc0,
                    &dest, &src, FCMP_COND_CEQ);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_neq);
    /* two operands are equal */
    /* set zf,pf,cf = 100 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x8);

    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 3: not unordered, not equal. less than? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_neq);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, fcc0,
                    &dest, &src, FCMP_COND_CLT);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_exit);
    /* less than */
    /* set zf.pf.cf = 001 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x1);

    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    /* set zf,pf,cf = 000, as we have set all eflags to 0, */
    /* so just exit */

    /* exit */
    /* set of,sf,af = 000 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);

    latxs_ra_free_temp(&allone);
    return true;
}

bool latxs_translate_cvtdq2pd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd2(LISA_VFFINTL_D_W, &dest, &src);
    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtdq2ps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd2(LISA_VFFINT_S_W, &dest, &src);
    return true;
}

bool latxs_translate_cvtpd2ps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFCVT_S_D, &dest, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVPICKVE_D, &dest, &dest, 0);
    return true;
}

bool latxs_translate_cvtsd2ss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    /* TODO:simply */
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FCVT_S_D, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);

    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtsi2sd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND fcsr_opnd = latxs_set_fpu_fcsr_rounding_field_by_x86();
    /* TODO:simply */
    IR2_OPND dest = XMM_LOAD128(opnd0);

    IR2_OPND src = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src, opnd1, EXMode_S, false);

    IR2_OPND temp_src = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &temp_src, &src);

    if (ir1_opnd_size(opnd0 + 1) == 64) {
        latxs_append_ir2_opnd2(LISA_FFINT_D_L, &temp_src, &temp_src);
    } else {
        latxs_append_ir2_opnd2(LISA_FFINT_D_W, &temp_src, &temp_src);
    }

    latxs_append_ir2_opnd2i(LISA_VEXTRINS_D, &dest, &temp_src,
                                 VEXTRINS_IMM_4_0(0, 0));
    latxs_set_fpu_fcsr(&fcsr_opnd);
    return true;
}

bool latxs_translate_cvtss2sd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));

    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();

    latxs_append_ir2_opnd2(LISA_FCVT_D_S, &temp, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);

    return true;
}

bool latxs_translate_cvttss2si(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND *zero = &latxs_zero_ir2_opnd;
    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *fcsr = &latxs_fcsr_ir2_opnd;

    IR2_OPND src = XMM_LOAD128(opnd1);

    IR2_OPND src_lo = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2(LISA_FCVT_D_S, &src_lo, &src);

    IR2_OPND temp_over_flow  = latxs_ra_alloc_itemp();
    IR2_OPND ftemp_over_flow = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2i(LISA_LU52I_D, &temp_over_flow, zero, 0x41e);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp_over_flow,
                                            &temp_over_flow);

    IR2_OPND temp_under_flow  = latxs_ra_alloc_itemp();
    IR2_OPND ftemp_under_flow = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd2i(LISA_LU52I_D, &temp_under_flow, zero, 0xc1e);
    latxs_append_ir2_opnd2(LISA_MOVGR2FR_D, &ftemp_under_flow,
                                            &temp_under_flow);

    /*is unoder?*/
    IR2_OPND label_for_flow = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &src_lo, &src_lo,
                                 FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    /*is over flow or under flow*/
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &ftemp_over_flow,
                        &src_lo, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &src_lo,
              &ftemp_under_flow, FCMP_COND_CLE);
    latxs_append_ir2_opnd2(LISA_BCNEZ, fcc0, &label_for_flow);

    /*not over flow and under flow*/
    IR2_OPND temp_fcsr = latxs_ra_alloc_itemp();

    /*save fscr in temp_int for reload*/
    latxs_append_ir2_opnd2(LISA_MOVFCSR2GR, &temp_fcsr, fcsr);
    latxs_append_ir2_opnd3(LISA_OR, &temp_under_flow,
                                    &temp_under_flow, &temp_fcsr);

    /*set fscr for rounding to zero according to x86 operation*/
    latxs_append_ir2_opnd2ii(LISA_BSTRINS_W, &temp_fcsr, zero, 9, 8);
    latxs_append_ir2_opnd2i(LISA_ORI, &temp_fcsr, &temp_fcsr, 0x1 << 8);
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr, &temp_fcsr);

    if (ir1_opnd_size(ir1_get_opnd(pir1, 0)) == 64) {
        latxs_append_ir2_opnd2(LISA_FTINT_L_D, &ftemp_over_flow, &src_lo);
    } else {
        latxs_append_ir2_opnd2(LISA_FTINT_W_D, &ftemp_over_flow, &src_lo);
    }

    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &temp_fcsr,
                                            &ftemp_over_flow);
    /* reload fcsr */
    latxs_append_ir2_opnd2(LISA_MOVGR2FCSR, fcsr,
                                            &temp_under_flow);

    IR2_OPND label_for_exit = latxs_ir2_opnd_new_label();
    latxs_append_ir2_opnd1(LISA_B, &label_for_exit);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_flow);
    latxs_load_imm32_to_ir2(&temp_fcsr, 0x80000000, EXMode_Z);
    latxs_append_ir2_opnd1(LISA_LABEL, &label_for_exit);
    latxs_store_ir2_to_ir1(&temp_fcsr, opnd0, false);

    latxs_ra_free_temp(&temp_fcsr);
    latxs_ra_free_temp(&temp_over_flow);
    latxs_ra_free_temp(&temp_under_flow);
    latxs_ra_free_temp(&ftemp_over_flow);
    latxs_ra_free_temp(&ftemp_under_flow);

    return true;
}

bool latxs_translate_divsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FDIV_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_divss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FDIV_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_maxpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMAX_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_maxps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMAX_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_maxsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMAX_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_maxss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMAX_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_minpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMIN_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_minps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMIN_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_minsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMIN_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_minss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_VFMIN_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_movntpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (option_xmm128map) {
        IR1_OPND *dest = opnd0;
        IR1_OPND *src  = opnd1;

        if (ir1_opnd_is_xmm(src)) {
            IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(src));
            latxs_store_freg128_to_ir1_mem(&src_ir2, dest);
            return true;
        }
    }

    IR2_OPND src_lo = XMM_LOADFP1(opnd1);
    IR2_OPND src_hi = latxs_load_freg_from_ir1_1(opnd1, true, true);

    latxs_store_freg_to_ir1(&src_lo, opnd0, false, true);
    latxs_store_freg_to_ir1(&src_hi, opnd0, true, true);

    return true;
}

bool latxs_translate_movntps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (option_xmm128map) {
        IR1_OPND *dest = opnd0;
        IR1_OPND *src  = opnd1;

        if (ir1_opnd_is_xmm(src)) {
            IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(src));
            latxs_store_freg128_to_ir1_mem(&src_ir2, dest);
            return true;
        }
    }

    IR2_OPND src_lo = XMM_LOADFP1(opnd1);
    IR2_OPND src_hi = latxs_load_freg_from_ir1_1(opnd1, true, true);

    latxs_store_freg_to_ir1(&src_lo, opnd0, false, true);
    latxs_store_freg_to_ir1(&src_hi, opnd0, true, true);

    return true;
}

bool latxs_translate_movntq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (option_xmm128map) {
        IR1_OPND *dest = opnd0;
        IR1_OPND *src  = opnd1;

        if (ir1_opnd_is_xmm(src)) {
            IR2_OPND src_ir2 = latxs_ra_alloc_xmm(XMM_REG(src));
            latxs_store_freg_to_ir1(&src_ir2, dest, false, false);
            return true;
        }
    }

    IR2_OPND src_lo = XMM_LOADFP1(opnd1);
    latxs_store_freg_to_ir1(&src_lo, opnd0, false, true);

    return true;
}

bool latxs_translate_mulpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMUL_D, &dest, &dest, &src);
    return true;
}

bool latxs_translate_mulps(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VFMUL_S, &dest, &dest, &src);
    return true;
}

bool latxs_translate_mulsd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FMUL_D, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 0);
    return true;
}

bool latxs_translate_mulss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    IR2_OPND temp = latxs_ra_alloc_ftemp();
    latxs_append_ir2_opnd3(LISA_FMUL_S, &temp, &dest, &src);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_W, &dest, &temp, 0);
    return true;
}

bool latxs_translate_orpd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    latxs_append_ir2_opnd3(LISA_VOR_V, &dest, &dest, &src);
    return true;
}

bool latxs_translate_orps(IR1_INST *pir1)
{
    latxs_translate_orpd(pir1);
    return true;
}

bool latxs_translate_paddq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VADD_D, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);

    latxs_append_ir2_opnd3(LISA_VADD_D, &dest_lo, &dest_lo, &src_lo);

    return true;
}

bool latxs_translate_pextrw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    if (ir1_opnd_is_xmm(opnd1)) {
        uint8_t imm = ir1_opnd_uimm(opnd2);
        imm &= 7;
        IR2_OPND gpr = latxs_ra_alloc_gpr(XMM_REG(opnd0));
        IR2_OPND xmm = latxs_ra_alloc_xmm(XMM_REG(opnd1));
        latxs_append_ir2_opnd2i(LISA_VPICKVE2GR_HU, &gpr, &xmm, imm);
        return true;
    }

    IR2_OPND src_lo = latxs_ra_alloc_itemp();
    latxs_load_ir1_to_ir2(&src_lo, opnd1, EXMode_N, false);

    uint8_t imm = ir1_opnd_uimm(opnd2);
    uint8_t select = imm & 0x3;
    switch (select) {
    case 0:
        break;
    case 1:
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &src_lo, &src_lo, 0x10);
        break;
    case 2:
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &src_lo, &src_lo, 0x20);
        break;
    case 3:
        latxs_append_ir2_opnd2i(LISA_SRLI_D, &src_lo, &src_lo, 0x30);
        break;
    default:
        fprintf(stderr, "1: invalid imm8<0:1> in PEXTRW : %d\n", select);
        exit(-1);
    }
    latxs_append_ir2_opnd2ii(LISA_BSTRPICK_W, &src_lo, &src_lo, 15, 0);
    latxs_store_ir2_to_ir1(&src_lo, opnd0, false);

    return true;
}

bool latxs_translate_pmaxsw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMAXSW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMAX_H, &dest, &dest, &src);
        return true;
    }

    /* PMAXSW mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMAX_H, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pmaxub(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMAXUB xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMAX_BU, &dest, &dest, &src);
        return true;
    }

    /* PMAXUB mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMAX_BU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pminsw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMINSW xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMIN_H, &dest, &dest, &src);
        return true;
    }

    /* PMINSW mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMIN_H, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pminub(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        /* PMINUB xmm1, xmm2/m128 */
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMIN_BU, &dest, &dest, &src);
        return true;
    }

    /* PMINUB mm1, mm2/m64 */
    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src  = XMM_LOADFP1(opnd1);
    latxs_append_ir2_opnd3(LISA_VMIN_BU, &dest, &dest, &src);

    return true;
}

bool latxs_translate_pmovmskb(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR1_OPND *dest = opnd0;
    IR1_OPND *src  = opnd1;

    if (ir1_opnd_is_xmm(src)) {
        IR2_OPND temp = latxs_ra_alloc_ftemp();
        IR2_OPND xmm_src  = latxs_ra_alloc_xmm(XMM_REG(src));
        IR2_OPND gpr_dest = latxs_ra_alloc_gpr(XMM_REG(dest));

        latxs_append_ir2_opnd2(LISA_VMSKLTZ_B,  &temp, &xmm_src);
        latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &gpr_dest, &temp);
        return true;
    }

    IR2_OPND ftemp = latxs_ra_alloc_ftemp();
    IR2_OPND itemp = latxs_ra_alloc_itemp();
    IR2_OPND src_opnd = latxs_ra_alloc_mmx(XMM_REG(src));

    latxs_append_ir2_opnd2(LISA_VMSKLTZ_B, &ftemp, &src_opnd);

    latxs_append_ir2_opnd2(LISA_MOVFR2GR_D, &itemp, &ftemp);
    latxs_append_ir2_opnd2i(LISA_ANDI, &itemp, &itemp, 0xff);
    latxs_store_ir2_to_ir1(&itemp, dest, false);

    latxs_ra_free_temp(&itemp);
    latxs_ra_free_temp(&ftemp);
    return true;
}

bool latxs_translate_pmuludq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src  = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VMULWEV_D_WU, &dest, &dest, &src);
        return true;
    }

    IR2_OPND dest_lo = XMM_LOADFP1(opnd0);
    IR2_OPND src_lo  = XMM_LOADFP1(opnd1);

    latxs_append_ir2_opnd3(LISA_VMULWEV_D_WU, &dest_lo,
                                              &dest_lo, &src_lo);
    return true;
}

bool latxs_translate_pshufd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);
    IR1_OPND *opnd2 = ir1_get_opnd(pir1, 2);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);
    uint64_t imm8 = ir1_opnd_uimm(opnd2);

    latxs_append_ir2_opnd2i(LISA_VSHUF4I_W, &dest, &src, imm8);

    return true;
}

bool latxs_translate_pshufw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND dest = XMM_LOADFP1(opnd0);
    IR2_OPND src = load_freg_from_ir1_1(opnd0 + 1, true, true);

    IR1_OPND *imm8_reg = ir1_get_opnd(pir1, 2);
    uint64_t imm8 = ir1_opnd_uimm(imm8_reg);
    if (ir1_opnd_is_mem(opnd1) ||
        (XMM_REG(opnd0) != XMM_REG(opnd1))) {
        latxs_append_ir2_opnd2i(LISA_VORI_B, &dest, &src, 0);
    }
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_H, &dest, &dest, imm8);

    return true;
}

bool latxs_translate_pshuflw(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND temp = latxs_ra_alloc_ftemp();

    uint64_t imm8 = ir1_opnd_uimm(ir1_get_opnd(pir1, 2));
    if (ir1_opnd_is_mem(opnd1) ||
        (XMM_REG(opnd0) != XMM_REG(opnd1))) {
        latxs_append_ir2_opnd2i(LISA_VORI_B, &dest, &src, 0);
    }

    latxs_append_ir2_opnd2i(LISA_VORI_B, &temp, &src, 0);
    latxs_append_ir2_opnd2i(LISA_VBSRL_V, &temp, &temp, 8);
    latxs_append_ir2_opnd2i(LISA_VSHUF4I_H, &dest, &dest, imm8);
    latxs_append_ir2_opnd2i(LISA_XVINSVE0_D, &dest, &temp, 1);
    return true;
}

bool latxs_translate_psrldq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    lsassert(ir1_opnd_is_xmm(opnd0));
    IR2_OPND dest = XMM_LOAD128(opnd0);
    uint8_t imm8 = ir1_opnd_uimm(opnd1);
    if (imm8 > 15) {
        latxs_append_ir2_opnd3(LISA_VXOR_V, &dest, &dest, &dest);
    } else if (imm8 == 0) {
        return true;
    } else {
        latxs_append_ir2_opnd2i(LISA_VBSRL_V, &dest, &dest, imm8);
    }
    return true;
}

bool latxs_translate_psubq(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    if (ir1_opnd_is_xmm(opnd0)) {
        IR2_OPND dest = XMM_LOAD128(opnd0);
        IR2_OPND src = XMM_LOAD128(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_D, &dest, &dest, &src);
    } else { /* mmx */
        IR2_OPND dest = XMM_LOADFP1(opnd0);
        IR2_OPND src = XMM_LOADFP1(opnd1);
        latxs_append_ir2_opnd3(LISA_VSUB_D, &dest, &dest, &src);
    }
    return true;
}

bool latxs_translate_ucomisd(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND allone = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_NOR, &allone, zero, zero);

    lsassert(ir1_opnd_num(pir1) == 2);
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND label_nun  = latxs_ir2_opnd_new_label();
    IR2_OPND label_neq  = latxs_ir2_opnd_new_label();
    IR2_OPND label_gt   = latxs_ir2_opnd_new_label();
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();

    /*  case 1: are they unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &dest, &src,
                                 FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_nun);
    /* at least one of the operands is NaN */
    /* set zf,pf,cf = 111 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0xb);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 2: not unordered. are they equal? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_nun);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &dest, &src,
                                 FCMP_COND_CEQ);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_neq);
    /* two operands are equal */
    /* set zf,pf,cf = 100 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x8);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x3);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 3: not unordered, not equal. less than? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_neq);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_D, fcc0, &dest, &src,
                                 FCMP_COND_CLT);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_gt);
    /* less than */
    /* set zf.pf.cf = 001 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0xa);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x1);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    /* set zf,pf,cf = 000 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_gt);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0xb);

    /* exit */
    /* set of,sf,af = 000 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x34);

    return true;
}

bool latxs_translate_ucomiss(IR1_INST *pir1)
{
    XMM_EXCP(pir1);

    IR1_OPND *opnd0 = ir1_get_opnd(pir1, 0);
    IR1_OPND *opnd1 = ir1_get_opnd(pir1, 1);

    IR2_OPND *fcc0 = &latxs_fcc0_ir2_opnd;
    IR2_OPND *zero = &latxs_zero_ir2_opnd;

    IR2_OPND allone = latxs_ra_alloc_itemp();
    latxs_append_ir2_opnd3(LISA_NOR, &allone, zero, zero);

    lsassert(ir1_opnd_num(pir1) == 2);
    IR2_OPND dest = XMM_LOAD128(opnd0);
    IR2_OPND src  = XMM_LOAD128(opnd1);

    IR2_OPND label_nun  = latxs_ir2_opnd_new_label();
    IR2_OPND label_neq  = latxs_ir2_opnd_new_label();
    IR2_OPND label_gt   = latxs_ir2_opnd_new_label();
    IR2_OPND label_exit = latxs_ir2_opnd_new_label();

    /* case 1: are they unordered? */
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, fcc0, &dest, &src,
                                 FCMP_COND_CUN);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_nun);
    /* at least one of the operands is NaN */
    /* set zf,pf,cf = 111 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0xb);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 2: not unordered. are they equal? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_nun);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, fcc0, &dest, &src,
                                 FCMP_COND_CEQ);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_neq);
    /* two operands are equal */
    /* set zf,pf,cf = 100 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x8);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x3);
    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* case 3: not unordered, not equal. less than? */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_neq);
    latxs_append_ir2_opnd3i(LISA_FCMP_COND_S, fcc0, &dest, &src,
                                 FCMP_COND_CLT);
    latxs_append_ir2_opnd2(LISA_BCEQZ, fcc0, &label_gt);
    /* less than */
    /* set zf.pf.cf = 001 */
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0xa);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, &allone, 0x1);

    latxs_append_ir2_opnd1(LISA_B, &label_exit);

    /* not unordered, not equal, not less than, so it's greater than */
    /* set zf,pf,cf = 000 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_gt);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0xb);

    /* exit*/
    /* set of,sf,af = 000 */
    latxs_append_ir2_opnd1(LISA_LABEL, &label_exit);
    latxs_append_ir2_opnd1i(LISA_X86MTFLAG, zero, 0x34);

    return true;
}
