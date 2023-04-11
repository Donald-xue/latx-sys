#ifndef _LATX_SYS_INSTPTN_H_
#define _LATX_SYS_INSTPTN_H_

#define LATXS_INSTPTN_ENABLE
#define LATXS_INSTPTN_OPTION_CHECK
#define LATXS_INSTPTN_CPL_FILTER



#ifdef LATXS_INSTPTN_ENABLE

#define INSTPTN_BUF_SIZE    2

#define INSTPTN_OPC_NONE        0x00
#define INSTPTN_OPC_NOP         0x01
#define INSTPTN_OPC_CMP_JCC     0x10
#define INSTPTN_OPC_TEST_JCC    0x20

#define latxs_inst_pattern(tb) do { \
      __latxs_inst_pattern(tb);     \
} while (0)
void __latxs_inst_pattern(TranslationBlock *tb);

bool try_translate_instptn(IR1_INST *pir1);

#ifdef LATXS_INSTPTN_OPTION_CHECK
#define latxs_instptn_check_void() do { \
    if (!option_instptn) return;        \
} while (0)
#define latxs_instptn_check_false() do {    \
    if (!option_instptn) return false;      \
} while (0)
#else
#define latxs_instptn_check_void(option)
#define latxs_instptn_check_false(option)
#endif

#ifdef LATXS_INSTPTN_CPL_FILTER
#define latxs_instptn_cpl_filter_void(tb) do {  \
    if ((tb->flags & 0x3) != 0x3) return;       \
} while (0)
#else
#define latxs_instptn_cpl_filter_void(tb)
#endif

#else /* no LATXS_INSTPTN_ENABLE */

#define latxs_inst_pattern(tb)
#endif /* LATXS_INSTPTN_ENABLE */

#endif
