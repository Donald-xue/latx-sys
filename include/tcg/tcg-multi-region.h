#ifndef _TCG_MULTI_REGION_H_
#define _TCG_MULTI_REGION_H_

#include "tcg/tcg-ng.h"

#ifdef TCG_USE_MULTI_REGION

void tcg_multi_region_reset(TCGContext *s, int rid);
void tcg_multi_region_switch(TCGContext *s, int rid);
void tcg_multi_region_save(TCGContext *s, int rid);

#endif

#endif
