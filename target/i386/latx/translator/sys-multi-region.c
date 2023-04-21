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
#include "latx-sigint-sys.h"
#include "latx-intb-sys.h"
#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#ifdef LATX_USE_MULTI_REGION

typedef struct latx_multi_region_info {

#ifdef LATX_BPC_ENABLE
    uint64_t latxs_sc_bpc;
#endif

    uint64_t latxs_sc_intb_njc;

    uint64_t latxs_sc_fcs_jmp_glue_fpu_0;
    uint64_t latxs_sc_fcs_jmp_glue_fpu_1;
    uint64_t latxs_sc_fcs_jmp_glue_xmm_0;
    uint64_t latxs_sc_fcs_jmp_glue_xmm_1;

#ifdef LATXS_NP_ENABLE
    uint64_t latxs_native_printer;
#endif

    uint64_t latxs_sc_scs_prologue;
    uint64_t latxs_sc_scs_epilogue;

    uint64_t latxs_sc_fcs_F_0;
    uint64_t latxs_sc_fcs_F_1;
    uint64_t latxs_sc_fcs_S_0;
    uint64_t latxs_sc_fcs_S_1;
    uint64_t latxs_sc_fcs_FS_0;
    uint64_t latxs_sc_fcs_FS_1;
    uint64_t latxs_sc_fcs_check_load_F;
    uint64_t latxs_sc_fcs_check_load_S;
    uint64_t latxs_sc_fcs_check_load_FS;
    uint64_t latxs_sc_fcs_load_F;
    uint64_t latxs_sc_fcs_load_S;
    uint64_t latxs_sc_fcs_load_FS;

#ifdef LATXS_INTB_LINK_ENABLE
    uint64_t latxs_sc_intb_lookup;
#endif

    uint64_t context_switch_bt_to_native;
    uint64_t context_switch_native_to_bt_ret_0;
    uint64_t context_switch_native_to_bt;
    uint64_t ss_match_fail_native;

    uint64_t native_rotate_fpu_by; /* native_rotate_fpu_by(step, return_address) */
    uint64_t native_jmp_glue_0;
    uint64_t native_jmp_glue_1;
    uint64_t native_jmp_glue_2;

    uint64_t fpu_enable_top;
    uint64_t fpu_disable_top;
    uint64_t fpu_get_top;
    uint64_t fpu_set_top;
    uint64_t fpu_inc_top;
    uint64_t fpu_dec_top;
} latx_multi_region_info;

latx_multi_region_info latx_mregion_info[LATX_MULTI_REGION_N];

#define LATX_MR_LOAD(id, name) do {     \
    name = latx_mregion_info[id].name;  \
} while (0)
#define LATX_MR_SAVE(id, name) do {     \
    latx_mregion_info[id].name = name;  \
} while (0)

void __latx_multi_region_switch(int rid)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    if (td->region_id == rid) return;

    /*printf("%-20s [LATX] switch to region[%d]\n",*/
            /*__func__, rid);*/
    td->region_id = rid;

    /* set global variables of static codes */
#ifdef LATX_BPC_ENABLE
    LATX_MR_LOAD(rid, latxs_sc_bpc);
#endif
    LATX_MR_LOAD(rid, latxs_sc_intb_njc);
    LATX_MR_LOAD(rid, latxs_sc_fcs_jmp_glue_fpu_0);
    LATX_MR_LOAD(rid, latxs_sc_fcs_jmp_glue_fpu_1);
    LATX_MR_LOAD(rid, latxs_sc_fcs_jmp_glue_xmm_0);
    LATX_MR_LOAD(rid, latxs_sc_fcs_jmp_glue_xmm_1);
#ifdef LATXS_NP_ENABLE
    LATX_MR_LOAD(rid, latxs_native_printer);
#endif
    LATX_MR_LOAD(rid, latxs_sc_scs_prologue);
    LATX_MR_LOAD(rid, latxs_sc_scs_epilogue);
    LATX_MR_LOAD(rid, latxs_sc_fcs_F_0);
    LATX_MR_LOAD(rid, latxs_sc_fcs_F_1);
    LATX_MR_LOAD(rid, latxs_sc_fcs_S_0);
    LATX_MR_LOAD(rid, latxs_sc_fcs_S_1);
    LATX_MR_LOAD(rid, latxs_sc_fcs_FS_0);
    LATX_MR_LOAD(rid, latxs_sc_fcs_FS_1);
    LATX_MR_LOAD(rid, latxs_sc_fcs_check_load_F);
    LATX_MR_LOAD(rid, latxs_sc_fcs_check_load_S);
    LATX_MR_LOAD(rid, latxs_sc_fcs_check_load_FS);
    LATX_MR_LOAD(rid, latxs_sc_fcs_load_F);
    LATX_MR_LOAD(rid, latxs_sc_fcs_load_S);
    LATX_MR_LOAD(rid, latxs_sc_fcs_load_FS);
#ifdef LATXS_INTB_LINK_ENABLE
    LATX_MR_LOAD(rid, latxs_sc_intb_lookup);
#endif
    LATX_MR_LOAD(rid, context_switch_bt_to_native);
    LATX_MR_LOAD(rid, context_switch_native_to_bt_ret_0);
    LATX_MR_LOAD(rid, context_switch_native_to_bt);
    LATX_MR_LOAD(rid, ss_match_fail_native);
    LATX_MR_LOAD(rid, native_rotate_fpu_by);
    LATX_MR_LOAD(rid, native_jmp_glue_0);
    LATX_MR_LOAD(rid, native_jmp_glue_1);
    LATX_MR_LOAD(rid, native_jmp_glue_2);
    LATX_MR_LOAD(rid, fpu_enable_top);
    LATX_MR_LOAD(rid, fpu_disable_top);
    LATX_MR_LOAD(rid, fpu_get_top);
    LATX_MR_LOAD(rid, fpu_set_top);
    LATX_MR_LOAD(rid, fpu_inc_top);
    LATX_MR_LOAD(rid, fpu_dec_top);
}

void __latx_multi_region_save(int rid)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td->region_id == rid);

    /* save info mregion_info */
#ifdef LATX_BPC_ENABLE
    LATX_MR_SAVE(rid, latxs_sc_bpc);
#endif
    LATX_MR_SAVE(rid, latxs_sc_intb_njc);
    LATX_MR_SAVE(rid, latxs_sc_fcs_jmp_glue_fpu_0);
    LATX_MR_SAVE(rid, latxs_sc_fcs_jmp_glue_fpu_1);
    LATX_MR_SAVE(rid, latxs_sc_fcs_jmp_glue_xmm_0);
    LATX_MR_SAVE(rid, latxs_sc_fcs_jmp_glue_xmm_1);
#ifdef LATXS_NP_ENABLE
    LATX_MR_SAVE(rid, latxs_native_printer);
#endif
    LATX_MR_SAVE(rid, latxs_sc_scs_prologue);
    LATX_MR_SAVE(rid, latxs_sc_scs_epilogue);
    LATX_MR_SAVE(rid, latxs_sc_fcs_F_0);
    LATX_MR_SAVE(rid, latxs_sc_fcs_F_1);
    LATX_MR_SAVE(rid, latxs_sc_fcs_S_0);
    LATX_MR_SAVE(rid, latxs_sc_fcs_S_1);
    LATX_MR_SAVE(rid, latxs_sc_fcs_FS_0);
    LATX_MR_SAVE(rid, latxs_sc_fcs_FS_1);
    LATX_MR_SAVE(rid, latxs_sc_fcs_check_load_F);
    LATX_MR_SAVE(rid, latxs_sc_fcs_check_load_S);
    LATX_MR_SAVE(rid, latxs_sc_fcs_check_load_FS);
    LATX_MR_SAVE(rid, latxs_sc_fcs_load_F);
    LATX_MR_SAVE(rid, latxs_sc_fcs_load_S);
    LATX_MR_SAVE(rid, latxs_sc_fcs_load_FS);
#ifdef LATXS_INTB_LINK_ENABLE
    LATX_MR_SAVE(rid, latxs_sc_intb_lookup);
#endif
    LATX_MR_SAVE(rid, context_switch_bt_to_native);
    LATX_MR_SAVE(rid, context_switch_native_to_bt_ret_0);
    LATX_MR_SAVE(rid, context_switch_native_to_bt);
    LATX_MR_SAVE(rid, ss_match_fail_native);
    LATX_MR_SAVE(rid, native_rotate_fpu_by);
    LATX_MR_SAVE(rid, native_jmp_glue_0);
    LATX_MR_SAVE(rid, native_jmp_glue_1);
    LATX_MR_SAVE(rid, native_jmp_glue_2);
    LATX_MR_SAVE(rid, fpu_enable_top);
    LATX_MR_SAVE(rid, fpu_disable_top);
    LATX_MR_SAVE(rid, fpu_get_top);
    LATX_MR_SAVE(rid, fpu_set_top);
    LATX_MR_SAVE(rid, fpu_inc_top);
    LATX_MR_SAVE(rid, fpu_dec_top);
}

void __latx_multi_region_init(int region_nr)
{
    lsassert(region_nr == LATX_MULTI_REGION_N);
}

int __latx_multi_region_get_id(void *__cpu)
{
    CPUState *cpu = __cpu;
    CPUX86State *env = cpu->env_ptr;
    if ((env->hflags & 0x3) == 3) {
        return 1; /* CPL3 in region[1] */
    } else {
        return 0; /* CPL0 in region[0] */
    }
}

#endif
