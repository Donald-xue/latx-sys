#include "common.h"
#include "cpu.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "latx-options.h"
#include "translate.h"
#include <string.h>
#include "latxs-fastcs-cfg.h"
#include "latx-fastcs-sys.h"
#include "latx-perfmap.h"
#include "latx-bpc-sys.h"
#include "latx-np-sys.h"
#include "latx-sigint-sys.h"
#include "latx-intb-sys.h"

#include "latx-multi-region-sys.h"
#include "latx-static-codes.h"

#ifdef LATX_USE_MULTI_REGION

void __latx_multi_region_switch(int rid)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    td->region_id = rid;
}

void __latx_multi_region_save(int rid)
{
    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td->region_id == rid);
}

void __latx_multi_region_init(int region_nr)
{
    lsassert(region_nr == LATX_REGION_N);
}

int __latx_multi_region_get_id(void *__cpu)
{
    CPUState *cpu = __cpu;
    CPUX86State *env = cpu->env_ptr;
    if ((env->hflags & 0x3) == 3) {
        return LATX_REGION_ID_CPL3; /* CPL3 in region[1] */
    } else {
        return LATX_REGION_ID_CPL0; /* CPL0 in region[0] */
    }
}

void __latx_multi_region_prepare_exec(int rid)
{
    latxs_fastcs_set_indir_table(rid);
}

#endif
