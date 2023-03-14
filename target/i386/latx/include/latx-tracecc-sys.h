#ifndef _LATXS_CODE_CACHE_H_
#define _LATXS_CODE_CACHE_H_

/*#define LATXS_TRACECC_ENABLE*/

#ifdef LATXS_TRACECC_ENABLE

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

void __latxs_tracecc_gen_tb_insert(TranslationBlock *tb, uint64_t p1, uint64_t p2, int exist);
void __latxs_tracecc_gen_tb_start(void);
void __latxs_tracecc_before_exec_tb(CPUX86State *env, TranslationBlock *tb);
void __latxs_tracecc_target_to_host(CPUX86State *env, TranslationBlock *tb);
void __latxs_tracecc_do_tb_flush(void);
void __latxs_tracecc_tb_inv(TranslationBlock *tb);
void __latxs_tracecc_tb_link(TranslationBlock *tb, int n, TranslationBlock *ntb);

#define latxs_tracecc_gen_tb_insert(tb, p1, p2, exist) do {     \
      __latxs_tracecc_gen_tb_insert(tb, p1, p2, exist);         \
} while (0)

#define latxs_tracecc_gen_tb_start() do {   \
      __latxs_tracecc_gen_tb_start();       \
} while (0)

#define latxs_tracecc_before_exec_tb(env, tb) do {  \
      __latxs_tracecc_before_exec_tb(env, tb);      \
} while (0)

#define latxs_tracecc_target_to_host(env, tb) do {  \
      __latxs_tracecc_target_to_host(env, tb);      \
} while (0)

#define latxs_tracecc_do_tb_flush() do {    \
      __latxs_tracecc_do_tb_flush();        \
} while (0)

#define latxs_tracecc_tb_inv(tb) do {   \
      __latxs_tracecc_tb_inv(tb);       \
} while (0)

#define latxs_tracecc_tb_link(tb, n, ntb) do {  \
      __latxs_tracecc_tb_link(tb, n, ntb);      \
} while (0)

#else /* disable trace code cache */

#define latxs_tracecc_gen_tb_insert(tb, p1, p2, exist)
#define latxs_tracecc_gen_tb_start()
#define latxs_tracecc_before_exec_tb(env, tb)
#define latxs_tracecc_target_to_host(env, tb)
#define latxs_tracecc_do_tb_flush()
#define latxs_tracecc_tb_inv(tb)
#define latxs_tracecc_tb_link(tb, n, ntb)

#endif /* LATXS_TRACECC_ENABLE */

#endif
