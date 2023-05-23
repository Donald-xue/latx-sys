#ifndef _LATXS_CC_PRO_H_
#define _LATXS_CC_PRO_H_

#define CC_FLAG_MASK    0xe00

int latxs_cc_pro(void);

int latxs_cc_pro_nolink(void);
int latxs_cc_pro_checktb(void);
int latxs_cc_pro_checkjmp(void);

void latxs_cc_pro_gen_tb_start(void);

void *latxs_cc_pro_get_next_ptr(void *tb, void *nexttb);

int latxs_cc_pro_tb_flags_cmp(
        const TranslationBlock *tb,
        uint32_t cpu_flags);

#endif
