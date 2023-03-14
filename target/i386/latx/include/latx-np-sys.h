#ifndef _LATX_NP_SYS_H_
#define _LATX_NP_SYS_H_

#ifdef CONFIG_SOFTMMU

/*#define LATXS_NP_ENABLE*/

extern ADDR latxs_native_printer;
int gen_latxs_native_printer(void *code_ptr);
void latxs_np_env_init(CPUX86State *env);
/* optimization/sys-fastcs.c */
void latxs_native_printer_cs(lsenv_np_data_t *npd,
        int, int, int, int, int, int);
/* translator/sys-softmmu.c */
void latxs_native_printer_tlbcmp(lsenv_np_data_t *npd,
        int, int, int, int, int, int, uint32_t);
/* translator/sys-native-printer.c */
void latxs_native_printer_tb(lsenv_np_data_t *npd,
        int, int, int, int, int, int);
void latxs_np_tb_print(CPUX86State *env);

int latxs_np_enabled(void);
int latxs_np_cs_enabled(void);
int latxs_np_tlbcmp_enabled(void);
int latxs_np_tb_enabled(void);

#define LATXS_NP_TLBCMP     1
#define LATXS_NP_CS         2
#define LATXS_NP_TB         4

#define LATXS_NP_CS_PRO         1
#define LATXS_NP_CS_EPI         2
#define LATXS_NP_CS_SPRO        3
#define LATXS_NP_CS_SEPI        4
#define LATXS_NP_CS_HPRO        5
#define LATXS_NP_CS_HEPI        6

void latxs_np_tr_cs_prologue(void);
void latxs_np_tr_cs_epilogue(void);
void latxs_np_tr_scs_prologue(void);
void latxs_np_tr_scs_epilogue(void);
void latxs_np_tr_hcs_prologue(void);
void latxs_np_tr_hcs_epilogue(void);

void latxs_np_tr_tb_start(void);
void latxs_np_tr_tb_end(void);

#endif /* CONFIG_SOFTMMU */

#endif
