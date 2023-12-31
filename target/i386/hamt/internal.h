#ifndef INTERNAL_H_6IUWCEFP
#define INTERNAL_H_6IUWCEFP

/* Loongarch KVM register ids */
#define LOONGARCH_CSR_32(_R, _S)                                               \
    (KVM_REG_LOONGARCH_CSR | KVM_REG_SIZE_U32 | (8 * (_R) + (_S)))

#define LOONGARCH_CSR_64(_R, _S)                                               \
    (KVM_REG_LOONGARCH_CSR | KVM_REG_SIZE_U64 | (8 * (_R) + (_S)))

#define KVM_LOONGARCH_CSR_CRMD 0
#define KVM_LOONGARCH_CSR_PRMD 1
#define KVM_LOONGARCH_CSR_EUEN 2
#define KVM_LOONGARCH_CSR_MISC 3
#define KVM_LOONGARCH_CSR_ECFG 4
#define KVM_LOONGARCH_CSR_ESTAT 5
#define KVM_LOONGARCH_CSR_EPC 6
#define KVM_LOONGARCH_CSR_BADV 7
#define KVM_LOONGARCH_CSR_BADI 8
#define KVM_LOONGARCH_CSR_EBASE 0xc
#define KVM_LOONGARCH_CSR_TLBIDX 0x10
#define KVM_LOONGARCH_CSR_TLBHI 0x11
#define KVM_LOONGARCH_CSR_TLBLO0 0x12
#define KVM_LOONGARCH_CSR_TLBLO1 0x13
#define KVM_LOONGARCH_CSR_GTLBC 0x15
#define KVM_LOONGARCH_CSR_TRGP 0x16
#define KVM_LOONGARCH_CSR_ASID 0x18
#define KVM_LOONGARCH_CSR_PGDL 0x19
#define KVM_LOONGARCH_CSR_PGDH 0x1a
#define KVM_LOONGARCH_CSR_PGD 0x1b
#define KVM_LOONGARCH_CSR_PWCTL0 0x1c
#define KVM_LOONGARCH_CSR_PWCTL1 0x1d
#define KVM_LOONGARCH_CSR_STLBPS 0x1e
#define KVM_LOONGARCH_CSR_RVACFG 0x1f
#define KVM_LOONGARCH_CSR_CPUNUM 0x20
#define KVM_LOONGARCH_CSR_PRCFG1 0x21
#define KVM_LOONGARCH_CSR_PRCFG2 0x22
#define KVM_LOONGARCH_CSR_PRCFG3 0x23
#define KVM_LOONGARCH_CSR_KSCRATCH0 0x30
#define KVM_LOONGARCH_CSR_KSCRATCH1 0x31
#define KVM_LOONGARCH_CSR_KSCRATCH2 0x32
#define KVM_LOONGARCH_CSR_KSCRATCH3 0x33
#define KVM_LOONGARCH_CSR_KSCRATCH4 0x34
#define KVM_LOONGARCH_CSR_KSCRATCH5 0x35
#define KVM_LOONGARCH_CSR_KSCRATCH6 0x36
#define KVM_LOONGARCH_CSR_KSCRATCH7 0x37
#define KVM_LOONGARCH_CSR_KSCRATCH8 0x38
#define KVM_LOONGARCH_CSR_TIMERID 0x40
#define KVM_LOONGARCH_CSR_TIMERCFG 0x41
#define KVM_LOONGARCH_CSR_TIMERTICK 0x42
#define KVM_LOONGARCH_CSR_TIMEROFFSET 0x43
#define KVM_LOONGARCH_CSR_GSTAT 0x50
#define KVM_LOONGARCH_CSR_GCFG 0x51
#define KVM_LOONGARCH_CSR_GINTC 0x52
#define KVM_LOONGARCH_CSR_GCNTC 0x53
#define KVM_LOONGARCH_CSR_LLBCTL 0x60
#define KVM_LOONGARCH_CSR_IMPCTL1 0x80
#define KVM_LOONGARCH_CSR_IMPCTL2 0x81
#define KVM_LOONGARCH_CSR_GNMI 0x82
#define KVM_LOONGARCH_CSR_TLBREBASE 0x88
#define KVM_LOONGARCH_CSR_TLBRBADV 0x89
#define KVM_LOONGARCH_CSR_TLBREPC 0x8a
#define KVM_LOONGARCH_CSR_TLBRSAVE 0x8b
#define KVM_LOONGARCH_CSR_TLBRELO0 0x8c
#define KVM_LOONGARCH_CSR_TLBRELO1 0x8d
#define KVM_LOONGARCH_CSR_TLBREHI 0x8e
#define KVM_LOONGARCH_CSR_TLBRPRMD 0x8f
#define KVM_LOONGARCH_CSR_ERRCTL 0x90
#define KVM_LOONGARCH_CSR_ERRINFO1 0x91
#define KVM_LOONGARCH_CSR_ERRINFO2 0x92
#define KVM_LOONGARCH_CSR_ERREBASE 0x93
#define KVM_LOONGARCH_CSR_ERREPC 0x94
#define KVM_LOONGARCH_CSR_ERRSAVE 0x95
#define KVM_LOONGARCH_CSR_CTAG 0x98
#define KVM_LOONGARCH_CSR_MCSR0 0xc0
#define KVM_LOONGARCH_CSR_MCSR1 0xc1
#define KVM_LOONGARCH_CSR_MCSR2 0xc2
#define KVM_LOONGARCH_CSR_MCSR3 0xc3
#define KVM_LOONGARCH_CSR_MCSR8 0xc8
#define KVM_LOONGARCH_CSR_MCSR9 0xc9
#define KVM_LOONGARCH_CSR_MCSR10 0xca
#define KVM_LOONGARCH_CSR_MCSR24 0xf0
#define KVM_LOONGARCH_CSR_UCAWIN 0x100
#define KVM_LOONGARCH_CSR_UCAWIN0_LO 0x102
#define KVM_LOONGARCH_CSR_UCAWIN0_HI 0x103
#define KVM_LOONGARCH_CSR_UCAWIN1_LO 0x104
#define KVM_LOONGARCH_CSR_UCAWIN1_HI 0x105
#define KVM_LOONGARCH_CSR_UCAWIN2_LO 0x106
#define KVM_LOONGARCH_CSR_UCAWIN2_HI 0x107
#define KVM_LOONGARCH_CSR_UCAWIN3_LO 0x108
#define KVM_LOONGARCH_CSR_UCAWIN3_HI 0x109
#define KVM_LOONGARCH_CSR_DMWIN0 0x180
#define KVM_LOONGARCH_CSR_DMWIN1 0x181
#define KVM_LOONGARCH_CSR_DMWIN2 0x182
#define KVM_LOONGARCH_CSR_DMWIN3 0x183
#define KVM_LOONGARCH_CSR_PERF0_EVENT 0x200
#define KVM_LOONGARCH_CSR_PERF0_COUNT 0x201
#define KVM_LOONGARCH_CSR_PERF1_EVENT 0x202
#define KVM_LOONGARCH_CSR_PERF1_COUNT 0x203
#define KVM_LOONGARCH_CSR_PERF2_EVENT 0x204
#define KVM_LOONGARCH_CSR_PERF2_COUNT 0x205
#define KVM_LOONGARCH_CSR_PERF3_EVENT 0x206
#define KVM_LOONGARCH_CSR_PERF3_COUNT 0x207
#define KVM_LOONGARCH_CSR_DEBUG 0x500
#define KVM_LOONGARCH_CSR_DEPC 0x501
#define KVM_LOONGARCH_CSR_DESAVE 0x502

#define KVM_CSR_CRMD LOONGARCH_CSR_64(0, 0)
#define KVM_CSR_PRMD LOONGARCH_CSR_64(1, 0)
#define KVM_CSR_EUEN LOONGARCH_CSR_64(2, 0)
#define KVM_CSR_MISC LOONGARCH_CSR_64(3, 0)
#define KVM_CSR_ECFG LOONGARCH_CSR_64(4, 0)
#define KVM_CSR_ESTAT LOONGARCH_CSR_64(5, 0)
#define KVM_CSR_EPC LOONGARCH_CSR_64(6, 0)
#define KVM_CSR_BADV LOONGARCH_CSR_64(7, 0)
#define KVM_CSR_BADI LOONGARCH_CSR_64(8, 0)
#define KVM_CSR_EBASE LOONGARCH_CSR_64(0xc, 0)
#define KVM_CSR_TLBIDX LOONGARCH_CSR_64(0x10, 0)
#define KVM_CSR_TLBHI LOONGARCH_CSR_64(0x11, 0)
#define KVM_CSR_TLBLO0 LOONGARCH_CSR_64(0x12, 0)
#define KVM_CSR_TLBLO1 LOONGARCH_CSR_64(0x13, 0)
#define KVM_CSR_GTLBC LOONGARCH_CSR_64(0x15, 0)
#define KVM_CSR_TRGP LOONGARCH_CSR_64(0x16, 0)
#define KVM_CSR_ASID LOONGARCH_CSR_64(0x18, 0)
#define KVM_CSR_PGDL LOONGARCH_CSR_64(0x19, 0)
#define KVM_CSR_PGDH LOONGARCH_CSR_64(0x1a, 0)
#define KVM_CSR_PGD LOONGARCH_CSR_64(0x1b, 0)
#define KVM_CSR_PWCTL0 LOONGARCH_CSR_64(0x1c, 0)
#define KVM_CSR_PWCTL1 LOONGARCH_CSR_64(0x1d, 0)
#define KVM_CSR_STLBPS LOONGARCH_CSR_64(0x1e, 0)
#define KVM_CSR_RVACFG LOONGARCH_CSR_64(0x1f, 0)
#define KVM_CSR_CPUNUM LOONGARCH_CSR_64(0x20, 0)
#define KVM_CSR_PRCFG1 LOONGARCH_CSR_64(0x21, 0)
#define KVM_CSR_PRCFG2 LOONGARCH_CSR_64(0x22, 0)
#define KVM_CSR_PRCFG3 LOONGARCH_CSR_64(0x23, 0)
#define KVM_CSR_KSCRATCH0 LOONGARCH_CSR_64(0x30, 0)
#define KVM_CSR_KSCRATCH1 LOONGARCH_CSR_64(0x31, 0)
#define KVM_CSR_KSCRATCH2 LOONGARCH_CSR_64(0x32, 0)
#define KVM_CSR_KSCRATCH3 LOONGARCH_CSR_64(0x33, 0)
#define KVM_CSR_KSCRATCH4 LOONGARCH_CSR_64(0x34, 0)
#define KVM_CSR_KSCRATCH5 LOONGARCH_CSR_64(0x35, 0)
#define KVM_CSR_KSCRATCH6 LOONGARCH_CSR_64(0x36, 0)
#define KVM_CSR_KSCRATCH7 LOONGARCH_CSR_64(0x37, 0)
#define KVM_CSR_KSCRATCH8 LOONGARCH_CSR_64(0x38, 0)
#define KVM_CSR_TIMERID LOONGARCH_CSR_64(0x40, 0)
#define KVM_CSR_TIMERCFG LOONGARCH_CSR_64(0x41, 0)
#define KVM_CSR_TIMERTICK LOONGARCH_CSR_64(0x42, 0)
#define KVM_CSR_TIMEROFFSET LOONGARCH_CSR_64(0x43, 0)
#define KVM_CSR_GSTAT LOONGARCH_CSR_64(0x50, 0)
#define KVM_CSR_GCFG LOONGARCH_CSR_64(0x51, 0)
#define KVM_CSR_GINTC LOONGARCH_CSR_64(0x52, 0)
#define KVM_CSR_GCNTC LOONGARCH_CSR_64(0x53, 0)
#define KVM_CSR_LLBCTL LOONGARCH_CSR_64(0x60, 0)
#define KVM_CSR_IMPCTL1 LOONGARCH_CSR_64(0x80, 0)
#define KVM_CSR_IMPCTL2 LOONGARCH_CSR_64(0x81, 0)
#define KVM_CSR_GNMI LOONGARCH_CSR_64(0x82, 0)
#define KVM_CSR_TLBREBASE LOONGARCH_CSR_64(0x88, 0)
#define KVM_CSR_TLBRBADV LOONGARCH_CSR_64(0x89, 0)
#define KVM_CSR_TLBREPC LOONGARCH_CSR_64(0x8a, 0)
#define KVM_CSR_TLBRSAVE LOONGARCH_CSR_64(0x8b, 0)
#define KVM_CSR_TLBRELO0 LOONGARCH_CSR_64(0x8c, 0)
#define KVM_CSR_TLBRELO1 LOONGARCH_CSR_64(0x8d, 0)
#define KVM_CSR_TLBREHI LOONGARCH_CSR_64(0x8e, 0)
#define KVM_CSR_TLBRPRMD LOONGARCH_CSR_64(0x8f, 0)
#define KVM_CSR_ERRCTL LOONGARCH_CSR_64(0x90, 0)
#define KVM_CSR_ERRINFO1 LOONGARCH_CSR_64(0x91, 0)
#define KVM_CSR_ERRINFO2 LOONGARCH_CSR_64(0x92, 0)
#define KVM_CSR_ERREBASE LOONGARCH_CSR_64(0x93, 0)
#define KVM_CSR_ERREPC LOONGARCH_CSR_64(0x94, 0)
#define KVM_CSR_ERRSAVE LOONGARCH_CSR_64(0x95, 0)
#define KVM_CSR_CTAG LOONGARCH_CSR_64(0x98, 0)
#define KVM_CSR_MCSR0 LOONGARCH_CSR_64(0xc0, 0)
#define KVM_CSR_MCSR1 LOONGARCH_CSR_64(0xc1, 0)
#define KVM_CSR_MCSR2 LOONGARCH_CSR_64(0xc2, 0)
#define KVM_CSR_MCSR3 LOONGARCH_CSR_64(0xc3, 0)
#define KVM_CSR_MCSR8 LOONGARCH_CSR_64(0xc8, 0)
#define KVM_CSR_MCSR9 LOONGARCH_CSR_64(0xc9, 0)
#define KVM_CSR_MCSR10 LOONGARCH_CSR_64(0xca, 0)
#define KVM_CSR_MCSR24 LOONGARCH_CSR_64(0xf0, 0)
#define KVM_CSR_UCWIN LOONGARCH_CSR_64(0x100, 0)
#define KVM_CSR_UCWIN0_LO LOONGARCH_CSR_64(0x102, 0)
#define KVM_CSR_UCWIN0_HI LOONGARCH_CSR_64(0x103, 0)
#define KVM_CSR_UCWIN1_LO LOONGARCH_CSR_64(0x104, 0)
#define KVM_CSR_UCWIN1_HI LOONGARCH_CSR_64(0x105, 0)
#define KVM_CSR_UCWIN2_LO LOONGARCH_CSR_64(0x106, 0)
#define KVM_CSR_UCWIN2_HI LOONGARCH_CSR_64(0x107, 0)
#define KVM_CSR_UCWIN3_LO LOONGARCH_CSR_64(0x108, 0)
#define KVM_CSR_UCWIN3_HI LOONGARCH_CSR_64(0x109, 0)
#define KVM_CSR_DMWIN0 LOONGARCH_CSR_64(0x180, 0)
#define KVM_CSR_DMWIN1 LOONGARCH_CSR_64(0x181, 0)
#define KVM_CSR_DMWIN2 LOONGARCH_CSR_64(0x182, 0)
#define KVM_CSR_DMWIN3 LOONGARCH_CSR_64(0x183, 0)
#define KVM_CSR_PERF0_EVENT LOONGARCH_CSR_64(0x200, 0)
#define KVM_CSR_PERF0_COUNT LOONGARCH_CSR_64(0x201, 0)
#define KVM_CSR_PERF1_EVENT LOONGARCH_CSR_64(0x202, 0)
#define KVM_CSR_PERF1_COUNT LOONGARCH_CSR_64(0x203, 0)
#define KVM_CSR_PERF2_EVENT LOONGARCH_CSR_64(0x204, 0)
#define KVM_CSR_PERF2_COUNT LOONGARCH_CSR_64(0x205, 0)
#define KVM_CSR_PERF3_EVENT LOONGARCH_CSR_64(0x206, 0)
#define KVM_CSR_PERF3_COUNT LOONGARCH_CSR_64(0x207, 0)
#define KVM_CSR_DEBUG LOONGARCH_CSR_64(0x500, 0)
#define KVM_CSR_DEPC LOONGARCH_CSR_64(0x501, 0)
#define KVM_CSR_DESAVE LOONGARCH_CSR_64(0x502, 0)

// following copied from arch/loongarch/include/asm/loongarchregs.h
/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#define _U64CAST_
#else
#define _ULCAST_ (unsigned long)
#define _U64CAST_ (u64)
#endif

#ifdef __ASSEMBLY__
#define _ATYPE_
#define _ATYPE32_
#define _ATYPE64_
#define _CONST64_(x) x
#else
#define _ATYPE_ __PTRDIFF_TYPE__
#define _ATYPE32_ int
#define _ATYPE64_ __s64
#ifdef CONFIG_64BIT
#define _CONST64_(x) x##L
#else
#define _CONST64_(x) x##LL
#endif
#endif

#define LOONGARCH_CSR_CRMD      0x0
#define  CSR_CRMD_PG_SHIFT      4
#define  CSR_CRMD_PG            (_ULCAST_(0x1) << CSR_CRMD_PG_SHIFT)
#define  CSR_CRMD_DA_SHIFT      3
#define  CSR_CRMD_DA            (_ULCAST_(0x1) << CSR_CRMD_DA_SHIFT)

/* TLB refill registers */
#define LOONGARCH_CSR_TLBREBASE 0x88 /* TLB refill exception base address */
#define LOONGARCH_CSR_TLBRBADV  0x89 /* TLB refill badvaddr */
#define LOONGARCH_CSR_TLBREPC   0x8a /* TLB refill EPC */
#define LOONGARCH_CSR_TLBRSAVE  0x8b /* KScratch for TLB refill exception */
#define LOONGARCH_CSR_TLBRELO0  0x8c /* TLB refill entrylo0 */
#define LOONGARCH_CSR_TLBRELO1  0x8d /* TLB refill entrylo1 */
#define LOONGARCH_CSR_TLBREHI   0x8e /* TLB refill entryhi */
#define LOONGARCH_CSR_TLBRPRMD  0x8f /* TLB refill mode info */
#define LOONGARCH_CSR_ASID      0x18 /* ASID */
#define LOONGARCH_CSR_EPC       0x6  /* EPC */
#define LOONGARCH_CSR_BADV      0x7  /* Bad virtual address */
#define LOONGARCH_CSR_TLBEHI    0x11 /* TLB EntryHi */
#define LOONGARCH_CSR_TLBIDX    0x10
#define LOONGARCH_CSR_TLBELO0   0x12
#define LOONGARCH_CSR_TLBELO1   0x13

#define DMW_PABITS 48
#define CSR_DMW1_PLV0 _CONST64_(1 << 0)
#define CSR_DMW1_MAT _CONST64_(1 << 4)
#define CSR_DMW1_VSEG _CONST64_(0x9000)
#define CSR_DMW1_BASE (CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT (CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

/* Kscratch registers */
#define LOONGARCH_CSR_KS0 0x30
#define LOONGARCH_CSR_KS1 0x31
#define LOONGARCH_CSR_KS2 0x32
#define LOONGARCH_CSR_KS3 0x33
#define LOONGARCH_CSR_KS4 0x34
#define LOONGARCH_CSR_KS5 0x35
#define LOONGARCH_CSR_KS6 0x36
#define LOONGARCH_CSR_KS7 0x37
#define LOONGARCH_CSR_KS8 0x38

/* TLB exception allocated KS0 and KS1 statically */
#define TLB_EXC_KS0 LOONGARCH_CSR_KS0
#define TLB_EXC_KS1 LOONGARCH_CSR_KS1
#define TLB_KSCRATCH_MASK (1 << 0 | 1 << 1)

/* KVM allocated KS2 and KS3 statically */
#define KVM_VCPU_KS LOONGARCH_CSR_KS2
#define KVM_TEMP_KS LOONGARCH_CSR_KS3
#define KVM_KSCRATCH_MASK (1 << 2 | 1 << 3)

/* Percpu allocated KS4 */
#define PERCPU_KS4 LOONGARCH_CSR_KS4

#define fcsr0 $r0
#define fcsr1 $r1
#define fcsr2 $r2
#define fcsr3 $r3
#define vcsr16 $r16

// copied from arch/loongarch/include/asm/regdef.h
#define zero $r0 /* wired zero */
#define ra $r1 /* return address */
#define tp $r2
#define sp $r3 /* stack pointer */
#define v0 $r4 /* return value - caller saved */
#define v1 $r5
#define a0 $r4 /* argument registers */
#define a1 $r5
#define a2 $r6
#define a3 $r7
#define a4 $r8
#define a5 $r9
#define a6 $r10
#define a7 $r11
#define t0 $r12 /* caller saved */
#define t1 $r13
#define t2 $r14
#define t3 $r15
#define t4 $r16
#define t5 $r17
#define t6 $r18
#define t7 $r19
#define t8 $r20
/* $r21: Temporarily reserved */
#define fp $r22 /* frame pointer */
#define s0 $r23 /* callee saved */
#define s1 $r24
#define s2 $r25
#define s3 $r26
#define s4 $r27
#define s5 $r28
#define s6 $r29
#define s7 $r30
#define s8 $r31

// 分配四个 page 来作为 ebase，其中 reset 放在第四个页面
//
// 6.1.1 中断一共 13 个, SWI0 的中断号为 0, SWI1 的中断号为 1
// 6.2.1 中断的 ecode 是 中断号 + 64
//
// (64 + 14) * vec_size
#define CSR_ECFG_VS_SHIFT 16
#define INSTRUCTION_SIZE 4
#define INT_OFFSET 64
#define VEC_SIZE                                                               \
    (1 << (INIT_VALUE_ECFG >> CSR_ECFG_VS_SHIFT)) * INSTRUCTION_SIZE
#define ERREBASE_OFFSET (PAGESIZE * 3)

/*
 * ExStatus.ExcCode
 */
#define EXCCODE_RSV         0   /* Reserved */
#define EXCCODE_TLBL        1   /* TLB miss on a load */
#define EXCCODE_TLBS        2   /* TLB miss on a store */
#define EXCCODE_TLBI        3   /* TLB miss on a ifetch */
#define EXCCODE_TLBM        4   /* TLB modified fault */
#define EXCCODE_TLBRI       5   /* TLB Read-Inhibit exception */
#define EXCCODE_TLBXI       6   /* TLB Execution-Inhibit exception */
#define EXCCODE_TLBPE       7   /* TLB Privilege Error */
#define EXCCODE_ADE         8   /* Address Error */
     #define EXSUBCODE_ADEF 0   /* Fetch Instruction */
     #define EXSUBCODE_ADEM 1   /* Access Memory*/
#define EXCCODE_ALE         9   /* Unalign Access */
#define EXCCODE_OOB         10  /* Out of bounds */
#define EXCCODE_SYS         11  /* System call */
#define EXCCODE_BP          12  /* Breakpoint */
#define EXCCODE_INE         13  /* Inst. Not Exist */
#define EXCCODE_IPE         14  /* Inst. Privileged Error */
#define EXCCODE_FPDIS       15  /* FPU Disabled */
#define EXCCODE_LSXDIS      16  /* LSX Disabled */
#define EXCCODE_LASXDIS     17  /* LASX Disabled */
#define EXCCODE_FPE         18  /* Floating Point Exception */
    #define EXCSUBCODE_FPE  0   /* Floating Point Exception */
    #define EXCSUBCODE_VFPE 1   /* Vector Exception */
#define EXCCODE_WATCH       19  /* Watch address reference */
#define EXCCODE_BTDIS       20  /* Binary Trans. Disabled */
#define EXCCODE_BTE         21  /* Binary Trans. Exception */
#define EXCCODE_PSI         22  /* Guest Privileged Error */
#define EXCCODE_HYP         23  /* Hypercall */
#define EXCCODE_GCM         24  /* Guest CSR modified */
    #define EXCSUBCODE_GCSC 0   /* Software caused */
    #define EXCSUBCODE_GCHC 1   /* Hardware caused */
#define EXCCODE_SE          25  /* Security */

#define CSR_TLBRELO_RPLV_SHIFT 63
#define CSR_TLBRELO_RPLV (_ULCAST_(0x1) << CSR_TLBRELO_RPLV_SHIFT)
#define CSR_TLBRELO_XI_SHIFT 62
#define CSR_TLBRELO_XI (_ULCAST_(0x1) << CSR_TLBRELO_XI_SHIFT)
#define CSR_TLBRELO_RI_SHIFT 61
#define CSR_TLBRELO_RI (_ULCAST_(0x1) << CSR_TLBRELO_RI_SHIFT)
#define CSR_TLBRELO_PFN_SHIFT 12
#define CSR_TLBRELO_PFN_WIDTH 36
#define CSR_TLBRELO_PFN (_ULCAST_(0xfffffffff) << CSR_TLBRELO_PFN_SHIFT)
#define CSR_TLBRELO_GLOBAL_SHIFT 6
#define CSR_TLBRELO_GLOBAL (_ULCAST_(0x0) << CSR_TLBRELO_GLOBAL_SHIFT)
#define CSR_TLBELO_GLOBAL  (_ULCAST_(0x0) << CSR_TLBRELO_GLOBAL_SHIFT)
#define CSR_TLBRELO_CCA_SHIFT 4
#define CSR_TLBRELO_CCA_WIDTH 2
#define CSR_TLBRELO_CCA (_ULCAST_(0x1) << CSR_TLBRELO_CCA_SHIFT)
#define CSR_TLBELO_CCA  (_ULCAST_(0x1) << CSR_TLBRELO_CCA_SHIFT)
#define CSR_TLBRELO_PLV_SHIFT 2
#define CSR_TLBRELO_PLV_WIDTH 2
#define CSR_TLBRELO_PLV (_ULCAST_(0x3) << CSR_TLBRELO_PLV_SHIFT)
#define CSR_TLBRELO_WE_SHIFT 1
#define CSR_TLBRELO_WE (_ULCAST_(0x1) << CSR_TLBRELO_WE_SHIFT)
#define CSR_TLBELO_WE  (_ULCAST_(0x1) << CSR_TLBRELO_WE_SHIFT)
#define CSR_TLBRELO_V_SHIFT 0
#define CSR_TLBRELO_V (_ULCAST_(0x1) << CSR_TLBRELO_V_SHIFT)
#define CSR_TLBELO_V  (_ULCAST_(0x1) << CSR_TLBRELO_V_SHIFT)

// 512M 从 INIT_VALUE_PRCFG2 中获取
#define TLB_PS_4K 0xc
#define TLB_PS_1G 0x1e
#define TLB_PS_1G_shift 30
#define TLB_MASK (1 << TLB_PS - 1)

#define TLBRELO0_STANDARD_BITS                                                 \
    (CSR_TLBRELO_V | CSR_TLBRELO_WE | CSR_TLBRELO_CCA | CSR_TLBRELO_GLOBAL)
#define TLBRELO1_STANDARD_BITS (TLBRELO0_STANDARD_BITS | (1 << TLB_PS_1G_shift))

#define TLBELO_STANDARD_BITS                                                   \
    (CSR_TLBELO_V | CSR_TLBELO_WE | CSR_TLBELO_CCA | CSR_TLBELO_GLOBAL)

// ring 0, disable interrupt, mapping
#define CRMD_PG 4
#define INIT_VALUE_CRMD (1 << CRMD_PG)
#define INIT_VALUE_EUEN 0xf
#define INIT_VALUE_MISC 0x0
// VS 指令间距是 2 ** 7, 屏蔽 IPI ，时钟，性能计数器 和 硬中断
#define INIT_VALUE_ECFG 0x70000
// 无需配置缩减虚拟地址
#define INIT_VALUE_RVACFG 0x0
// 读取的 LLBCTL 总是 0, 猜测是因为多数情况下，LLBit 都不会被其他人清零，所以总是 0
#define INIT_VALUE_LLBCTL 0x1

#define INIT_VALUE_STLBPS 0xc
#define INIT_VALUE_PWCTL0 0x5e56e
#define INIT_VALUE_PWCTL1 0x2e4

// kvm_vz_vcpu_setup 将 timerid 初始化为 `vcpu->vcpu_id`
// 而 ioctl(vcpu->vm->vm_fd, KVM_CREATE_VCPU, vcpu->cpu_id);
#define INIT_VALUE_TIMERCFG 0x0

// CPUCFG 知道一共存在 4 个，从内核的定义来看，也是如此的
#define INIT_VALUE_PERF0_EVENT 0x0
#define INIT_VALUE_PERF1_EVENT 0x0
#define INIT_VALUE_PERF2_EVENT 0x0
#define INIT_VALUE_PERF3_EVENT 0x0

#define INIT_VALUE_DEBUG 0x0

#define INVALID_CODEFLOW_1 .word(0x002b8000 | (0x1))
#define INVALID_CODEFLOW_2 .word(0x002b8000 | (0x2))
#define INVALID_CODEFLOW_3 .word(0x002b8000 | (0x3))
#define INVALID_EBASE_POSITION 0x13
// TODO check kvm_loongarch_emul_hypcall
// how EMULATE_DEBUG works ?
#define HYPERCALL        .word(0x002b8000)
#define UNEXPECTED_TLBL  .word(0x002b8001)
#define UNEXPECTED_TLBS  .word(0x002b8002)
#define UNEXPECTED_TLBI  .word(0x002b8003)
#define UNEXPECTED_TLBM  .word(0x002b8004)
#define UNEXPECTED_TLBRI .word(0x002b800f)
#define UNEXPECTED_TLBXI .word(0x002b8006)
#define UNEXPECTED_TLBPE .word(0x002b8007)
#define UNEXPECTED_ADE   .word(0x002b8008)
#define UNEXPECTED_ALE   .word(0x002b8009)
#define UNEXPECTED_TRAP  .word(0x002b800a)
#define UNEXPECTED_INE   .word(0x002b800d)

// copied arch/loongarch/include/asm/asm.h
/*
 * LEAF - declare leaf routine
 */
#define ENTRY(symbol)                                                          \
    .globl symbol;                                                         \
    .align 2;                                                              \
    .type symbol, @function;                                               \
    symbol:

/*
 * END - mark end of function
 */
#define END(function)                   \
        .size   function, .-function

#define VCPU_FCSR0 0
#define VCPU_VCSR 4
#define VCPU_FCC 8
#define VCPU_FPR0 16
#define VCPU_FPR_LEN 32

#define ERA_INDEX       32
#define BADV_INDEX      33
#define TLBR_EHI_INDEX  40

#define COUNTER_UFAST_HAMT   100
#define COUNTER_UFAST_QEMU   101

#define TRAP_CODE_INDEX 506
#define ASID_INDEX      507
#define TLBELO1_INDEX   508
#define TLBELO0_INDEX   509
#define TLBEHI_INDEX    510
#define TLBIDX_INDEX    511

#define MAPPING_BASE_ADDRESS 0x0
#define DATA_STORAGE_ADDRESS 0x8100000000
#define CODE_STORAGE_ADDRESS 0x7FFFFFC000
#define FASTCODE_STORAGE_ADDRESS 0x7FFFFF8000

#define DATA_STORAGE_ADDRESS_S0 0x8100000000
#define DATA_STORAGE_ADDRESS_S1 0x8200000000
#define DATA_STORAGE_ADDRESS_S2 0x8300000000
#define DATA_STORAGE_ADDRESS_S3 0x8400000000

#define CODE_STORAGE_ADDRESS_S0 0x7FFFFFC000 /* -16384 */
#define CODE_STORAGE_ADDRESS_S1 0x7FFFFF8000 /* -32768 */
#define CODE_STORAGE_ADDRESS_S2 0x7FFFFF4000 /* -49152 */
#define CODE_STORAGE_ADDRESS_S3 0x7FFFFF0000 /* -65536 */

#endif /* end of include guard: INTERNAL_H_6IUWCEFP */
