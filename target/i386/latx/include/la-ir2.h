#ifndef __LA_IR2_H_
#define __LA_IR2_H_

//0-byte 1-half 2-word 3-double
#define VLDI_IMM_TYPE0(mode, imm) ((mode&0x3)<<10|(imm&0x3ff))
//vseti
#define VLDI_IMM_TYPE1(mode, imm) (1<<12|(mode&0x3f)<<8|(imm&0xff))
#define VEXTRINS_IMM_4_0(n, m) ((n&0xf)<<4|(m&0xf))

/* IR2_JCC; */
typedef enum {
    COND_A,		//!CF && !ZF
    COND_AE,	//!CF
    COND_B,     // CF
    COND_BE,    // CF || ZF
    COND_E,     // ZF
    COND_NE,    //!ZF
    COND_G,     //!ZF && (SF == OF)
    COND_GE,    // SF == OF
    COND_L,     // SF != OF
    COND_LE,    // ZF && (SF != OF)
    COND_S,     // SF
    COND_NS,    //!SF
    COND_O,     // OF
    COND_NO,    //!OF
    COND_PE,    // PF
    COND_PO,    //!PF
} IR2_JCC;

/* IR2_FCMP_COND; */
typedef enum {
    FCMP_COND_CAF, //0
    FCMP_COND_SAF,
    FCMP_COND_CLT,
    FCMP_COND_SLT,
    FCMP_COND_CEQ, //4
    FCMP_COND_SEQ,
    FCMP_COND_CLE,
    FCMP_COND_SLE,
    FCMP_COND_CUN, //8
    FCMP_COND_SUN,
    FCMP_COND_CULT,
    FCMP_COND_SULT,
    FCMP_COND_CUEQ, //c
    FCMP_COND_SUEQ,
    FCMP_COND_CULE,
    FCMP_COND_SULE,
    FCMP_COND_CNE, //10
    FCMP_COND_SNE,
    FCMP_COND_UND1,
    FCMP_COND_UND2,
    FCMP_COND_COR, //14
    FCMP_COND_SOR,
    FCMP_COND_UND3,
    FCMP_COND_UND4,
    FCMP_COND_CUNE, //18
    FCMP_COND_SUNE,
} IR2_FCMP_COND;

/* IR2_OPCODE; */
#include "ir2-opcode.h"

/* IR2_OPND_TYPE */
typedef enum {
    IR2_OPND_INV = 80,
    IR2_OPND_GPR,
    IR2_OPND_SCR,
    IR2_OPND_FPR,
    IR2_OPND_FCSR,    /* immediate used in cfc1/ctc1 */
    IR2_OPND_CC,    /* condition code, FCC field in FCSR */
    IR2_OPND_IMMD,  /* decimal immediate */
    IR2_OPND_IMMH,  /* hexadecimal immediate */
    IR2_OPND_LABEL,
#ifdef CONFIG_SOFTMMU
    IR2_OPND_MEMY,
#endif

    /*
     * FIXME: to make GCC happy.
     * below items will be removed once LA backend ready.
     */
    IR2_OPND_NONE = 80,
    IR2_OPND_IREG,
    IR2_OPND_NONE1,
    IR2_OPND_FREG,
    IR2_OPND_CREG,
    IR2_OPND_MEM,
    IR2_OPND_IMM,
    IR2_OPND_ADDR,
    IR2_OPND_FCC,    /* condition code, FCC field in FCSR */
} IR2_OPND_TYPE;

/* IR2_OPND */
typedef struct {
    IR2_OPND_TYPE _type;
    int val;
#ifdef CONFIG_SOFTMMU
    int imm;
    EXMode em;
    EXBits eb;
#endif
    /*
     * FIXME: below member is to make GCC happy.
     * We will remove this if LA backend ready.
     */
    int16 _reg_num;
    union {
        int16 _imm16;
        uint32 _addr;
        int32 _label_id;
    };
} IR2_OPND;
#endif
