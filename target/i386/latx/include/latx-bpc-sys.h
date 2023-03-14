#ifndef _LATX_BPC_SYS_H_
#define _LATX_BPC_SYS_H_

/*#define LATX_BPC_ENABLE*/

extern ADDR latxs_sc_bpc;

int gen_latxs_sc_bpc(void *code_ptr);
void latxs_break_point(CPUX86State *env, TranslationBlock *tb);

#endif
