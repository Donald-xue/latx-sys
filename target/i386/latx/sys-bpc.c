#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "translate.h"
#include "latx-options.h"
#include "latx-bpc-sys.h"

#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#ifdef LATX_BPC_ENABLE

int gen_latxs_sc_bpc(void *code_ptr)
{
    int code_nr = 0;
    latxs_tr_init(NULL);

    latxs_append_ir2_opnd0_(lisa_nop);
    latxs_append_ir2_opnd0_(lisa_nop);
    latxs_append_ir2_opnd0_(lisa_return);

    code_nr = latxs_tr_ir2_assemble(code_ptr);

    latxs_tr_fini();

    return code_nr;
}

static unsigned long long latxs_bpc_tb_cnt;

void __latxs_break_point(CPUX86State *env, TranslationBlock *tb)
{
    if (!option_break_point) {
        return;
    }

    int rid = lsenv->tr_data->region_id;
#ifdef LATX_USE_MULTI_REGION
    lsassert(rid == 0);
#endif

    uint64_t latxs_sc_bpc = GET_SC_TABLE(rid, bpc);

    if (option_break_point &&
            latxs_sc_bpc &&
            tb->pc == option_break_point_addrx) {
        latxs_bpc_tb_cnt += 1;
        fprintf(stderr, "[debug] BP at TB 0x"TARGET_FMT_lx" cnt = %lld\n",
                tb->pc, latxs_bpc_tb_cnt);

        if (latxs_bpc_tb_cnt >= option_break_point_count) {
            ((void(*)(void))latxs_sc_bpc)();
        }
    }
}

#endif
