#include "common.h"
#include "lsenv.h"
#include "reg-alloc.h"
#include "translate.h"
#include "latx-options.h"

void latxs_tr_fpu_init(TRANSLATION_DATA *td, TranslationBlock *tb)
{
    if (option_lsfpu) {
        return;
    }

    if (tb != NULL) {
        latxs_td_fpu_set_top(tb->_top_in);
        lsassert(t->curr_top != -1);
    } else {
        latxs_td_fpu_set_top(0);
    }
}

void latxs_td_fpu_set_top(int ctop)
{
    if (option_lsfpu) {
        lsassert(0);
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td);

    td->curr_top = ctop & 0x7;
}

int latxs_td_fpu_get_top(void)
{
    if (option_lsfpu) {
        lsassert(0);
    }

    TRANSLATION_DATA *td = lsenv->tr_data;
    lsassert(td);

    return td->curr_top & 0x7;
}
