#ifndef _FLAG_REDUCTION_H_
#define _FLAG_REDUCTION_H_

#include "latx-types.h"
#include "etb.h"

typedef struct {
    uint8 use;
    uint8 def;
    uint8 undef;
} IR1_EFLAG_USEDEF;

#define __INVALID    (1 << 7)
#define __VALID      (0)

#define __CF         ((1 << CF_USEDEF_BIT_INDEX) | __VALID)
#define __PF         ((1 << PF_USEDEF_BIT_INDEX) | __VALID)
#define __AF         ((1 << AF_USEDEF_BIT_INDEX) | __VALID)
#define __ZF         ((1 << ZF_USEDEF_BIT_INDEX) | __VALID)
#define __SF         ((1 << SF_USEDEF_BIT_INDEX) | __VALID)
#define __OF         ((1 << OF_USEDEF_BIT_INDEX) | __VALID)
#define __DF         ((1 << DF_USEDEF_BIT_INDEX) | __VALID)

#define __NONE       (0)
#define __OSAPF      (__OF | __SF | __AF | __PF)
#define __OSZPF      (__OF | __SF | __ZF | __PF)
#define __SZAPF      (__SF | __ZF | __AF | __PF)
#define __SZAPCF     (__SZAPF | __CF)
#define __ALL_EFLAGS (__OF | __SF | __ZF | __AF | __PF | __CF)

void tb_flag_reduction(void *tb);
void tb_flag(struct TranslationBlock *tb);
#ifdef CONFIG_LATX_FLAG_REDUCTION
uint8 pending_use_of_succ(ETB* etb, int max_depth);
#endif

#endif
