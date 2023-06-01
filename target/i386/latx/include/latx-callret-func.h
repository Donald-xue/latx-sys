#ifndef _LATX_CALLREG_FUNC_H_
#define _LATX_CALLREG_FUNC_H_

int latxs_jr_ra(void);
int latxs_jr_ra_for_tb(TranslationBlock *tb);

void latxs_jr_ra_finish_call(TranslationBlock *tb, TranslationBlock *tb2);
void latxs_jr_ra_reset_call(TranslationBlock *tb);
void latxs_clear_scr(void);

#endif
