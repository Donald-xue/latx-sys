#include "common.h"
#include "ir1.h"
#include "translate.h"
#include "latx-options.h"
#include "latx-sys-flag-rdtn.h"

#define LATXS_FLAG_DEFINE(opcode, _use, _def, _undef) \
[X86_INS_##opcode] = (IR1_EFLAG_USEDEF) \
{.use = _use, .def = _def, .undef = _undef}

/**
 * @brief ir1 opcode (x86) per instruction eflags using table
 * @note If the flag is 'unaffected', please DO NOT set at 'used' part!
 */
static const IR1_EFLAG_USEDEF latxs_ir1_opcode_eflag_usedef[] = {
    LATXS_FLAG_DEFINE(INVALID, __INVALID, __INVALID, __INVALID),
    LATXS_FLAG_DEFINE(AAA,     __AF, __ALL_EFLAGS, __OF | __SF | __ZF | __PF),
    LATXS_FLAG_DEFINE(AAD,     __NONE, __ALL_EFLAGS, __OF | __AF | __CF),
    LATXS_FLAG_DEFINE(AAM,     __NONE, __ALL_EFLAGS, __OF | __AF | __CF),
    LATXS_FLAG_DEFINE(AAS,     __AF, __ALL_EFLAGS, __OF | __SF | __ZF | __PF),
    LATXS_FLAG_DEFINE(ADC,     __CF, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(ADD,     __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(AND,     __NONE, __ALL_EFLAGS, __AF),
    LATXS_FLAG_DEFINE(ARPL,    __NONE, __ZF, __NONE),
    LATXS_FLAG_DEFINE(BOUND,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(BSF,     __NONE, __ALL_EFLAGS, __OSAPF | __CF),
    LATXS_FLAG_DEFINE(BSR,     __NONE, __ALL_EFLAGS, __OSAPF | __CF),
    LATXS_FLAG_DEFINE(BSWAP,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(BT,      __NONE, __OSAPF | __CF, __OSAPF),
    LATXS_FLAG_DEFINE(BTC,     __NONE, __OSAPF | __CF, __OSAPF),
    LATXS_FLAG_DEFINE(BTR,     __NONE, __OSAPF | __CF, __OSAPF),
    LATXS_FLAG_DEFINE(BTS,     __NONE, __OSAPF | __CF, __OSAPF),
    LATXS_FLAG_DEFINE(CALL,    __NONE, __NONE, __NONE), /* change */
    LATXS_FLAG_DEFINE(CBW,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CLC,     __NONE, __CF, __NONE),
    LATXS_FLAG_DEFINE(CLD,     __NONE, __DF, __NONE),
    /* LATXS_FLAG_DEFINE(CLI,     __NONE, __INVALID, __NONE), */
    /* LATXS_FLAG_DEFINE(CLTS,     __NONE, __INVALID, __NONE), */
    LATXS_FLAG_DEFINE(CMC,     __CF, __CF, __NONE),
    /* CMOVcc */
    LATXS_FLAG_DEFINE(CMOVA,   __ZF | __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVAE,  __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVB,   __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVBE,  __ZF | __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVE,   __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVG,   __OF | __SF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVGE,  __OF | __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVL,   __OF | __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVLE,  __OF | __SF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVNE,  __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVO,   __OF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVNO,  __OF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVP,   __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVNP,  __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVS,   __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVNS,  __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVLE,  __OF | __SF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVLE,  __OF | __SF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CMOVLE,  __OF | __SF | __ZF, __NONE, __NONE),

    LATXS_FLAG_DEFINE(CMP,     __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(CMPSB,   __DF, __ALL_EFLAGS, __NONE), /* change */
    LATXS_FLAG_DEFINE(CMPSW,   __DF, __ALL_EFLAGS, __NONE), /* change */
    LATXS_FLAG_DEFINE(CMPSD,   __DF, __ALL_EFLAGS, __NONE), /* change */
    LATXS_FLAG_DEFINE(CMPSQ,   __DF, __ALL_EFLAGS, __NONE), /* change */
    LATXS_FLAG_DEFINE(CMPXCHG, __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(CMPXCHG8B, __NONE, __ZF, __NONE),
    LATXS_FLAG_DEFINE(COMISD,  __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(COMISS,  __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(CPUID,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(CWD,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(DAA,     __AF | __CF, __ALL_EFLAGS, __OF), /* change */
    LATXS_FLAG_DEFINE(DAS,     __AF | __CF, __ALL_EFLAGS, __OF), /* change */
    LATXS_FLAG_DEFINE(DEC,     __NONE, __OF | __SZAPF, __NONE),
    LATXS_FLAG_DEFINE(DIV,     __NONE, __ALL_EFLAGS, __ALL_EFLAGS),
    LATXS_FLAG_DEFINE(ENTER,   __NONE, __NONE, __NONE),

    /* LATXS_FLAG_DEFINE(ESC,     __NONE, __INVALID, __NONE), */
    /* FCMOVcc */
    LATXS_FLAG_DEFINE(FCMOVB,  __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVBE, __ZF | __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVE,  __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVNB, __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVNBE, __ZF | __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVNE, __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVNU, __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(FCMOVU,  __PF, __NONE, __NONE),

    LATXS_FLAG_DEFINE(FCOMI,   __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(FCOMIP,  __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(FUCOMI,  __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(FUCOMIP, __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(HLT,     __NONE, __NONE, __NONE), /* change */
    LATXS_FLAG_DEFINE(IDIV,    __NONE, __ALL_EFLAGS, __ALL_EFLAGS),
    LATXS_FLAG_DEFINE(IMUL,    __NONE, __ALL_EFLAGS, __SZAPF),
    LATXS_FLAG_DEFINE(IN,      __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(INC,     __NONE, __OF | __SZAPF, __NONE),
    LATXS_FLAG_DEFINE(INSB,    __NONE, __DF, __NONE), /* change */
    LATXS_FLAG_DEFINE(INSW,    __NONE, __DF, __NONE),
    LATXS_FLAG_DEFINE(INSD,    __NONE, __DF, __NONE),
    LATXS_FLAG_DEFINE(INT,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(INTO,    __OF, __NONE, __NONE), /* change */
    LATXS_FLAG_DEFINE(INVD,    __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(INVLPG,  __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(UCOMISD, __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(UCOMISS, __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(IRET,    __NONE, __INVALID, __NONE),

    /* Jcc */
    LATXS_FLAG_DEFINE(JO,      __OF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JNO,     __OF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JB,      __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JAE,     __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JE,      __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JNE,     __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JBE,     __CF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JA,      __CF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JS,      __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JNS,     __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JP,      __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JNP,     __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JL,      __OF | __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JGE,     __OF | __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JLE,     __OF | __SF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JG,      __OF | __SF | __ZF, __NONE, __NONE),

    LATXS_FLAG_DEFINE(JCXZ,    __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(JMP,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(LAHF,    __SZAPCF, __NONE, __NONE), /* change */
    /* INVALID */
    LATXS_FLAG_DEFINE(LAR,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LDS,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LES,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LSS,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LFS,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LGS,     __NONE, __INVALID, __NONE),

    LATXS_FLAG_DEFINE(LEA,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(LEAVE,   __NONE, __NONE, __NONE),
    /* INVALID */
    LATXS_FLAG_DEFINE(LGDT,    __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LIDT,    __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LLDT,    __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LMSW,    __NONE, __INVALID, __NONE),

    /* LATXS_FLAG_DEFINE(LOCK,    __NONE, __INVALID, __NONE), */
    /* lods */
    LATXS_FLAG_DEFINE(LODSB,   __DF, __NONE, __NONE), /* change */
    LATXS_FLAG_DEFINE(LODSW,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(LODSD,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(LODSQ,   __DF, __NONE, __NONE),

    LATXS_FLAG_DEFINE(LOOP,    __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(LOOPE,   __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(LOOPNE,  __ZF, __NONE, __NONE),

    LATXS_FLAG_DEFINE(LSL,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(LTR,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(MONITOR, __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(MWAIT,   __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(MOV,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(MOVABS,  __NONE, __NONE, __NONE),
    /* movs */
    LATXS_FLAG_DEFINE(MOVSB,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(MOVSW,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(MOVSD,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(MOVSQ,   __DF, __NONE, __NONE),

    LATXS_FLAG_DEFINE(MOVSX,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(MOVZX,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(MUL,     __NONE, __ALL_EFLAGS, __SZAPF),
    LATXS_FLAG_DEFINE(NEG,     __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(NOP,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(NOT,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(OR,      __NONE, __ALL_EFLAGS, __AF),
    LATXS_FLAG_DEFINE(OUT,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(OUTSB,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(OUTSW,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(OUTSD,   __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(POP,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(POPAW,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(POPAL,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(POPF,    __NONE, __ALL_EFLAGS | __DF, __NONE),
    LATXS_FLAG_DEFINE(POPFD,   __NONE, __ALL_EFLAGS | __DF, __NONE),
    LATXS_FLAG_DEFINE(POPFQ,   __NONE, __ALL_EFLAGS | __DF, __NONE),
    LATXS_FLAG_DEFINE(PUSH,    __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(PUSHAW,  __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(PUSHAL,  __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(PUSHF,   __ALL_EFLAGS | __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(PUSHFD,  __ALL_EFLAGS | __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(PUSHFQ,  __ALL_EFLAGS | __DF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(RCL,     __CF, __CF | __OF, __NONE),
    LATXS_FLAG_DEFINE(RCR,     __CF, __CF | __OF, __NONE),
    LATXS_FLAG_DEFINE(RDMSR,   __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(RDPMC,   __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(RDTSC,   __NONE, __NONE, __NONE),
    /* LATXS_FLAG_DEFINE(REP,     __NONE, __INVALID, __NONE), */
    /* LATXS_FLAG_DEFINE(REPNE,     __NONE, __INVALID, __NONE), */
    LATXS_FLAG_DEFINE(RET,     __NONE, __NONE, __NONE), /* change */
    LATXS_FLAG_DEFINE(ROL,     __CF, __OF | __CF, __NONE),
    LATXS_FLAG_DEFINE(ROR,     __CF, __OF | __CF, __NONE),
    LATXS_FLAG_DEFINE(RSM,     __NONE, __ALL_EFLAGS | __DF, __NONE),
    LATXS_FLAG_DEFINE(SAHF,    __NONE, __SZAPCF, __NONE),
    LATXS_FLAG_DEFINE(SAL,     __NONE, __ALL_EFLAGS, __AF), /* change */
    LATXS_FLAG_DEFINE(SAR,     __NONE, __ALL_EFLAGS, __AF),
    LATXS_FLAG_DEFINE(SHL,     __NONE, __ALL_EFLAGS, __AF),
    LATXS_FLAG_DEFINE(SHR,     __NONE, __ALL_EFLAGS, __AF),
    LATXS_FLAG_DEFINE(SBB,     __CF, __ALL_EFLAGS, __NONE),
    /* scas */
    LATXS_FLAG_DEFINE(SCASB,   __DF, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(SCASW,   __DF, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(SCASD,   __DF, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(SCASQ,   __DF, __ALL_EFLAGS, __NONE),
    /* SETcc */
    LATXS_FLAG_DEFINE(SETO,    __OF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETNO,   __OF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETB,    __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETAE,   __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETE,    __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETNE,   __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETBE,   __ZF | __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETA,    __ZF | __CF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETS,    __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETNS,   __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETP,    __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETNP,   __PF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETL,    __OF | __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETGE,   __OF | __SF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETLE,   __OF | __SF | __ZF, __NONE, __NONE),
    LATXS_FLAG_DEFINE(SETG,    __OF | __SF | __ZF, __NONE, __NONE),
    /* INVALID */
    LATXS_FLAG_DEFINE(SGDT,    __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(SIDT,    __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(SLDT,    __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(SMSW,    __NONE, __INVALID, __NONE),

    LATXS_FLAG_DEFINE(SHLD,    __NONE, __ALL_EFLAGS, __AF), /* change */
    LATXS_FLAG_DEFINE(SHRD,    __NONE, __ALL_EFLAGS, __AF),

    LATXS_FLAG_DEFINE(STC,     __NONE, __CF, __NONE),
    LATXS_FLAG_DEFINE(STD,     __NONE, __DF, __NONE),
    LATXS_FLAG_DEFINE(STI,     __NONE, __INVALID, __NONE),
    /* stos */
    LATXS_FLAG_DEFINE(STOSB,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(STOSW,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(STOSD,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(STOSQ,   __NONE, __NONE, __NONE),

    LATXS_FLAG_DEFINE(STR,     __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(SUB,     __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(TEST,    __NONE, __ALL_EFLAGS, __AF),
    LATXS_FLAG_DEFINE(TZCNT,   __NONE, __ALL_EFLAGS, __OSAPF),
    LATXS_FLAG_DEFINE(UD0,     __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(VERR,    __NONE, __ZF, __NONE),
    LATXS_FLAG_DEFINE(VERW,    __NONE, __ZF, __NONE),
    LATXS_FLAG_DEFINE(WAIT,    __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(WBINVD,  __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(WRMSR,   __NONE, __INVALID, __NONE),
    LATXS_FLAG_DEFINE(XADD,    __NONE, __ALL_EFLAGS, __NONE),
    LATXS_FLAG_DEFINE(XCHG,    __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(XLATB,   __NONE, __NONE, __NONE),
    LATXS_FLAG_DEFINE(XOR,     __NONE, __ALL_EFLAGS, __AF),

    LATXS_FLAG_DEFINE(ENDING,  __INVALID, __INVALID, __INVALID),
};

static const IR1_EFLAG_USEDEF ir1_eflag_none = {0, 0, 0};
static const IR1_EFLAG_USEDEF ir1_eflag_all  = {__ALL_EFLAGS, 0, 0};

static const IR1_EFLAG_USEDEF *latxs_ir1_opcode_to_eflag_usedef(IR1_INST *ir1)
{
    if (ir1_opcode(ir1) == X86_INS_MOVSD) {
        if (ir1->info->detail->x86.opcode[0] == 0xa5) {
            return latxs_ir1_opcode_eflag_usedef +
                (X86_INS_MOVSB - X86_INS_INVALID);
        }
    }

    if (ir1_opcode(ir1) == X86_INS_INVALID) {
        if (ir1->flags == LATXS_IR1_FLAGS_GENNOP)
            return &ir1_eflag_none;
        if (ir1->flags == LATXS_IR1_FLAGS_ILLOP)
            return &ir1_eflag_all;
    }

    lsassertm((latxs_ir1_opcode_eflag_usedef +
        (ir1_opcode(ir1) - X86_INS_INVALID))->use !=
         __INVALID, "%s\n", ir1->info->mnemonic);
    return latxs_ir1_opcode_eflag_usedef + (ir1_opcode(ir1) - X86_INS_INVALID);
}

#ifdef LATXS_FLAGRDTN_ENABLE

static inline bool cmp_scas_need_zf(IR1_INST *pir1)
{
    return (ir1_prefix(pir1) == X86_PREFIX_REPE ||
            ir1_prefix(pir1) == X86_PREFIX_REPNE) &&
           (ir1_opcode(pir1) == X86_INS_CMPSB ||
            ir1_opcode(pir1) == X86_INS_CMPSW ||
            ir1_opcode(pir1) == X86_INS_CMPSD ||
            ir1_opcode(pir1) == X86_INS_CMPSQ ||
            ir1_opcode(pir1) == X86_INS_SCASB ||
            ir1_opcode(pir1) == X86_INS_SCASW ||
            ir1_opcode(pir1) == X86_INS_SCASD ||
            ir1_opcode(pir1) == X86_INS_SCASQ);
}

static void __latxs_flag_reduction_ir1(IR1_INST *pir1, uint8 *pending_use)
{
    uint8 current_def = __NONE;
    /*
     * 1. Get current inst flag defination
     *   - flag def:     current inst will generate flags
     *   - flag use:     current inst will use flags
     *   - flag undef:   current inst mark undef flags
     */
    IR1_EFLAG_USEDEF curr_usedef = *latxs_ir1_opcode_to_eflag_usedef(pir1);

    /*
     * 2. Drop the flag which did not need.
     *   - Pending will mark which are needed.
     */
    curr_usedef.def &= *pending_use;

    /*
     * 3. Recalculate the pending
     *   - Drop the current calculated flags.
     */
    *pending_use &= (~curr_usedef.def);

    /*
     * CMPSx and SCASx will use REPNZ/REPZ
     */
    if (cmp_scas_need_zf(pir1)) {
        /* "rep cmps" need add zf */
        curr_usedef.use |= __ZF;
    }

    /*
     * 4. Add the pending which will be used
     *   - Current inst may use some flags, so we need add to pending.
     */
    *pending_use |= curr_usedef.use;
    current_def  |= curr_usedef.def;
    ir1_set_eflag_use(pir1, curr_usedef.use);
    ir1_set_eflag_def(pir1, current_def & (~curr_usedef.undef));
}

void __latxs_flag_reduction(TranslationBlock *tb)
{
    latxs_flag_check(option_sys_flag_reduction, tb);
    latxs_flag_cpl_filter(tb);

    IR1_INST *ir1 = NULL;
    /* cross scanning var defination */
    uint8 rdtn_pending_use =  __ALL_EFLAGS;
    /* scanning instructions in reverse order */
    for (int i = tb_ir1_num(tb) - 1; i >= 0; --i) {
        ir1 = tb_ir1_inst(tb, i);
        /* do core optimize */
        __latxs_flag_reduction_ir1(ir1, &rdtn_pending_use);
    }
}

#endif /* LATXS_FLAGRDTN_ENABLE */
