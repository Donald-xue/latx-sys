#ifndef __HAMT_MISC_H
#define __HAMT_MISC_H

#include <stdlib.h>
#include <stdint.h>
#include <larchintrin.h>
#include "internal.h"
#include "qemu/queue.h"

#define DIRECT_MAPPING(x) (x + 0x9800000000000000)
#define REVDIRECT_MAPPING(x) (x - 0x9800000000000000)

enum invtlb_ops {
    /* invalidate all tlb*/
    INVTLB_ALL = 0x0,
    /* invalidate current tlb */
    INVTLB_CURRENT_ALL = 0x1,
    /* invalidate all global=1 entries in current tlb */
    INVTLB_CURRENT_GTRUE = 0x2,
    /* invalidate all global=0 entries in current tlb */
    INVTLB_CURRENT_GFALSE = 0x3,
    /* invalidate global=0 and matched asid lines in current tlb */
    INVTLB_GFALSE_AND_ASID = 0x4,
};

#define PROT_RWX (PROT_READ | PROT_WRITE | PROT_EXEC)

#define TARGET_ADDR_SIZE (1UL << 32)

#define LB_MASK 0xffffffffffffff00
#define LH_MASK 0xffffffffffff0000
#define LW_MASK 0xffffffff00000000

#define MAX_PTE_PAGES 512
#define PTE_PAGE_SIZE 4096

#define MAX_ASID 1024

#define ASID_VERSION_MASK 0xffffffffff00
#define ASID_VALUE_MASK 0xff
#define GET_ASID_VERSION(asid) ((asid & ASID_VERSION_MASK) >> 8)
#define GET_ASID_VALUE(asid) (asid & ASID_VALUE_MASK)
#define FORM_NEW_ASID(version, value) \
            ((version << 8) | (value & ASID_VALUE_MASK))

struct pte_page {
    QLIST_ENTRY(pte_page) entry;
    uint64_t page;
};

struct pgtable_head {

    QLIST_ENTRY(pgtable_head) entry;

    uint64_t cr3_value;

    uint16_t asid;
};

struct htable_ele {

    QLIST_HEAD(, pgtable_head) pgtables_list;

    int pgtable_num;
};

/*
 * memory access type:
 *     LD.{B[U]/H[U]/W[U]/D}, ST.{B/H/W/D}
 *     LDX.{B[U]/H[U]/W[U]/D}, STX.{B/H/W/D}
 */
typedef enum {
    OPC_LD_B     = 0b0010100000 << 22,
    OPC_LD_H     = 0b0010100001 << 22,
    OPC_LD_W     = 0b0010100010 << 22,
    OPC_LD_D     = 0b0010100011 << 22,
    OPC_ST_B     = 0b0010100100 << 22,
    OPC_ST_H     = 0b0010100101 << 22,
    OPC_ST_W     = 0b0010100110 << 22,
    OPC_ST_D     = 0b0010100111 << 22,
    OPC_LD_BU    = 0b0010101000 << 22,
    OPC_LD_HU    = 0b0010101001 << 22,
    OPC_LD_WU    = 0b0010101010 << 22,
/*
    OPC_LDX_B    = 0b00111000000000 << 18,
    OPC_LDX_H    = 0b00111000000001 << 18,
    OPC_LDX_W    = 0b00111000000010 << 18,
    OPC_LDX_D    = 0b00111000000011 << 18,
    OPC_STX_B    = 0b00111000000100 << 18,
    OPC_STX_H    = 0b00111000000101 << 18,
    OPC_STX_W    = 0b00111000000110 << 18,
    OPC_STX_D    = 0b00111000000111 << 18,
    OPC_LDX_BU   = 0b00111000001000 << 18,
    OPC_LDX_HU   = 0b00111000001001 << 18,
    OPC_LDX_WU   = 0b00111000001010 << 18,
*/
} ld_st_inst;

#define PS_MASK     0x3f000000
#define PS_SHIFT    24
#define PS_4K       0x0000000c
#define PS_8K       0x0000000d
#define PS_16K      0x0000000e
#define PS_32K      0x0000000f
#define PS_64K      0x00000010
#define PS_128K     0x00000011
#define PS_256K     0x00000012
#define PS_512K     0x00000013
#define PS_1M       0x00000014
#define PS_2M       0x00000015
#define PS_4M       0x00000016
#define PS_8M       0x00000017
#define PS_16M      0x00000018
#define PS_32M      0x00000019
#define PS_64M      0x0000001a
#define PS_256M     0x0000001b
#define PS_1G       0x0000001c

void enable_x86vm_hamt(int mode, int cpuid);

#define read_csr_asid()        __dcsrrd(LOONGARCH_CSR_ASID)
#define write_csr_asid(val)    __dcsrwr(val, LOONGARCH_CSR_ASID)
#define read_csr_tlbehi()      __dcsrrd(LOONGARCH_CSR_TLBEHI)
#define write_csr_tlbehi(val)  __dcsrwr(val, LOONGARCH_CSR_TLBEHI)
#define read_csr_tlbidx()       __dcsrrd(LOONGARCH_CSR_TLBIDX)
#define write_csr_tlbidx(val)   __dcsrwr(val, LOONGARCH_CSR_TLBIDX)
#define read_csr_tlbelo0()     __dcsrrd(LOONGARCH_CSR_TLBELO0)
#define write_csr_tlbelo0(val) __dcsrwr(val, LOONGARCH_CSR_TLBELO0)
#define read_csr_tlbelo1()     __dcsrrd(LOONGARCH_CSR_TLBELO1)
#define write_csr_tlbelo1(val) __dcsrwr(val, LOONGARCH_CSR_TLBELO1)

#define read_csr_tlbr_ehi()      __dcsrrd(LOONGARCH_CSR_TLBREHI)
#define write_csr_tlbr_ehi(val)  __dcsrwr(val, LOONGARCH_CSR_TLBREHI)
#define read_csr_tlbr_elo0()     __dcsrrd(LOONGARCH_CSR_TLBRELO0)
#define write_csr_tlbr_elo0(val) __dcsrwr(val, LOONGARCH_CSR_TLBRELO0)
#define read_csr_tlbr_elo1()     __dcsrrd(LOONGARCH_CSR_TLBRELO1)
#define write_csr_tlbr_elo1(val) __dcsrwr(val, LOONGARCH_CSR_TLBRELO1)

/*
 * parse_r var, r - Helper assembler macro for parsing register names.
 *
 * This converts the register name in $n form provided in \r to the
 * corresponding register number, which is assigned to the variable \var. It is
 * needed to allow explicit encoding of instructions in inline assembly where
 * registers are chosen by the compiler in $n form, allowing us to avoid using
 * fixed register numbers.
 *
 * It also allows newer instructions (not implemented by the assembler) to be
 * transparently implemented using assembler macros, instead of needing separate
 * cases depending on toolchain support.
 *
 * Simple usage example:
 * __asm__ __volatile__("parse_r __rt, %0\n\t"
 *            "# di    %0\n\t"
 *            ".word   (0x41606000 | (__rt << 16))"
 *            : "=r" (status);
 */

/* Match an individual register number and assign to \var */
#define _IFC_REG(n)                 \
    ".ifc    \\r, $r" #n "\n\t"     \
    "\\var    = " #n "\n\t"         \
    ".endif\n\t"

__asm__(".macro    parse_r var r\n\t"
    "\\var    = -1\n\t"
    _IFC_REG(0)  _IFC_REG(1)  _IFC_REG(2)  _IFC_REG(3)
    _IFC_REG(4)  _IFC_REG(5)  _IFC_REG(6)  _IFC_REG(7)
    _IFC_REG(8)  _IFC_REG(9)  _IFC_REG(10) _IFC_REG(11)
    _IFC_REG(12) _IFC_REG(13) _IFC_REG(14) _IFC_REG(15)
    _IFC_REG(16) _IFC_REG(17) _IFC_REG(18) _IFC_REG(19)
    _IFC_REG(20) _IFC_REG(21) _IFC_REG(22) _IFC_REG(23)
    _IFC_REG(24) _IFC_REG(25) _IFC_REG(26) _IFC_REG(27)
    _IFC_REG(28) _IFC_REG(29) _IFC_REG(30) _IFC_REG(31)
    ".iflt    \\var\n\t"
    ".error    \"Unable to parse register name \\r\"\n\t"
    ".endif\n\t"
    ".endm");

#undef _IFC_REG

#endif
