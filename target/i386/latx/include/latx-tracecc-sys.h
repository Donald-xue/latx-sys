#ifndef _LATXS_CODE_CACHE_H_
#define _LATXS_CODE_CACHE_H_

//#define LATXS_TRACECC_ENABLE

#ifdef LATXS_TRACECC_ENABLE

int tracecc_has_tb_tr(void);
int tracecc_has_tb_exec(void);
int tracecc_has_tb_inv(void);
int tracecc_has_tb_flush_full(void);
int tracecc_has_tb_flush_full_print(void);
int tracecc_has_tb_link(void);
int tracecc_has_tb_flush_fifo(void);
int tracecc_has_tb_flush_fifo_print(void);
int tracecc_has_region_assign(void);

void __latxs_tracecc_gen_tb_insert(TranslationBlock *tb, uint64_t p1, uint64_t p2, int exist);
void __latxs_tracecc_gen_tb_start(void);
void __latxs_tracecc_before_exec_tb(CPUX86State *env, TranslationBlock *tb);
void __latxs_tracecc_target_to_host(CPUX86State *env, TranslationBlock *tb);
void __latxs_tracecc_do_tb_flush_full(void);
void __latxs_tracecc_do_tb_flush_fifo(int rid, int tid);
void __latxs_tracecc_tb_inv(TranslationBlock *tb);
void __latxs_tracecc_tb_link(TranslationBlock *tb, int n, TranslationBlock *ntb);
void __latxs_tracecc_region_assign(int rid, int cid, void *st, void *ed);

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

#define latxs_tracecc_do_tb_flush_full() do {    \
      __latxs_tracecc_do_tb_flush_full();        \
} while (0)

#define latxs_tracecc_do_tb_flush_fifo(rid, tid) do {    \
      __latxs_tracecc_do_tb_flush_fifo(rid, tid);        \
} while (0)

#define latxs_tracecc_tb_inv(tb) do {   \
      __latxs_tracecc_tb_inv(tb);       \
} while (0)

#define latxs_tracecc_tb_link(tb, n, ntb) do {  \
      __latxs_tracecc_tb_link(tb, n, ntb);      \
} while (0)

#define latxs_tracecc_region_assign(rid, cid, st, ed) do {  \
      __latxs_tracecc_region_assign(rid, cid, st, ed);      \
} while (0)

#else /* disable trace code cache */

#define latxs_tracecc_gen_tb_insert(tb, p1, p2, exist)
#define latxs_tracecc_gen_tb_start()
#define latxs_tracecc_before_exec_tb(env, tb)
#define latxs_tracecc_target_to_host(env, tb)
#define latxs_tracecc_do_tb_flush_full()
#define latxs_tracecc_do_tb_flush_fifo(rid, tid)
#define latxs_tracecc_tb_inv(tb)
#define latxs_tracecc_tb_link(tb, n, ntb)
#define latxs_tracecc_region_assign(rid, cid, st, ed)

#endif /* LATXS_TRACECC_ENABLE */

#endif
