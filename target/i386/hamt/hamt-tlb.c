#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include <pthread.h>

#include "qemu/osdep.h"
#include "qemu/compiler.h"
#include "qemu/host-utils.h"
#include "qemu/typedefs.h"
#include "exec/cpu-defs.h"
#include "cpu.h"
#include "exec/cpu-all.h"
#include "exec/exec-all.h"
#include "qemu/xxhash.h"
#include "exec/ram_addr.h"

#include "svm.h"
#include "tcg/helper-tcg.h"

#include "hamt.h"
#include "hamt_misc.h"
#include "internal.h"
#include "info.h"
#include "hamt-tlb.h"

#include "monitor/monitor.h"
#include "monitor/hmp-target.h"
#include "monitor/hmp.h"
#include "qapi/qmp/qdict.h"

#include "latx-options.h"
#include "latx-sigint-fn-sys.h"

#include "latx-counter-sys.h"

static inline void tlb_read(void)
{
    __asm__ __volatile__("tlbrd");
}

static inline void tlb_probe(void)
{
    __asm__ __volatile__("tlbsrch");
}

static bool tlbindex_valid(int32_t index)
{
    return index >= 0;
}

static inline void tlb_write_indexed(void)
{
    __asm__ __volatile__("tlbwr");
}

static inline void tlb_write_random(void)
{
    __asm__ __volatile__("tlbfill");
}

static inline void enable_pg(void)
{
    __asm__ __volatile__(
            "ori $t0, $zero, %0\n\t"
            "csrwr $t0, %1\n\t"
            :
            : "i"(CSR_CRMD_PG), "i"(LOONGARCH_CSR_CRMD)
            : "$t0"
            );
}

static inline void disable_pg(void)
{
    __asm__ __volatile__(
            "ori $t0, $zero, %0\n\t"
            "csrwr $t0, %1\n\t"
            :
            : "i"(CSR_CRMD_DA), "i"(LOONGARCH_CSR_CRMD)
            : "$t0"
            );
}

static inline void set_attr(int r, int w, int x, uint64_t *addr)
{
    if (r) clear_bit(61, addr);
    else set_bit(61, addr);

    if (x) clear_bit(62, addr);
    else set_bit(62, addr);

    if (w) set_bit(1, addr);
    else clear_bit(1, addr);
}


/*
 * invtlb op info addr
 * (0x1 << 26) | (0x24 << 20) | (0x13 << 15) |
 * (addr << 10) | (info << 5) | op
 */
#define STLBSETS 256
static inline void local_flush_tlb_all(void)
{
#if 1
    disable_pg();
    /* invtlb 0x3 : flush guest all TLB that G=0 */
    __asm__ __volatile__(
        ".word ((0x6498000) | (0 << 10) | (0 << 5) | 0x3)\n\t"
        );
    enable_pg();
#endif
#if 0
    int i = 0;
    int32_t csr_tlbidx;

    disable_pg();

    /* flush mtlb */
    csr_tlbidx = read_csr_tlbidx();
    csr_tlbidx &= 0xffff0000;
    csr_tlbidx += 0x800;
    write_csr_tlbidx(csr_tlbidx);
    __asm__ __volatile__("tlbflush");

#if 0
    /* flush stlb */
    for (i = 0; i < STLBSETS; ++i) {
        csr_tlbidx = read_csr_tlbidx();
        csr_tlbidx &= 0xffff0000;
        csr_tlbidx += i;
        write_csr_tlbidx(csr_tlbidx);
        __asm__ __volatile__("tlbflush");
    } 
#endif

    enable_pg();
#endif

}

void hamt_flush_tlb_all(void)
{
    local_flush_tlb_all();
}

static void hamt_set_tlbr(uint64_t vaddr, uint64_t paddr,
        int prot, bool mode)
{
    uint64_t csr_tlbr_ehi = 0;
    uint64_t csr_tlbr_elo0 = 0;
    uint64_t csr_tlbr_elo1 = 0;

    int32_t csr_tlbidx = 0;
    /*uint32_t csr_asid = asid_value; TODO */
    uint32_t csr_asid = 0;

    int w = prot & PAGE_WRITE ? 1 : 0;
    int r = prot & PAGE_READ  ? 1 : 0;
    int x = prot & PAGE_EXEC  ? 1 : 0;

    assert(!hamt_interpreter());

    csr_tlbr_ehi = vaddr & ~0x1fffULL;
    write_csr_tlbr_ehi(csr_tlbr_ehi);
    write_csr_asid(csr_asid);
    tlb_probe(); /* tlbsrch */
    csr_tlbidx = read_csr_tlbidx();
    if (tlbindex_valid(csr_tlbidx)) {
        tlb_read(); /* tlbrd: always update TLBELO0/1 */
        csr_tlbr_elo0 = read_csr_tlbelo0();
        csr_tlbr_elo1 = read_csr_tlbelo1();
    }

    //FIX
    if (csr_tlbidx == 0xc00083f) {
        local_flush_tlb_all();
        csr_tlbidx |= 0x80000000;
    }

    if (vaddr & 0x1000) {
        if (mode) {
            csr_tlbr_elo1 = ((paddr >> 12 << 12) | TLBRELO0_STANDARD_BITS) & (~((uint64_t)0xe000 << 48));
            set_attr(r, w, x, &csr_tlbr_elo1);
        }
        else csr_tlbr_elo1 = 0; 
    } else {
        if (mode) {
            csr_tlbr_elo0 = ((paddr >> 12 << 12) | TLBRELO0_STANDARD_BITS) & (~((uint64_t)0xe000 << 48));
            set_attr(r, w, x, &csr_tlbr_elo0);
        }
        else csr_tlbr_elo0 = 0; 
    }

    csr_tlbidx &= 0xc0ffffff;
    csr_tlbidx |= PS_4K << PS_SHIFT;

    csr_tlbr_ehi &= ~0x1f;
    csr_tlbr_ehi |= PS_4K ;

    write_csr_asid(csr_asid);
    write_csr_tlbr_ehi(csr_tlbr_ehi);
    write_csr_tlbr_elo0(csr_tlbr_elo0);
    write_csr_tlbr_elo1(csr_tlbr_elo1);
    write_csr_tlbidx(csr_tlbidx);

    /* tlbwr : tlbfill */
    tlbindex_valid(csr_tlbidx) ? tlb_write_indexed() : tlb_write_random();
}

/*
 * mode:
 *     true: hamt_set_tlb fills in valid pte entry
 *     false: hamt_set_tlb fills in invalid pte entry
 * trap_code:
 *     0: tlbsearch
 *     1: tlbwr
 *     2: tlbfill
 * TODO:
 *     tlbelo0 / tlbelo1 may not be right
 */
void hamt_set_tlb(uint64_t vaddr, uint64_t paddr, int prot, bool mode)
{
    uint64_t csr_tlbehi, csr_tlbelo0 = 0, csr_tlbelo1 = 0;
    int32_t csr_tlbidx;
    /*uint32_t csr_asid = asid_value; TODO */
    uint32_t csr_asid = 0;

    int w = prot & PAGE_WRITE ? 1 : 0;
    int r = prot & PAGE_READ  ? 1 : 0;
    int x = prot & PAGE_EXEC  ? 1 : 0;

    assert(!hamt_interpreter());

    disable_pg();

    // to see whether there is already a valid adjacent tlb entry
    csr_tlbehi = vaddr & ~0x1fffULL;
    write_csr_tlbehi(csr_tlbehi);
    write_csr_asid(csr_asid);
    tlb_probe();
    csr_tlbidx = read_csr_tlbidx();
    if (csr_tlbidx >= 0) {
        tlb_read();
        csr_tlbelo0 = read_csr_tlbelo0();
        csr_tlbelo1 = read_csr_tlbelo1();
    }

    //FIX
    if (csr_tlbidx == 0xc00083f) {
        local_flush_tlb_all();
        csr_tlbidx |= 0x80000000;
    }

    if (tlbindex_valid(csr_tlbidx)) {

        if (vaddr & 0x1000) {

            if (mode) {
                csr_tlbelo1 = ((paddr >> 12 << 12) | TLBELO_STANDARD_BITS) & (~((uint64_t)0xe000 << 48));
                set_attr(r, w, x, &csr_tlbelo1);
            }
            else csr_tlbelo1 = 0; 

        } else {

            if (mode) {
                csr_tlbelo0 = ((paddr >> 12 << 12) | TLBELO_STANDARD_BITS) & (~((uint64_t)0xe000 << 48));
                set_attr(r, w, x, &csr_tlbelo0);
            }
            else csr_tlbelo0 = 0;

        }

    } else {

        if (vaddr & 0x1000) {

            //csr_tlbelo0 = 0;

            if (mode) {
                csr_tlbelo1 = ((paddr >> 12 << 12) | TLBELO_STANDARD_BITS) & (~((uint64_t)0xe000 << 48));
                set_attr(r, w, x, &csr_tlbelo1);
            }
            else csr_tlbelo1 = 0; 

        } else {

            if (mode) {
                csr_tlbelo0 = ((paddr >> 12 << 12) | TLBELO_STANDARD_BITS) & (~((uint64_t)0xe000 << 48));
                set_attr(r, w, x, &csr_tlbelo0);
            }
            else csr_tlbelo0 = 0;

            //csr_tlbelo1 = 0;

        }

    }

    // set page size 4K
    csr_tlbidx &= 0xc0ffffff;
    csr_tlbidx |= PS_4K << PS_SHIFT;
    write_csr_asid(csr_asid);
    write_csr_tlbehi(csr_tlbehi);
    write_csr_tlbelo0(csr_tlbelo0);
    write_csr_tlbelo1(csr_tlbelo1);
    write_csr_tlbidx(csr_tlbidx);

    tlbindex_valid(csr_tlbidx) ? tlb_write_indexed() : tlb_write_random();

    enable_pg();
}

void __hamt_set_hardware_tlb(uint32_t vaddr, uint64_t paddr,
        int prot, int is_tlbr)
{
    bool mode = prot ? true : false;
    if (is_tlbr) {
        hamt_set_tlbr(vaddr, paddr, prot, mode);
    } else {
        hamt_set_tlb(vaddr, paddr, prot, mode);
    }
}

void hamt_set_hardware_tlb(uint32_t vaddr, uint64_t paddr,
        int prot, int is_tlbr)
{
    if (hamt_enable() && hamt_started()) {
        __hamt_set_hardware_tlb(vaddr, paddr, prot, is_tlbr);
    }
}

void hamt_protect_code(uint64_t guest_pc, int is_page2)
{
    if (!(hamt_enable() && hamt_started())) {
        return;
    }

    uint64_t csr_tlbehi, csr_tlbelo0 = 0, csr_tlbelo1 = 0;
    int32_t csr_tlbidx;
    /*uint32_t csr_asid = asid_value; TODO */
    uint32_t csr_asid = 0;
    int n;

    disable_pg(); /* --------------------------------------------- */

    csr_tlbehi = guest_pc & ~0x1fffULL;
    n = (guest_pc >> 12) & 0x1;
    if (is_page2) {
        guest_pc += 0x1000ULL;
        csr_tlbehi = guest_pc & ~0x1fffULL;
        n = (guest_pc >> 12) & 0x1;
    }

    write_csr_tlbehi(csr_tlbehi);
    write_csr_asid(csr_asid);
    tlb_probe();
    csr_tlbidx = read_csr_tlbidx();
    if (csr_tlbidx >= 0) {
        tlb_read();
        csr_tlbelo0 = read_csr_tlbelo0();
        csr_tlbelo1 = read_csr_tlbelo1();

        if (!n) {
            csr_tlbelo0 &= ~0x2;
        } else {
            csr_tlbelo1 &= ~0x2;
        }

        csr_tlbidx &= 0xc0ffffff;
        csr_tlbidx |= PS_4K << PS_SHIFT;

        write_csr_asid(csr_asid);
        write_csr_tlbehi(csr_tlbehi);
        write_csr_tlbelo0(csr_tlbelo0);
        write_csr_tlbelo1(csr_tlbelo1);
        write_csr_tlbidx(csr_tlbidx);

        tlb_write_indexed();
    }

    enable_pg(); /* --------------------------------------------- */
}

void hamt_unprotect_code(uint64_t guest_pc)
{
    if (!(hamt_enable() && hamt_started())) return;

    uint64_t csr_tlbehi, csr_tlbelo0 = 0, csr_tlbelo1 = 0;
    int32_t csr_tlbidx;
    /*uint32_t csr_asid = asid_value; TODO */
    uint32_t csr_asid = 0;

    disable_pg(); /* --------------------------------------------- */

    csr_tlbehi = guest_pc & ~0x1fffULL;
    int n = (guest_pc >> 12) & 0x1;

    write_csr_tlbehi(csr_tlbehi);
    write_csr_asid(csr_asid);
    tlb_probe();
    csr_tlbidx = read_csr_tlbidx();
    if (csr_tlbidx >= 0) {
        tlb_read();
        csr_tlbelo0 = read_csr_tlbelo0();
        csr_tlbelo1 = read_csr_tlbelo1();

        if (!n) {
            csr_tlbelo0 |= 0x2;
        } else {
            csr_tlbelo1 |= 0x2;
        }

        csr_tlbidx &= 0xc0ffffff;
        csr_tlbidx |= PS_4K << PS_SHIFT;

        write_csr_asid(csr_asid);
        write_csr_tlbehi(csr_tlbehi);
        write_csr_tlbelo0(csr_tlbelo0);
        write_csr_tlbelo1(csr_tlbelo1);
        write_csr_tlbidx(csr_tlbidx);

        tlb_write_indexed();
    }

    enable_pg(); /* --------------------------------------------- */
}
