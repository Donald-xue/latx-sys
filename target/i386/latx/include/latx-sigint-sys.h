#ifndef _LATX_SIGINT_SYS_H_
#define _LATX_SIGINT_SYS_H_

void latxs_tb_unlink(TranslationBlock *utb);
void latxs_tb_relink(TranslationBlock *utb);
void latxs_sigint_prepare_check_intb_lookup(IR2_OPND s, IR2_OPND e);

#endif
