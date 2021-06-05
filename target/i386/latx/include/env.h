#ifndef _ENV_H_
#define _ENV_H_

#include "flag-pattern.h"

typedef struct {
    int16 virtual_id;
    int8 physical_id;
} TEMP_REG_STATUS;

static const TEMP_REG_STATUS itemp_status_default[] = {
    {0, 4}, {0, 5},  {0, 6},  {0, 7},  {0, 8},
    {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}};

static const TEMP_REG_STATUS ftemp_status_default[] = {
    {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
};

#define itemp_status_num \
    (sizeof(itemp_status_default) / sizeof(TEMP_REG_STATUS))
#define ftemp_status_num \
    (sizeof(ftemp_status_default) / sizeof(TEMP_REG_STATUS))

typedef struct TRANSLATION_DATA {
    EXTENSION_MODE
    ireg_em[IR2_ITEMP_MAX]; /* extension mode of the 32 integer registers */
    int8 ireg_eb[IR2_ITEMP_MAX]; /* bits number where the extension starts */
    EXTENSION_MODE hi_em;
    EXTENSION_MODE lo_em;

    void *curr_tb; /* from QEMU */

    /* ir1 */
    IR1_INST *ir1_inst_array;
    int ir1_nr;
    IR1_INST *curr_ir1_inst;
    /* uint8       ir1_dump_threshold[MAX_IR1_NUM_PER_TB]; */

    /* ir2 */
    IR2_INST *ir2_inst_array;
    int ir2_inst_num_max;
    int ir2_inst_num_current;
    int real_ir2_inst_num;

    /* the list of ir2 */
    IR2_INST *first_ir2;
    IR2_INST *last_ir2;

    /* label number */
    int label_num;

    /* temp register number */
    int itemp_num;
    int ftemp_num;
    TEMP_REG_STATUS itemp_status[itemp_status_num];
    TEMP_REG_STATUS ftemp_status[ftemp_status_num];

    int curr_top;               /* top value (changes when translating) */
    int curr_esp_need_decrease; /* curr_esp need to decrease */

    /* TODO : support static translation */
    uint8 curr_ir1_skipped_eflags; /* these eflag calculation can be skipped */
                                   /* (because of flag pattern, etc) */
} TRANSLATION_DATA;

typedef struct ENV {
    void *cpu_state;            /* from QEMU,CPUArchState */
    TRANSLATION_DATA *tr_data;  /* from LATX */
#ifdef CONFIG_LATX_FLAG_PATTERN
    FLAG_PATTERN_DATA *fp_data; /* from LATX */
#endif
} ENV;

/* eflags mask */
#define CF_BIT (1 << 0)
#define PF_BIT (1 << 2)
#define AF_BIT (1 << 4)
#define ZF_BIT (1 << 6)
#define SF_BIT (1 << 7)
#define OF_BIT (1 << 11)
#define DF_BIT (1 << 10)

#define CF_BIT_INDEX 0
#define PF_BIT_INDEX 2
#define AF_BIT_INDEX 4
#define ZF_BIT_INDEX 6
#define SF_BIT_INDEX 7
#define OF_BIT_INDEX 11
#define DF_BIT_INDEX 10

/* fcsr */
#define FCSR_OFF_EN_I           0
#define FCSR_OFF_EN_U           1
#define FCSR_OFF_EN_O           2
#define FCSR_OFF_EN_Z           3
#define FCSR_OFF_EN_V           4
#define FCSR_OFF_FLAGS_I        16
#define FCSR_OFF_FLAGS_U        17
#define FCSR_OFF_FLAGS_O        18
#define FCSR_OFF_FLAGS_Z        19
#define FCSR_OFF_FLAGS_V        20
#define FCSR_OFF_CAUSE_I        24
#define FCSR_OFF_CAUSE_U        25
#define FCSR_OFF_CAUSE_O        26
#define FCSR_OFF_CAUSE_Z        27
#define FCSR_OFF_CAUSE_V        28
#define FCSR_ENABLE_SET         0x1f
#define FCSR_ENABLE_CLEAR       (~0x1f)
/* RM */
#define FCSR_OFF_RM             8
#define FCSR_RM_CLEAR           (~(0x3 << FCSR_OFF_RM))
#define FCSR_RM_RD              0x3
#define FCSR_RM_RU              0x2
#define FCSR_RM_RZ              0x1

/* x87 FPU Status Register */
#define X87_SR_OFF_IE           0
#define X87_SR_OFF_DE           1
#define X87_SR_OFF_ZE           2
#define X87_SR_OFF_OE           3
#define X87_SR_OFF_UE           4
#define X87_SR_OFF_PE           5
#define X87_SR_OFF_SF           6
#define X87_SR_OFF_ES           7
#define X87_SR_OFF_C0           8
#define X87_SR_OFF_C1           9
#define X87_SR_OFF_C2           10
#define X87_SR_OFF_C3           14

/* x87 FPU Control Register */
#define X87_CR_OFF_IM           0
#define X87_CR_OFF_DM           1
#define X87_CR_OFF_ZM           2
#define X87_CR_OFF_OM           3
#define X87_CR_OFF_UM           4
#define X87_CR_OFF_PM           5
#define X87_CR_OFF_PC           8
#define X87_CR_OFF_X            12
/* Rounding Control */
#define X87_CR_OFF_RC           10
#define X87_CR_RC_RN            0x0
#define X87_CR_RC_RD            0x1
#define X87_CR_RC_RU            0x2
#define X87_CR_RC_RZ            0x3
#endif
