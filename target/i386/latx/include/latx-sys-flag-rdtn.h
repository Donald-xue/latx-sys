#ifndef _LATX_SYS_FLAG_RDTN_H_
#define _LATX_SYS_FLAG_RDTN_H_

#include "flag-reduction.h"

#define LATXS_FLAGRDTN_ENABLE
#define LATXS_FLAGRDTN_OPTION_CHECK
#define LATXS_FLAGRDTN_CPL_FILTER_



#ifdef LATXS_FLAGRDTN_ENABLE
#define latxs_flag_reduction(tb) do {   \
      __latxs_flag_reduction(tb);       \
} while (0)
void __latxs_flag_reduction(TranslationBlock *tb);
#else /* no LATXS_FLAGRDTN_ENABLE */
#define latxs_flag_reduction(tb) tb_flag(tb)
#endif /* LATXS_FLAGRDTN_ENABLE */

#ifdef LATXS_FLAGRDTN_OPTION_CHECK
#define latxs_flag_check(opt, tb) do {  \
    if (!(opt)) {                       \
        tb_flag(tb);                    \
        return;                         \
    }                                   \
} while (0)
#else /* no LATXS_FLAGRDTN_OPTION_CHECK */
#define latxs_flag_check(opt)
#endif /* LATXS_FLAGRDTN_OPTION_CHECK */

#ifdef LATXS_FLAGRDTN_CPL_FILTER_
#define latxs_flag_cpl_filter(tb) do {  \
    if ((tb->flags & 0x3) != 3)  {      \
        tb_flag(tb);                    \
        return;                         \
    }                                   \
} while (0)
#else /* no LATXS_FLAGRDTN_CPL_FILTER_ */
#define latxs_flag_cpl_filter()
#endif /* LATXS_FLAGRDTN_CPL_FILTER_ */

#endif
