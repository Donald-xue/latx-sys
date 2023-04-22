#ifndef _LATX_MULTI_REGION_H_
#define _LATX_MULTI_REGION_H_

#include "tcg/tcg-ng.h"

#ifdef TCG_USE_MULTI_REGION
#define LATX_USE_MULTI_REGION
#endif /* TCG_USE_MULTI_REGION */



#ifdef LATX_USE_MULTI_REGION

#define LATX_REGION_N 2

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

void __latx_multi_region_switch(int rid);
void __latx_multi_region_save(int rid);
void __latx_multi_region_init(int region_nr);
int __latx_multi_region_get_id(void *cpu);

void __latx_multi_region_prepare_exec(int rid);

#else

#define LATX_REGION_N 1

#define latx_multi_region_switch(rid)
#define latx_multi_region_save(rid)
#define latx_multi_region_init(rid)
#define latx_multi_region_get_id(cpu)   (0)

#define latx_multi_region_prepare_exec(rid)

#endif

#endif
