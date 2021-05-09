#ifndef _LSENV_H_
#define _LSENV_H_
void latx_lsenv_init(CPUArchState *env);
void latx_set_parent_env(CPUArchState *env);
void latx_set_tls_ibtc_table(CPUArchState *env);
void latx_copy_lsenv(CPUArchState *env);
#endif
