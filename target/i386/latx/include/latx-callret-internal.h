#ifndef _LATX_CALLRET_H_
#define _LATX_CALLRET_H_

#include "latx-callret-func.h"

void latxs_jr_ra_gen_call(IR1_INST *pir1,
        IR2_OPND *nextpc, IR2_OPND *tmp);
void latxs_jr_ra_gen_ret(IR1_INST *pir1,
        IR2_OPND *retpc, IR2_OPND *tmp);

#endif
