#ifndef _LATXS_CC_PRO_H_
#define _LATXS_CC_PRO_H_

#define CC_FLAG_MASK    0xe00

int latxs_cc_pro(void);

int latxs_cc_pro_nolink(void);
int latxs_cc_pro_checktb(void);
int latxs_cc_pro_checkjmp(void);

int latxs_cc_pro_for_tb(void *tb);

#define CCPRO_SET_INST(tb) do {     \
    if (latxs_cc_pro_for_tb(tb)) {  \
        tb->cc_flags |= 0x2;        \
        tb->cc_mask = ~0x0;         \
    }                               \
} while (0)

#define CCPRO_SET_EXCP(tb) do {     \
    if (latxs_cc_pro_for_tb(tb))    \
        tb->cc_flags |= 0x1;        \
} while (0)

void latxs_cc_pro_init_tb(void *tb);

uint32_t latxs_cc_pro_reset_flags_for_hash(uint32_t flags);
uint32_t latxs_cc_pro_get_tb_flags_for_hash(void *tb);

void latxs_cc_pro_gen_tb_start(void);

void *latxs_cc_pro_get_next_ptr(void *tb, void *nexttb);

int latxs_cc_pro_tb_flags_cmp(
        const TranslationBlock *tb,
        uint32_t cpu_flags);

#endif
