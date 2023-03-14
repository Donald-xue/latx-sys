#ifndef _LATXS_CODE_CACHE_H_
#define _LATXS_CODE_CACHE_H_

/*#define LATXS_TRACECC_ENABLE*/

#define LATXS_TRACECC_TB_TR           (1 << 0)
#define LATXS_TRACECC_TB_EXEC         (1 << 1)
#define LATXS_TRACECC_TB_INV          (1 << 2)
#define LATXS_TRACECC_TB_FLUSH        (1 << 3)
#define LATXS_TRACECC_TB_FLUSH_PRINT  (1 << 4)
#define LATXS_TRACECC_TB_LINK         (1 << 5)

int tracecc_has_tb_tr(void);
int tracecc_has_tb_exec(void);
int tracecc_has_tb_inv(void);
int tracecc_has_tb_flush(void);
int tracecc_has_tb_flush_print(void);
int tracecc_has_tb_link(void);



void latxs_tracecc_gen_tb_insert(TranslationBlock *tb, uint64_t p1, uint64_t p2, int exist);
void latxs_tracecc_gen_tb_start(void);
void latxs_tracecc_before_exec_tb(CPUX86State *env, TranslationBlock *tb);
void latxs_tracecc_target_to_host(CPUX86State *env, TranslationBlock *tb);
void latxs_tracecc_do_tb_flush(void);
void latxs_tracecc_tb_inv(TranslationBlock *tb);
void latxs_tracecc_tb_link(TranslationBlock *tb, int n, TranslationBlock *ntb);

#endif
