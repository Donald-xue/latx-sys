#ifndef _LATX_FASTCS_SYS_H_
#define _LATX_FASTCS_SYS_H_

int latxs_fastcs_enabled(void);
int latxs_fastcs_enable_tbctx(void);

int latxs_fastcs_is_no_link(void);
int latxs_fastcs_is_jmp_glue(void);
int latxs_fastcs_is_ld_excp(void);

#endif
