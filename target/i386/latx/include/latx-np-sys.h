#ifndef _LATX_NP_SYS_H_
#define _LATX_NP_SYS_H_

#ifdef CONFIG_SOFTMMU

/*#define LATXS_NP_ENABLE*/

#ifdef LATXS_NP_ENABLE

#define LATXS_NP_TLBCMP     1
#define LATXS_NP_CS         2
#define LATXS_NP_TB         4

#define LATXS_NP_CS_PRO         1
#define LATXS_NP_CS_EPI         2
#define LATXS_NP_CS_SPRO        3
#define LATXS_NP_CS_SEPI        4
#define LATXS_NP_CS_HPRO        5
#define LATXS_NP_CS_HEPI        6

int latxs_np_enabled(void);
int latxs_np_cs_enabled(void);
int latxs_np_tlbcmp_enabled(void);
int latxs_np_tb_enabled(void);

extern ADDR latxs_native_printer;
int gen_latxs_native_printer(void *code_ptr);

#define latxs_np_env_init(env) do { \
      __latxs_np_env_init(env);     \
} while (0)

/* optimization/sys-fastcs.c */
void latxs_native_printer_cs(lsenv_np_data_t *npd,
        int, int, int, int, int, int);
/* translator/sys-softmmu.c */
void latxs_native_printer_tlbcmp(lsenv_np_data_t *npd,
        int, int, int, int, int, int, uint32_t);
/* translator/sys-native-printer.c */
void latxs_native_printer_tb(lsenv_np_data_t *npd,
        int, int, int, int, int, int);

#define latxs_np_tb_print(env) do { \
      __latxs_np_tb_print(env);     \
} while (0)

#define latxs_np_tr_cs_prologue() do {  \
      __latxs_np_tr_cs_prologue();      \
} while (0)
#define latxs_np_tr_cs_epilogue() do {  \
      __latxs_np_tr_cs_epilogue();      \
} while (0)
#define latxs_np_tr_scs_prologue() do { \
      __latxs_np_tr_scs_prologue();     \
} while (0)
#define latxs_np_tr_scs_epilogue() do { \
      __latxs_np_tr_scs_epilogue();     \
} while (0)
#define latxs_np_tr_hcs_prologue() do { \
      __latxs_np_tr_hcs_prologue();     \
} while (0)
#define latxs_np_tr_hcs_epilogue() do { \
      __latxs_np_tr_hcs_epilogue();     \
} while (0)

#define latxs_np_tr_tb_start() do { \
      __latxs_np_tr_tb_start();     \
} while (0)
#define latxs_np_tr_tb_end() do {   \
      __latxs_np_tr_tb_end();       \
} while (0)

void __latxs_np_env_init(CPUX86State *env);
void __latxs_np_tb_print(CPUX86State *env);

void __latxs_np_tr_cs_prologue(void);
void __latxs_np_tr_cs_epilogue(void);
void __latxs_np_tr_scs_prologue(void);
void __latxs_np_tr_scs_epilogue(void);
void __latxs_np_tr_hcs_prologue(void);
void __latxs_np_tr_hcs_epilogue(void);

void __latxs_np_tr_tb_start(void);
void __latxs_np_tr_tb_end(void);

#else /* disable native printer */

#define latxs_np_env_init(env)
#define latxs_np_tr_cs_prologue()
#define latxs_np_tr_cs_epilogue()
#define latxs_np_tr_scs_prologue()
#define latxs_np_tr_scs_epilogue()
#define latxs_np_tr_hcs_prologue()
#define latxs_np_tr_hcs_epilogue()
#define latxs_np_tr_tb_start()
#define latxs_np_tr_tb_end()
#define latxs_np_tb_print(env)

#endif /* LATXS_NP_ENABLE */

#endif /* CONFIG_SOFTMMU */

#endif
