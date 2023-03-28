#ifndef _LATX_INTB_SYS_H_
#define _LATX_INTB_SYS_H_

#define LATXS_INTB_LINK_ENABLE
#define LATXS_INTB_LINK_OPTION_CHECK





#ifdef LATXS_INTB_LINK_ENABLE

extern ADDR latxs_sc_intb_lookup;
int gen_latxs_intb_lookup(void *code_ptr);

#ifdef LATXS_INTB_LINK_OPTION_CHECK
#define intb_link_enable()  __intb_link_enable()
int __intb_link_enable(void);
#else
#define intb_link_enable()  (1)
#endif

#else /* no LATXS_INTB_LINK_ENABLE */

#define intb_link_enable()  (0)

#endif /* LATXS_INTB_LINK_ENABLE */



#endif
