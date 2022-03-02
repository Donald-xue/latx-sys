#include "internal.h"
; .global err_entry_begin
; .global err_entry_end
;
; err_entry_begin:
; INVALID_CODEFLOW_1
; err_entry_end:

.global tlb_refill_entry_begin
.global tlb_refill_entry_end
tlb_refill_entry_begin:
csrwr    t0, LOONGARCH_CSR_TLBRSAVE
csrwr    t1, LOONGARCH_CSR_KS0
csrwr    t2, LOONGARCH_CSR_KS1

// t0: bad vaddr t1: 0x1000000000
csrrd    t0, LOONGARCH_CSR_TLBRBADV
or       t1, zero, zero
lu32i.d  t1, 0x100000000>>32
bltu     t0, t1, hamt_space 

qemu_space:
csrrd t0, LOONGARCH_CSR_TLBREHI
// 因为 Loongarch 的 TLB 采用奇偶页, 所以从 EntryHi 的获取掩码为 1G 而不是 512M

// page fault 的 1G 对齐地址
srli.d t0, t0, (TLB_PS_1G_shift + 1)
slli.d t0, t0, (TLB_PS_1G_shift + 1)

// 整个计算过程中， t0 的数值始终保持不变
// 使用 t1 来对应的掩码
ori   t1, zero, TLB_PS_1G
add.d t1, t1, t0
csrwr t1, LOONGARCH_CSR_TLBREHI

csrrd t1, LOONGARCH_CSR_KS6
add.d t1, t1, t0
csrwr t1, LOONGARCH_CSR_TLBRELO0

csrrd t1, LOONGARCH_CSR_KS7
add.d t1, t1, t0
csrwr t1, LOONGARCH_CSR_TLBRELO1
tlbfill

b tlbr_restore

hamt_space:
csrrd  t0, LOONGARCH_CSR_TLBREHI

// set TLBREHI.PS
srli.d t0, t0, 0x6
slli.d t0, t0, 0x6
ori    t1, t0, TLB_PS_4K

csrwr  t1, LOONGARCH_CSR_TLBREHI
csrwr  zero, LOONGARCH_CSR_TLBRELO0
csrwr  zero, LOONGARCH_CSR_TLBRELO1
tlbfill

tlbr_restore:
csrrd    t0, LOONGARCH_CSR_TLBRSAVE
csrrd    t1, LOONGARCH_CSR_KS0
csrrd    t2, LOONGARCH_CSR_KS1

ertn

tlb_refill_entry_end:

// EXCCODE_TLBL 1
.global tlb_load_entry_begin
.global tlb_load_entry_end
tlb_load_entry_begin:
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000008100000000
or      t0, zero, zero
lu32i.d t0, 0x8100000000>>32
lu52i.d t0, t0, 0x9000000000000000>>52

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (BADV_INDEX*8)
csrrd   t1, LOONGARCH_CSR_EPC
st.d    t1, t0, (ERA_INDEX*8)

lu12i.w t1, -16384>>12
lu32i.d t1, 0x7f00000000>>32
csrwr   t1, LOONGARCH_CSR_EPC

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0
ertn
tlb_load_entry_end:

// EXCCODE_TLBS 2
.global tlb_store_entry_begin
.global tlb_store_entry_end
tlb_store_entry_begin:
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000008100000000
or      t0, zero, zero
lu32i.d t0, 0x8100000000>>32
lu52i.d t0, t0, 0x9000000000000000>>52

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (BADV_INDEX*8)
csrrd   t1, LOONGARCH_CSR_EPC
st.d    t1, t0, (ERA_INDEX*8)

lu12i.w t1, -16384>>12
lu32i.d t1, 0x7f00000000>>32
csrwr   t1, LOONGARCH_CSR_EPC

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0
ertn
tlb_store_entry_end:

// EXCCODE_TLBI 3
.global tlb_ifetch_entry_begin
.global tlb_ifetch_entry_end
tlb_ifetch_entry_begin:
UNEXPECTED_TLBI
tlb_ifetch_entry_end:

// EXCCODE_TLBM 4
.global tlb_modified_entry_begin
.global tlb_modified_entry_end
tlb_modified_entry_begin:
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1

// make t0 0x9000008100000000
or      t0, zero, zero
lu32i.d t0, 0x8100000000>>32
lu52i.d t0, t0, 0x9000000000000000>>52

csrrd   t1, LOONGARCH_CSR_BADV
st.d    t1, t0, (BADV_INDEX*8)
csrrd   t1, LOONGARCH_CSR_EPC
st.d    t1, t0, (ERA_INDEX*8)

lu12i.w t1, -16384>>12
lu32i.d t1, 0x7f00000000>>32
csrwr   t1, LOONGARCH_CSR_EPC

csrrd   t1, LOONGARCH_CSR_KS1
csrrd   t0, LOONGARCH_CSR_KS0

ertn
tlb_modified_entry_end:

// EXCCODE_TLBRI 5
.global tlb_read_inhibit_entry_begin
.global tlb_read_inhibit_entry_end
tlb_read_inhibit_entry_begin:
UNEXPECTED_TLBRI
tlb_read_inhibit_entry_end:

// EXCCODE_TLBXI 6
.global tlb_exe_inhibit_entry_begin
.global tlb_exe_inhibit_entry_end
tlb_exe_inhibit_entry_begin:
UNEXPECTED_TLBXI
tlb_exe_inhibit_entry_end:

// EXCCODE_TLBPE 7
.global tlb_privilege_err_entry_begin
.global tlb_privilege_err_entry_end
tlb_privilege_err_entry_begin:
UNEXPECTED_TLBPE
tlb_privilege_err_entry_end:

// EXCCODE_ADE 8
.global addr_err_entry_begin
.global addr_err_entry_end
addr_err_entry_begin:
UNEXPECTED_ADE
addr_err_entry_end:

// EXCCODE_ALE 9
.global unaligned_access_entry_begin
.global unaligned_access_entry_end
unaligned_access_entry_begin:
UNEXPECTED_ALE
unaligned_access_entry_end:

// EXCCODE_BP 12
.global break_entry_begin
.global break_entry_end
break_entry_begin:
csrwr   t0, LOONGARCH_CSR_KS0
csrwr   t1, LOONGARCH_CSR_KS1
csrwr   t2, LOONGARCH_CSR_KS2

or      t0, zero, zero
lu32i.d t0, 0x8100000000>>32
lu52i.d t0, t0, 0x9000000000000000>>52

ldptr.d t1, t0, (TRAP_CODE_INDEX*8)

move    t2, t1
xori    t2, t2, 0x3
beq     t2, zero, restore_regs

UNEXPECTED_TRAP

restore_regs:
ld.d    ra, t0, 8*1
ld.d    tp, t0, 8*2
ld.d    sp, t0, 8*3
ld.d    a0, t0, 8*4
ld.d    a1, t0, 8*5
ld.d    a2, t0, 8*6
ld.d    a3, t0, 8*7
ld.d    a4, t0, 8*8
ld.d    a5, t0, 8*9
ld.d    a6, t0, 8*10
ld.d    a7, t0, 8*11
ld.d    t1, t0, 8*13
ld.d    t2, t0, 8*14
ld.d    t3, t0, 8*15
ld.d    t4, t0, 8*16
ld.d    t5, t0, 8*17
ld.d    t6, t0, 8*18
ld.d    t7, t0, 8*19
ld.d    t8, t0, 8*20
ld.d  $r21, t0, 8*21
ld.d    fp, t0, 8*22
ld.d    s0, t0, 8*23
ld.d    s1, t0, 8*24
ld.d    s2, t0, 8*25
ld.d    s3, t0, 8*26
ld.d    s4, t0, 8*27
ld.d    s5, t0, 8*28
ld.d    s6, t0, 8*29
ld.d    s7, t0, 8*30
ld.d    s8, t0, 8*31

csrwr   t1, LOONGARCH_CSR_KS0
ld.d    t1, t0, 8*32
csrwr   t1, LOONGARCH_CSR_EPC

ld.d    t0, t0, 8*12
csrrd   t1, LOONGARCH_CSR_KS0

ertn
break_entry_end:

/* t0 是 caller saved 寄存器 */
/* Syscall number held in a7 */
.global syscall_entry_begin
.global syscall_entry_end
syscall_entry_begin:
csrrd t0, LOONGARCH_CSR_KS5
st.d a0, t0, 0 
st.d a1, t0, 8 
st.d a2, t0, 16
st.d a3, t0, 24
st.d a4, t0, 32
st.d a5, t0, 40
st.d a6, t0, 48
st.d a7, t0, 56
/*
 * loongarch linux kernel checks hypcall parameter
 * to be no more than 6, thus the need to make $a0 0
 */
xor  a0, a0, a0
HYPERCALL
ld.d v0, t0, 0

csrrd t0, LOONGARCH_CSR_EPC
addi.d t0, t0, 4
csrwr t0, LOONGARCH_CSR_EPC

ertn
syscall_entry_end:

// EXCCODE_INE 13
.global ine_entry_begin
.global ine_entry_end
ine_entry_begin:
UNEXPECTED_INE
ine_entry_end:

//EXCP_FPE 18
.global fpe_entry_begin
.global fpe_entry_end
fpe_entry_begin:
movgr2fcsr fcsr1, zero
ertn
fpe_entry_end:

.global host_loop
.global switch_stack
switch_stack:
	move sp, a1
	b host_loop /* host_loop 绝对不会返回 */

// 从 arch/loongarch/include/asm/stackframe.h 看
// 调用 syscall 之后, ra, sp, fp 和参数都会保存下来

/* copied from /sysdeps/unix/sysv/linux/loongarch/clone.S */
// 参考 emulate_fork_by_two_vcpu 中在 stack 上设置的参数
ENTRY (__do_simulate_clone)
	/* Do the system call.  */
	li.d    a7,220 /* #define __NR_clone 220 */
	syscall	0

	beqz	a0, 1f

	/* Successful return from the parent.  */
  jirl zero, ra, 0

1:
  ld.d t0, sp, 0
  ld.d a0, sp, 8
  ld.d a1, sp, 16
  jirl zero, t0, 0
END (__do_simulate_clone)


ENTRY (get_fpu_regs)
	movfcsr2gr	t0, fcsr0
	st.w t0,	a0, VCPU_FCSR0

	movcf2gr	t0, $fcc0
	or	t1, t0, zero
	movcf2gr	t0, $fcc1
	bstrins.d	t1, t0, 0xf, 0x8
	movcf2gr	t0, $fcc2
	bstrins.d	t1, t0, 0x17, 0x10
	movcf2gr	t0, $fcc3
	bstrins.d	t1, t0, 0x1f, 0x18
	movcf2gr	t0, $fcc4
	bstrins.d	t1, t0, 0x27, 0x20
	movcf2gr	t0, $fcc5
	bstrins.d	t1, t0, 0x2f, 0x28
	movcf2gr	t0, $fcc6
	bstrins.d	t1, t0, 0x37, 0x30
	movcf2gr	t0, $fcc7
	bstrins.d	t1, t0, 0x3f, 0x38
	st.d	t1, a0, VCPU_FCC
	movgr2fcsr	fcsr0, zero

	movfcsr2gr	t0, $r16
	st.w	t0, a0, VCPU_VCSR

  __tmp = 12 /* #define t0 $r12 */
	li.d            t0, VCPU_FPR0
	add.d t0,       a0, t0
	.word (0xb3 << 22 | ((0 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 0)
	.word (0xb3 << 22 | ((1 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 1)
	.word (0xb3 << 22 | ((2 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 2)
	.word (0xb3 << 22 | ((3 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 3)
	.word (0xb3 << 22 | ((4 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 4)
	.word (0xb3 << 22 | ((5 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 5)
	.word (0xb3 << 22 | ((6 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 6)
	.word (0xb3 << 22 | ((7 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 7)
	.word (0xb3 << 22 | ((8 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 8)
	.word (0xb3 << 22 | ((9 * VCPU_FPR_LEN) << 10) | __tmp  << 5 | 9)
	.word (0xb3 << 22 | ((10* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 10)
	.word (0xb3 << 22 | ((11* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 11)
	.word (0xb3 << 22 | ((12* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 12)
	.word (0xb3 << 22 | ((13* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 13)
	.word (0xb3 << 22 | ((14* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 14)
	.word (0xb3 << 22 | ((15* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 15)
	.word (0xb3 << 22 | ((16* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 16)
	.word (0xb3 << 22 | ((17* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 17)
	.word (0xb3 << 22 | ((18* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 18)
	.word (0xb3 << 22 | ((19* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 19)
	.word (0xb3 << 22 | ((20* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 20)
	.word (0xb3 << 22 | ((21* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 21)
	.word (0xb3 << 22 | ((22* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 22)
	.word (0xb3 << 22 | ((23* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 23)
	.word (0xb3 << 22 | ((24* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 24)
	.word (0xb3 << 22 | ((25* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 25)
	.word (0xb3 << 22 | ((26* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 26)
	.word (0xb3 << 22 | ((27* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 27)
	.word (0xb3 << 22 | ((28* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 28)
	.word (0xb3 << 22 | ((29* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 29)
	.word (0xb3 << 22 | ((30* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 30)
	.word (0xb3 << 22 | ((31* VCPU_FPR_LEN) << 10) | __tmp  << 5 | 31)

  jirl zero, ra, 0
END (get_fpu_regs)
