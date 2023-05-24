#ifndef _LATXS_CC_PRO_EXCP_H_
#define _LATXS_CC_PRO_EXCP_H_

int latxs_cc_pro_excp_check_fp(IR1_INST *pir1);
int latxs_cc_pro_excp_check_sse(IR1_INST *pir1);
int latxs_cc_pro_excp_check_ldst_mxcsr(IR1_INST *pir1);
int latxs_cc_pro_excp_check_fxsave(IR1_INST *pir1);
int latxs_cc_pro_excp_check_wait(IR1_INST *pir1);

#endif
