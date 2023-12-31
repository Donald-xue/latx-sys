#ifndef _ENV_H_
#define _ENV_H_

#include "flag-pattern.h"
#include "cpu.h"

typedef struct {
    bool is_used;/* don't use now, may use for free in the future*/
    int16 physical_id;
} TEMP_REG_STATUS;

/* itemp */
#define ITEMP0_NUM      4
#define ITEMP1_NUM      5
#define ITEMP2_NUM      6
#define ITEMP3_NUM      7
#define ITEMP4_NUM      8
#define ITEMP5_NUM      9
#define ITEMP6_NUM      10
#define ITEMP7_NUM      11
#define ITEMP8_NUM      12
#define ITEMP9_NUM      13
/* ftemp */
#define FTEMP0_NUM      9
#define FTEMP1_NUM      10
#define FTEMP2_NUM      11
#define FTEMP3_NUM      12
#define FTEMP4_NUM      13
#define FTEMP5_NUM      14
#define FTEMP6_NUM      15

static const TEMP_REG_STATUS itemp_status_default[] = {
    {false, ITEMP0_NUM}, {false, ITEMP1_NUM},
    {false, ITEMP2_NUM}, {false, ITEMP3_NUM},
    {false, ITEMP4_NUM}, {false, ITEMP5_NUM},
    {false, ITEMP6_NUM}, {false, ITEMP7_NUM},
    {false, ITEMP8_NUM}, {false, ITEMP9_NUM}
};

#ifdef CONFIG_SOFTMMU

#ifdef TARGET_X86_64
/* temp registers in system mode : a4, a5, a7, t8, x, fp */
static const TEMP_REG_STATUS latxs_itemp_status_default[] = {
    {false, 8}, {false, 9}, {false, 11},
    {false, 20}, {false, 21}, {false, 22}
};
#else
/* temp registers in system mode : T0 - T7 */
static const TEMP_REG_STATUS latxs_itemp_status_default[] = {
    {false, 12}, {false, 13}, {false, 14}, {false, 15},
    {false, 16}, {false, 17}, {false, 18}, {false, 19}
};
#define LATXS_ITMP_CONTINUS
#define LATXS_ITMP_MIN 12
#define LATXS_ITMP_MAX 19
#endif

#define latxs_itemp_status_num \
    (sizeof(latxs_itemp_status_default) / sizeof(TEMP_REG_STATUS))

#endif

static const TEMP_REG_STATUS ftemp_status_default[] = {
    {false, FTEMP0_NUM}, {false, FTEMP1_NUM},
    {false, FTEMP2_NUM}, {false, FTEMP3_NUM},
    {false, FTEMP4_NUM}, {false, FTEMP5_NUM},
    {false, FTEMP6_NUM}
};
#define LATXS_FTMP_CONTINUS
#define LATXS_FTMP_MIN      FTEMP0_NUM
#define LATXS_FTMP_MAX      FTEMP6_NUM

#define itemp_status_num \
    (sizeof(itemp_status_default) / sizeof(TEMP_REG_STATUS))
#define ftemp_status_num \
    (sizeof(ftemp_status_default) / sizeof(TEMP_REG_STATUS))

typedef struct TRANSLATION_DATA {
    EXTENSION_MODE
    ireg_em[IR2_ITEMP_MAX]; /* extension mode of the 32 integer registers */
    int8 ireg_eb[IR2_ITEMP_MAX]; /* bits number where the extension starts */

    void *curr_tb; /* from QEMU */

    /* ir1 */
    IR1_INST *curr_ir1_inst;
    /* uint8       ir1_dump_threshold[MAX_IR1_NUM_PER_TB]; */

    /* ir2 list */
    IR2_INST *ir2_array;
    int ir2_max_nr;
    int ir2_cur_nr;
    int ir2_asm_nr;

    /* label number */
    int label_num;

    /* temp register number */
    int itemp_num;
    int ftemp_num;
    TEMP_REG_STATUS itemp_status[itemp_status_num];
    TEMP_REG_STATUS ftemp_status[ftemp_status_num];

    int curr_top;               /* top value (changes when translating) */

    /* TODO : support static translation */
    uint8 curr_ir1_skipped_eflags; /* these eflag calculation can be skipped */
                                   /* (because of flag pattern, etc) */

    /* multi code cache region */
    int region_id;

#ifdef CONFIG_SOFTMMU
    int max_insns; /* max number of target instruction */
    void *code_highwater; /* to check buffer overflow */

    IR1_INST *ir1_inst_array;
    int ir1_nr;

    int slow_path_rcd_max;
    int slow_path_rcd_nr;
    int in_gen_slow_path;
    void *slow_path_rcd;

    int x86_ins_idx[MAX_IR1_NUM_PER_TB];
    int x86_ins_lisa_nr[MAX_IR1_NUM_PER_TB];
    int x86_ins_nr;
    int x86_ins_size;

    EXMode reg_exmode[CPU_NB_REGS];
    EXBits reg_exbits[CPU_NB_REGS];

    uint32_t itemp_mask;
    uint32_t ftemp_mask;
    uint32_t itemp_mask_bk;
    int itemp_saved;

    /* flags for system-mode translation */
    struct __sys {
        uint32_t flags; /* all execution flags */
        uint32_t cflags; /* compile flags*/
        ADDRX pc;
        ADDRX cs_base;
        int pe;     /* protected mode */
        int code32; /* 32 bit code segment */
#ifdef TARGET_X86_64
        int lma;    /* long mode active */
        int code64; /* 64 bit code segment */
#endif
        int ss32;   /* 32 bit stack segment */
        int addseg; /* non zero if either DS/ES/SS have a non zero base */
        int f_st;   /* currently unused */
        int vm86;   /* vm86 mode */
        int cpl;    /* current privilege level */
        int iopl;   /* I/O privilege level */
        int tf;     /* TF cpu flag */
        int mem_index; /* select memory access functions */
        int popl_esp_hack; /* for correct popl with esp base handling */
        int cpuid_features;
        int cpuid_ext_features;
        int cpuid_ext2_features;
        int cpuid_ext3_features;
        int cpuid_7_0_ebx_features;
        int cpuid_xsave_features;
        int bp_hit;
    } sys;

    /* flags for special EOB */
    int need_eob;
    int inhibit_irq;
    int recheck_tf; /* currently not used. it is mainly for debug.*/
    /*
     * Special EOB is ended with a jmp to next instruction.
     * Some special instruction will modify eip in helper function.
     * The the execution will not continue from the next instruction.
     * Set this to ignoe the eip update in generate_exit_tb() of jmp.
     */
    int ignore_eip_update;
    int ignore_top_update;

    /*
     * @exitreq_label is used in:
     * (1) exit in icount mode             (-icount rr=... )
     * (2) proavtive checking of interrupt (-latxs sigint=0)
     * (3) dynamic checking of tb flags    (-latxs ccpro=2 )
     */
    IR2_OPND exitreq_label;

    /*
     * fail  +------------+ <--- cc_pro_check_label (TB.cc_ck_ptr)
     *  +--- | flag       |      : run with flags checking
     *  |    | check      |
     *  |    +------------+ <--- cc_pro_label (TB.cc_ok_ptr)
     *  |    | TB         |      : run without flags checking
     *  |    | codes      |
     *  +--> +------------+ <--- exitreq_label
     *       | exitreq    |
     *       +------------+
     */
    IR2_OPND cc_pro_label;
    IR2_OPND cc_pro_check_label;

    int in_gen_excp_dynamic_check;
    int end_with_exception;
    int dec_icount_inst_id;
    int cc_pro_dycheck_filter;

    /*
     * Usually sys-eob is not allowed tb-link.
     * While string instructions in icount mode
     * can do tb-link.
     */
    int sys_eob_can_link;

    int need_save_currtb_for_int;

    /*
     * for latxs_tr_gen_save_curr_top() when !LSFPU
     *
     * @curr_top_save record the last curr_top that is writen into ENV
     * - ignore saving curr_top if it is the same with last curr_top saved
     * - if @ignore_rcd_curr_top is set to 1, @curr_top_save will not be updated
     *
     * @force_curr_top_save set to 1 to force saving curr_top
     */
    int curr_top_save;
    int ignore_rcd_curr_top;
    int force_curr_top_save;
    int force_curr_top_save_bak;

    IR2_OPND jr_ra_label;
#endif

} TRANSLATION_DATA;

#ifdef CONFIG_SOFTMMU
EXMode latxs_td_get_reg_extm(int gpr);
EXBits latxs_td_get_reg_extb(int gpr);
void latxs_td_set_reg_extm(int gpr, EXMode em);
void latxs_td_set_reg_extb(int gpr, EXBits eb);
void latxs_td_set_reg_extmb(int gpr, EXMode em, EXBits eb);

void latxs_td_fpu_set_top(int ctop);
int  latxs_td_fpu_get_top(void);

typedef struct {
    void *env;
    uint32_t np_type;
    uint64_t np_regs[32];
    uint64_t np_tb_counter;
    uint32_t np_tb_flag;
    uint8_t *np_tb_chain_info;
    uint64_t np_tb_chain_nr;
    uint64_t np_tb_chain_max;
} lsenv_np_data_t;

typedef struct {
    void *env;
    void *indir_table[4][4];
    uint32_t cs_type; /*   1,2,3,4   */
} lsenv_fastcs_t;
#endif

typedef struct ENV {
    void *cpu_state;            /* from QEMU,CPUArchState */
    TRANSLATION_DATA *tr_data;  /* from LATX */
#ifdef CONFIG_LATX_FLAG_PATTERN
    FLAG_PATTERN_DATA *fp_data; /* from LATX */
#endif
#ifdef CONFIG_SOFTMMU
    /* SIGINT */
    struct {
        void *tb_unlinked;
    } sigint_data;
    int after_exec_tb_fixed;
    /* FASTCS */
    lsenv_fastcs_t fastcs_data;
    /* Native Printer */
    lsenv_np_data_t np_data;
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
#define IF_BIT (1 << 9)

#define CF_BIT_INDEX 0
#define PF_BIT_INDEX 2
#define AF_BIT_INDEX 4
#define ZF_BIT_INDEX 6
#define SF_BIT_INDEX 7
#define OF_BIT_INDEX 11
#define DF_BIT_INDEX 10
#define IF_BIT_INDEX 9

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
#define X87_SR_OFF_B            15

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
/* Exception Masks */
#define X87_CR_EXCP_MASK        0x3f
#endif
