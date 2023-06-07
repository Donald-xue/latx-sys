#ifndef _TCG_BG_JC_H_
#define _TCG_BG_JC_H_

void tcg_bg_jc_init_static(void);
void tcg_bg_init_jc(void *cpu);

#define TCG_BG_JC_CONFIG_SIZE
void tcg_bg_jc_init_jmp_cache_bits(int n);
void tcg_bg_jc_init_jmp_cache_page_bits(int n);

#ifdef TCG_BG_JC_CONFIG_SIZE
extern int tcg_bg_jc_bits;
extern int tcg_bg_jc_page_bits;
#define TCG_BG_JC_BITS_MAX          16
#define TCG_BG_JC_BITS_MIN          12
#define TCG_BG_JC_PAGE_BITS_MAX     9
#define TCG_BG_JC_PAGE_BITS_MIN     5
#define TCG_BG_JC_SIZE_MAX  (1 << TCG_BG_JC_BITS_MAX)
#define TCG_BG_JC_FLAG_MAX  (1 << (TCG_BG_JC_BITS_MAX-TCG_BG_JC_PAGE_BITS_MIN))
extern uint64_t tcg_bg_jc_page_mask;
extern uint64_t tcg_bg_jc_addr_mask;
#endif

#endif
