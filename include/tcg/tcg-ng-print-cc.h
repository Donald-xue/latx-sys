#ifndef _TCG_NG_PRINT_CC_H_
#define _TCG_NG_PRINT_CC_H_

//#define NG_TCG_DEBUG_CC

#ifdef NG_TCG_DEBUG_CC

#define cc_info(fmt, ...) do {         \
    fprintf(stderr, "%-30s " fmt,   \
            __func__,               \
            __VA_ARGS__);           \
} while (0)

#define cc_info_str(str) do {          \
    fprintf(stderr, "%-30s %s",     \
            __func__, str);         \
} while (0)

#define cc_endline() do {     \
    fprintf(stderr, "\n");      \
} while (0)

#else

#define cc_info(fmt, ...)
#define cc_info_str(str)
#define cc_endline()

#endif

#endif
