#ifndef _LATX_TB_TRACE_H_
#define _LATX_TB_TRACE_H_

/*#define LATXS_TB_TRACE_ENABLE*/

#ifdef LATXS_TB_TRACE_ENABLE

#define latxs_tb_trace(env, tb) do {    \
      __latxs_tb_trace(env, tb);        \
} while (0)

void __latxs_tb_trace(CPUX86State *env, TranslationBlock *tb);

#else /* disable simple trace */

#define latxs_tb_trace(env, tb)

#endif /* LATXS_TB_TRACE_ENABLE */

#endif
