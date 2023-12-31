#ifndef _LATX_MULTI_REGION_H_
#define _LATX_MULTI_REGION_H_

#include "tcg/tcg-ng.h"

#ifdef TCG_USE_MULTI_REGION
#define LATX_USE_MULTI_REGION
#endif /* TCG_USE_MULTI_REGION */



#ifdef LATX_USE_MULTI_REGION

#define LATX_REGION_N 2

#define LATX_REGION_ID_DISABLE  0
#define LATX_REGION_ID_CPL0 0
#define LATX_REGION_ID_CPL3 1

#define latx_multi_region_switch(rid) do {  \
      __latx_multi_region_switch(rid);      \
} while (0)
#define latx_multi_region_save(rid) do {    \
      __latx_multi_region_save(rid);        \
} while (0)
#define latx_multi_region_init(rid) do {    \
      __latx_multi_region_init(rid);        \
} while (0)
#define latx_multi_region_get_id(cpu)   \
      __latx_multi_region_get_id(cpu);  \

#define latx_multi_region_prepare_exec(rid) do {    \
      __latx_multi_region_prepare_exec(rid);        \
} while (0)

int latx_multi_region_enable(void);

void __latx_multi_region_switch(int rid);
void __latx_multi_region_save(int rid);
void __latx_multi_region_init(int region_nr);
int __latx_multi_region_get_id(void *cpu);

void __latx_multi_region_prepare_exec(int rid);

static inline
int latx_rid_is_cpl0(int rid) { return rid == LATX_REGION_ID_CPL0; }
static inline
int latx_rid_is_cpl3(int rid) { return rid == LATX_REGION_ID_CPL3; }

#else

#define LATX_REGION_N 1

#define latx_multi_region_switch(rid)
#define latx_multi_region_save(rid)
#define latx_multi_region_init(rid)
#define latx_multi_region_get_id(cpu)   (0)

#define latx_multi_region_prepare_exec(rid)

#endif

#endif
