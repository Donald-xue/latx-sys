#ifndef _LATX_STATIC_CODES_H_
#define _LATX_STATIC_CODES_H_

#include "latx-bpc-sys.h"
#include "latx-np-sys.h"
#include "latx-intb-sys.h"

typedef struct latx_sc_table_t {

#ifdef LATX_BPC_ENABLE
    uint64_t bpc;
#endif

#ifdef LATXS_INTB_LINK_ENABLE
    uint64_t intb_lookup;
#endif
    uint64_t intb_njc;

#ifdef LATXS_NP_ENABLE
    uint64_t nprint;
#endif

    uint64_t scs_prologue;
    uint64_t scs_epilogue;

    uint64_t fcs_jmp_glue_fpu_0;
    uint64_t fcs_jmp_glue_fpu_1;
    uint64_t fcs_jmp_glue_xmm_0;
    uint64_t fcs_jmp_glue_xmm_1;
    uint64_t fcs_F_0;
    uint64_t fcs_F_1;
    uint64_t fcs_S_0;
    uint64_t fcs_S_1;
    uint64_t fcs_FS_0;
    uint64_t fcs_FS_1;
    uint64_t fcs_check_load_F;
    uint64_t fcs_check_load_S;
    uint64_t fcs_check_load_FS;
    uint64_t fcs_load_F;
    uint64_t fcs_load_S;
    uint64_t fcs_load_FS;

    uint64_t cs_bt_to_native;
    uint64_t cs_native_to_bt_ret_0;
    uint64_t cs_native_to_bt;

    uint64_t jmp_glue_0;
    uint64_t jmp_glue_1;
    uint64_t jmp_glue_2;

    uint64_t jmp_glue_cpc_0;
    uint64_t jmp_glue_cpc_1;

    uint64_t fpu_rotate;
    uint64_t fpu_enable_top;
    uint64_t fpu_disable_top;
    uint64_t fpu_get_top;
    uint64_t fpu_set_top;
    uint64_t fpu_inc_top;
    uint64_t fpu_dec_top;

} latx_sc_table_t;

extern latx_sc_table_t *latx_sc_table;

#define SET_SC_TABLE(rid, name, value) do {     \
    latx_sc_table[rid].name = (value);          \
} while (0)

#define GET_SC_TABLE(rid, name) (latx_sc_table[rid].name)



#endif
