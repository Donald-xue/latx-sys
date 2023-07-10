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
#include "latx-helper.h"
#include "latx-cross-page.h"
#include "latx-counter-sys.h"
#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"



#ifdef TARGET_X86_64
#define LISA_LOAD_PC    LISA_LD_D
#define LISA_STORE_PC   LISA_ST_D
#else
#define LISA_LOAD_PC    LISA_LD_WU
#define LISA_STORE_PC   LISA_ST_W
#endif

int gen_latxs_jmp_glue_cpc(void *code_ptr, int n)
{
    int code_nr = 0;

    latxs_tr_init(NULL);

    TRANSLATION_DATA *td = lsenv->tr_data;
    int rid = td->region_id;
#ifndef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    int use_cc_pro = latxs_cc_pro();
#ifdef LATX_USE_MULTI_REGION
    if (!latx_rid_is_cpl3(rid)) {
        use_cc_pro = 0;
    }
#endif

    int start = (td->ir2_asm_nr << 2);
    int offset = 0;
    int64_t ins_offset = 0;

    int off_tb_tc_ptr = 0;
    off_tb_tc_ptr += offsetof(TranslationBlock, tc);
    off_tb_tc_ptr += offsetof(struct tb_tc, ptr);
    if (use_cc_pro && latxs_cc_pro_checktb()) {
        off_tb_tc_ptr = offsetof(TranslationBlock, cc_ok_ptr);
    }

    IR2_OPND *ret0 = &latxs_ret0_ir2_opnd;    /* $4   a0/v0 */
    IR2_OPND *zero = &latxs_zero_ir2_opnd;    /* $0   zero  */

    IR2_OPND tmp  = latxs_ra_alloc_itemp();
    IR2_OPND tmp0 = latxs_ra_alloc_itemp();
    IR2_OPND tmp1 = latxs_ra_alloc_itemp();

    IR2_OPND tb  = latxs_ra_alloc_dbt_arg1();

    latxs_append_ir2_opnd2i(LISA_LD_D, &tmp0, &tb,
            offsetof(TranslationBlock, next_tb) +
            n * sizeof(void *));

    IR2_OPND njc_miss = latxs_ir2_opnd_new_label();

    latxs_load_addr_to_ir2(&tmp, GET_SC_TABLE(rid, intb_njc));
    latxs_append_ir2_opnd1_(lisa_call, &tmp);
    latxs_append_ir2_opnd3(LISA_BEQ, ret0, zero, &njc_miss);
    latxs_append_ir2_opnd3(LISA_BNE, ret0, &tmp0, &njc_miss);

    latxs_append_ir2_opnd2i(LISA_LD_D, &tmp1, ret0,
            off_tb_tc_ptr);
    latxs_append_ir2_opnd2i(LISA_JIRL, zero, &tmp1, 0);

    latxs_append_ir2_opnd1(LISA_LABEL, &njc_miss);

    offset = (td->ir2_asm_nr << 2) - start;
    ins_offset =
        (GET_SC_TABLE(rid, cs_native_to_bt_ret_0) - (ADDR)code_ptr - offset) >>
        2;
    latxs_append_ir2_jmp_far(ins_offset, 0);



    code_nr = latxs_tr_ir2_assemble(code_ptr);
    latxs_tr_fini();
    return code_nr;
}
