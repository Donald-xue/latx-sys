#ifndef _TCG_BG_TLB_H_
#define _TCG_BG_TLB_H_

#define BG_TLB_ENABLE

#include "tcg/tcg-bg-tlb-init.h"

void tcg_bg_tlb_flush(CPUTLBDesc *desc, CPUTLBDescFast *fast);
void tcg_bg_tlb_init(CPUTLBDesc *desc, CPUTLBDescFast *fast);

#endif
