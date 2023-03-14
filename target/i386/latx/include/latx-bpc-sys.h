#ifndef _LATX_BPC_SYS_H_
#define _LATX_BPC_SYS_H_

/*#define LATX_BPC_ENABLE*/

#ifdef LATX_BPC_ENABLE

extern ADDR latxs_sc_bpc;

int gen_latxs_sc_bpc(void *code_ptr);

#define latxs_break_point(env, tb) do { \
      __latxs_break_point(env, tb);     \
} while (0)

void __latxs_break_point(CPUX86State *env, TranslationBlock *tb);

#else /* disable break point codes */

#define latxs_break_point(env, tb)

#endif /* LATX_BPC_ENABLE */

#endif
