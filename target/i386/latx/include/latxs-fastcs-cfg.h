#ifndef _LATXS_FASTCS_CONFIG_H_
#define _LATXS_FASTCS_CONFIG_H_

#define FASTCS_FPU_AND_SIMD
/*
 * This define affects the value of fastcs_ctx
 *
 *     defined: 00       11
 * not defined: 00 01 10 11
 */

//#define FASTCS_INCLUDE_FCSR
#define FCSR_LOAD_RM_ONLY
#define FCSR_SAVE_RM_ONLY

#endif
