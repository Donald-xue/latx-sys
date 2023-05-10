/*
 * Hardware assisted memory translation
 */
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

#include "latx-perfmap.h"

#define HAMT_TYPE_BASE          1
#define HAMT_TYPE_INTERPRETER   2
#define HAMT_TYPE_PG_ASID       3
#define HAMT_TYPE_SOFTMMU       4
#define HAMT_TYPE_MASK          0xf

#define HAMT_HAVE_TLBRFAST      0x10
#define HAMT_HAVE_STLB          0x20

int hamt_enable(void)
{
    int hamt = option_hamt & HAMT_TYPE_MASK;
    return hamt >= HAMT_TYPE_BASE;
}

int hamt_base(void)
{
    int hamt = option_hamt & HAMT_TYPE_MASK;
    return hamt == HAMT_TYPE_BASE;
}

int hamt_interpreter(void)
{
    int hamt = option_hamt & HAMT_TYPE_MASK;
    return hamt == HAMT_TYPE_INTERPRETER;
}

int hamt_pg_asid(void)
{
    int hamt = option_hamt & HAMT_TYPE_MASK;
    return hamt == HAMT_TYPE_PG_ASID;
}

int hamt_softmmu(void)
{
    int hamt = option_hamt & HAMT_TYPE_MASK;
    return hamt == HAMT_TYPE_SOFTMMU;
}

int hamt_have_tlbr_fastpath(void)
{
    return option_hamt & HAMT_HAVE_TLBRFAST;
}

int hamt_have_stlb(void)
{
    return option_hamt & HAMT_HAVE_STLB;
}

static int hamt_delay(void)
{
    return option_hamt_delay;
}

#define HAMT_INTERPRETER_DEBUG
#ifdef HAMT_INTERPRETER_DEBUG
#define hamt_interpreter_debug(str, ...) do { \
    if (hamt_interpreter() && debug) { \
        fprintf(stderr, str, __VA_ARGS__); \
    } \
} while (0)
#define VHV_FMT "vaddr=0x%lx haddr=0x%lx val=0x%lx"
#define HHV_FMT "haddr=0x%lx haddr=0x%lx val=0x%lx"
#else
#define hamt_interpreter_debug(str, ...)
#define VHV_FMT
#define HHV_FMT
#endif

pthread_key_t in_hamt;

/*
 * # HAMT uses hardware TLB
 * TLB Invalid Exception -> trampoline
 * -> hamt_exception_handler()
 * -> hamt_process_addr_mapping()
 * -> hamt_store/load_helper()
 * -> hamt_set_tlb()
 *
 * # HAMT interpreter mode : not unalign access
 * TLB Invalid Exception -> trampoline
 * -> hamt_exception_handler() # is_unalign_2 = 0
 * -> hamt_process_addr_mapping()
 * -> hamt_store/load_helper()
 * -> interpreter load/store # if is not unalign access
 *
 * # HAMT interpreter mode : unalign access
 * TLB Invalid Exception -> trampoline
 * -> hamt_exception_handler()    # is_unalign_2 = 0
 * -> hamt_process_addr_mapping() # walk the first page
 * -> hamt_store/load_helper()    # find out it is unaligned
 * -> hamt_exception_handler()    # is_unalign_2 = 1
 * -> hamt_process_addr_mapping() # walk the second page
 * -> hamt_store/load_helper()
 * -> interpreter unaligned load/store
 */
void hamt_exception_handler(uint64_t x86_vaddr, CPUX86State *env, uint32_t *epc,
        int is_unalign_2, CPUTLBEntry *unalign_entry1, int unalign_size,
        int is_unalign_clean_ram);


/*
 * data_storage layout:
 * ---0KB---
 * gpr[32]
 * ERA       - 32
 * BADV      - 33
 * ...
 * trap code - 506
 * ASID      - 507
 * TLBELO1   - 508
 * TLBELO0   - 509
 * TLBEHI    - 510
 * TLBIDX    - 511
 * ---4KB---
 */

uint64_t data_storage_s[4];

uint64_t code_storage_s[4];
uint64_t fastcode_storage_s[4];
uint64_t debug_code_storage_s[4];

uint64_t mapping_base_address = 0x0;

uint64_t asid_version = 0;
uint16_t asid_value = 0;

static uint64_t asid_map[16];

static bool need_flush = false;

/*
 * hamt_cr3_htable holds all page table inside VM
 */
static struct htable_ele hamt_cr3_htable[MAX_ASID];

static uint64_t flush_tlb_all_count = 0;
static uint64_t new_round_asid = 0;
static uint64_t delete_some_pgtable = 0;
static uint64_t write_2111 = 0;
static uint64_t other_source = 0;
uint64_t from_tlb_flush = 0;
uint64_t from_tlb_flush_page_locked = 0;
uint64_t from_by_mmuidx = 0;
static uint64_t tlb_lsm = 0;
static uint64_t page_fault = 0;
static int highest_guest_tlb = 0;
static int highest_qemu_tlb = 0;
static int highest_x86_tlb = 0;
void hmp_hamt(Monitor *mon, const QDict *qdict)
{
    if (!hamt_enable()) {
        monitor_printf(mon, "hamt is not enabled\n");
        return;
    }

    monitor_printf(mon, "hamt statistics\n");
    monitor_printf(mon, "delete a pgtable : %ld\n", delete_some_pgtable);
    monitor_printf(mon, "flush tlb all: %ld\n", flush_tlb_all_count);
    monitor_printf(mon, "--new round of asid: %ld\n", new_round_asid);
    monitor_printf(mon, "--write 2111st mtlb: %ld\n", write_2111);
    monitor_printf(mon, "--other source     : %ld\n", other_source);
    monitor_printf(mon, "----tlb flush            : %ld\n", from_tlb_flush);
    monitor_printf(mon, "----tlb flush page locked: %ld\n", from_tlb_flush_page_locked);
    monitor_printf(mon, "----tlb flush by mmuidx  : %ld\n", from_by_mmuidx);
    monitor_printf(mon, "asid version : %ld\n", asid_version);
    monitor_printf(mon, "tlb load/store/modify: %ld\n", tlb_lsm);
    monitor_printf(mon, "page fault: %ld\n", page_fault);
    monitor_printf(mon, "highest guest tlb: %d\n", highest_guest_tlb);
    monitor_printf(mon, "highest qemu tlb: %d\n", highest_qemu_tlb);
    monitor_printf(mon, "highest x86 tlb: %d\n", highest_x86_tlb);
}


void hamt_alloc_target_addr_space(void)
{
    mapping_base_address =
        //(uint64_t)mmap((void *)(MAPPING_BASE_ADDRESS+0x10000), TARGET_ADDR_SIZE, PROT_RWX, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        (uint64_t)mmap((void *)(MAPPING_BASE_ADDRESS), TARGET_ADDR_SIZE, PROT_RWX,
               MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (mapping_base_address == (uint64_t)MAP_FAILED)
        die("CANNOT ALLOC_TARGET_SPACE\n");

    printf("mapping base address 0x%lx\n", mapping_base_address);
//    assert(mapping_base_address == 0x10000);
    assert(mapping_base_address == 0x0);
    
//    mapping_base_address -= 0x10000;
}

static uint8_t cr3_hash_2_index(uint64_t cr3)
{
    return qemu_xxhash2(cr3) % MAX_ASID;
}

/*
 * check_cr3_in_htable is for two purposes:
 * 1. check whether cr3 corresponding page table is
 *    in hamt_cr3_htable
 * 2. get cr3 corresponding page table head
 */
static struct pgtable_head *check_cr3_in_htable(uint64_t cr3)
{
    uint32_t index = cr3_hash_2_index(cr3);

    if (hamt_cr3_htable[index].pgtable_num == 0){
        return NULL;
    }

    struct pgtable_head *var;

    QLIST_FOREACH(var, &(hamt_cr3_htable[index].pgtables_list), entry) {
        if (var->cr3_value == cr3) {
            return var;
        }
    }

    return NULL;
}

static bool check_asid_value_used(uint16_t asid)
{
    bool res = test_bit(asid, (unsigned long *)asid_map);

    return res;
}

static uint16_t find_first_unused_asid_value(void)
{
    uint64_t first = find_first_zero_bit((unsigned long *)asid_map, MAX_ASID);

    if (first > MAX_ASID) {
        die("find_first_unused_asid_value return invalid value: %llx", first);
    }

    return (uint16_t)first;
}

static inline void local_flush_tlb_asid(uint16_t asid)
{
    uint32_t op = INVTLB_GFALSE_AND_ASID;

    __asm__ __volatile__(
        "parse_r asid, %1\n\t"
        ".word ((0x6498000) | (0 << 10) | (asid << 5) | %0)\n\t"
        :
        : "i"(op), "r"(asid)
        :
        );
}

static uint16_t allocate_new_asid_value(void)
{
    uint16_t new_asid_value = find_first_unused_asid_value();

    if (new_asid_value == MAX_ASID) {
        /*
         * 1. flush all tlb
         * 2. asid_version++
         * 3. clear asid_map
         * 4. allocate new asid value
         * 5. set allocated bit to 1
         */

        hamt_flush_tlb_all();
        new_round_asid++;
        asid_version++;
        memset(asid_map, 0, sizeof(uint64_t) * 16);
        new_asid_value = find_first_unused_asid_value();
        assert(new_asid_value == 0);
        set_bit(new_asid_value, (unsigned long *)asid_map);

    } else {

        set_bit(new_asid_value, (unsigned long *)asid_map);

    }

    return new_asid_value;
}

static struct pgtable_head *insert_new_pgtable_node(uint64_t new_cr3, uint16_t new_asid_value)
{
    int index = cr3_hash_2_index(new_cr3);

    hamt_cr3_htable[index].pgtable_num++;

    struct pgtable_head *new_pgtable_head =
        (struct pgtable_head *)malloc(sizeof(struct pgtable_head));

    if(new_pgtable_head == NULL) {
        die("new_pgtable_head is NULL");
    }

    QLIST_INSERT_HEAD(&(hamt_cr3_htable[index].pgtables_list),
        new_pgtable_head, entry);

    new_pgtable_head->asid = FORM_NEW_ASID(asid_version, new_asid_value);

    new_pgtable_head->cr3_value = new_cr3;

    return new_pgtable_head;
}

static uint64_t request_old_cr3;
static bool request_del_pgtable;
void hamt_need_flush(uint64_t old_cr3, bool del_pgtable)
{
    if (hamt_interpreter()) {
        return;
    }

    need_flush = true;
    request_old_cr3 = old_cr3;
    if (!request_del_pgtable)
        request_del_pgtable = del_pgtable;
}

void hamt_set_context(uint64_t new_cr3)
{
    if (hamt_interpreter()) {
        return;
    }

    if (hamt_base()) {
        if (hamt_enable() && hamt_started()) {
            hamt_flush_tlb_all();
            return;
        } else {
            die("hamt flush all when not in hamt");
        }
    }

    /*
     * already in hamt_cr3_htable?
     * |_ yes -> same version?
     * |         |_ yes -> set asid_value and return
     * |         |_ no  -> asid has been used?
     * |                   |_ yes -> allocate a new asid
     * |                   |_ no -> use old asid, change asid_version
     * |
     * |_ no  -> allocate a new asid
     */

    struct pgtable_head *dest_pgtable_head = check_cr3_in_htable(new_cr3);
    if (dest_pgtable_head != NULL) {

        if (GET_ASID_VERSION(dest_pgtable_head->asid) != asid_version) {

            if (check_asid_value_used(GET_ASID_VALUE(dest_pgtable_head->asid))) {

                uint16_t new_asid_value = allocate_new_asid_value();

                dest_pgtable_head->asid = FORM_NEW_ASID(asid_version, new_asid_value);

            } else {

                dest_pgtable_head->asid =
                    FORM_NEW_ASID(asid_version, GET_ASID_VALUE(dest_pgtable_head->asid));

            }
        }

    } else {

        uint16_t new_asid_value = allocate_new_asid_value();

        dest_pgtable_head = insert_new_pgtable_node(new_cr3, new_asid_value);

    }

    asid_value = GET_ASID_VALUE(dest_pgtable_head->asid);

    write_csr_asid(asid_value & 0x3ff);

    if (need_flush) {
        if (request_del_pgtable) {
            delete_pgtable(request_old_cr3);
        } else {
            struct pgtable_head *old_pgtable_head = check_cr3_in_htable(request_old_cr3);
            if (old_pgtable_head == NULL) die("old_pgtable_head is NULL");
            local_flush_tlb_asid(old_pgtable_head->asid);
        }
        need_flush = false;
        request_del_pgtable = false;
    }
}

void delete_pgtable(uint64_t cr3)
{
    /*
     * 1. delete cr3 corresponding page table
     * TODO:
     * 2. invalidate all tlb belonging to this process
     *    currently I simply flush all tlb
     */

    delete_some_pgtable++;

    struct pgtable_head *dest_pghead = check_cr3_in_htable(cr3);

    local_flush_tlb_asid(dest_pghead->asid);

    QLIST_REMOVE(dest_pghead, entry);

    --(hamt_cr3_htable[cr3_hash_2_index(cr3)].pgtable_num);

    free(dest_pghead);
}

static inline int is_write_inst(uint32_t *epc)
{
    uint32_t inst = *epc;

    uint32_t opc = (inst >> 22) << 22;

    switch(opc) {
        case OPC_LD_B:
        case OPC_LD_H:
        case OPC_LD_W:
        case OPC_LD_BU:
        case OPC_LD_HU:
        case OPC_LD_WU:
        case OPC_LD_D:
        case OPC_FLD_S:
        case OPC_FLD_D:
        case OPC_VLD:
            return 0;
        case OPC_ST_B:
        case OPC_ST_H:
        case OPC_ST_W:
        case OPC_ST_D:
        case OPC_FST_S:
        case OPC_FST_D:
        case OPC_VST:
            return 1;
        default: {
            pr_info("inst: %x", *epc);
            die("invalid opc in is_write_inst");
        }
    }

    // would and should never reach here
    exit(1);
}

static inline MemOp hamt_get_memop(uint32_t *epc)
{
    uint32_t inst = *epc;
    
    uint32_t opc = (inst>>22) << 22;
    
    switch(opc) {
        case OPC_LD_B:
        case OPC_LD_BU:
        case OPC_ST_B:
            return MO_UB;
        case OPC_LD_H:
        case OPC_LD_HU:
        case OPC_ST_H:
            return MO_LEUW;
        case OPC_LD_W:
        case OPC_LD_WU:
        case OPC_ST_W:
            return MO_LEUL;
        case OPC_LD_D:
        case OPC_ST_D:
            return MO_LEQ;
        default: {
            pr_info("inst: %x",*epc);
            die("invalid opc in hamt_get_memop");
        }
    }

    // would and should never reach here
    exit(1);
}

static inline uint64_t hamt_get_mem_address(uint32_t *epc, int cpuid)
{
    uint32_t inst = *epc;
    uint32_t rj = (inst >> 5) & 0x1f;
    int imm = (inst >> 10) & 0xfff;
    int simm = (imm << 20) >> 20;
    uint64_t base = ((uint64_t*)(data_storage_s[cpuid]))[rj];
    return base + simm;
}

static inline int is_env_fpr(int fp, int is_simd)
{
    /*
     * [ 0- 7] mapping fp registers
     * [ 9-15] temp fp registers
     * [16-23] mapping simd registers
     */
    assert(0 <= fp && fp <= 23);
    return is_simd ? (16 <= fp && fp <= 23) : (0 <= fp && fp <= 7);
}

static inline void __fst_native_9_15(uint64_t *res)
{
    __asm__ __volatile__ (
            "fst.d $ft1, %0, 0\n\r"
            "fst.d $ft2, %0, 8\n\r"
            "fst.d $ft3, %0, 16\n\r"
            "fst.d $ft4, %0, 24\n\r"
            "fst.d $ft5, %0, 32\n\r"
            "fst.d $ft6, %0, 40\n\r"
            "fst.d $ft7, %0, 48\n\r"
            ::"r"(res):"memory");
}
static inline void __vst_native_9_15(uint64_t *res)
{
    __asm__ __volatile__ (
            "vst  $vr9,  %0, 0\n\r"
            "vst  $vr10, %0, 16\n\r"
            "vst  $vr11, %0, 32\n\r"
            "vst  $vr12, %0, 48\n\r"
            "vst  $vr13, %0, 64\n\r"
            "vst  $vr14, %0, 80\n\r"
            "vst  $vr15, %0, 96\n\r"
            ::"r"(res):"memory");
}

static inline void __fld32_native_09(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft1,  %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_10(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft2, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_11(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft3, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_12(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft4, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_13(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft5, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_14(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft6, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_15(uint64_t *src) {
    __asm__ __volatile__ ("fld.s $ft7, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld32_native_9_15(uint64_t val, int fp)
{
    switch (fp) {
    case  9: __fld32_native_09(&val); break;
    case 10: __fld32_native_10(&val); break;
    case 11: __fld32_native_11(&val); break;
    case 12: __fld32_native_12(&val); break;
    case 13: __fld32_native_13(&val); break;
    case 14: __fld32_native_14(&val); break;
    case 15: __fld32_native_15(&val); break;
    default: assert(0); break;
    }
}

static inline void __fld64_native_09(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft1,  %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_10(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft2, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_11(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft3, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_12(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft4, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_13(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft5, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_14(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft6, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_15(uint64_t *src) {
    __asm__ __volatile__ ("fld.d $ft7, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __fld64_native_9_15(uint64_t val, int fp)
{
    switch (fp) {
    case  9: __fld64_native_09(&val); break;
    case 10: __fld64_native_10(&val); break;
    case 11: __fld64_native_11(&val); break;
    case 12: __fld64_native_12(&val); break;
    case 13: __fld64_native_13(&val); break;
    case 14: __fld64_native_14(&val); break;
    case 15: __fld64_native_15(&val); break;
    default: assert(0); break;
    }
}

static inline void __vld128_native_09(void *src) {
    __asm__ __volatile__ ("vld $vr9,  %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_10(void *src) {
    __asm__ __volatile__ ("vld $vr10, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_11(void *src) {
    __asm__ __volatile__ ("vld $vr11, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_12(void *src) {
    __asm__ __volatile__ ("vld $vr12, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_13(void *src) {
    __asm__ __volatile__ ("vld $vr13, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_14(void *src) {
    __asm__ __volatile__ ("vld $vr14, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_15(void *src) {
    __asm__ __volatile__ ("vld $vr15, %0, 0\n\r"::"r"(src):"memory");
}
static inline void __vld128_native_9_15(void *src, int fp)
{
    switch (fp) {
    case  9: __vld128_native_09(src); break;
    case 10: __vld128_native_10(src); break;
    case 11: __vld128_native_11(src); break;
    case 12: __vld128_native_12(src); break;
    case 13: __vld128_native_13(src); break;
    case 14: __vld128_native_14(src); break;
    case 15: __vld128_native_15(src); break;
    default: assert(0); break;
    }
}

static inline uint64_t hamt_get_st_val(uint32_t *epc, int cpuid)
{
    uint32_t inst = *epc;

    uint32_t opc = inst >> 22 << 22;

    uint32_t rd = inst & 0x1f;

    switch(opc) {
        case OPC_ST_B:
            return ((uint64_t*)(data_storage_s[cpuid]))[rd] & (uint64_t)0xff;
        case OPC_ST_H:
            return ((uint64_t*)(data_storage_s[cpuid]))[rd] & (uint64_t)0xffff;
        case OPC_ST_W:
            return ((uint64_t*)(data_storage_s[cpuid]))[rd] & (uint64_t)0xffffffff;
        case OPC_ST_D:
            return ((uint64_t*)(data_storage_s[cpuid]))[rd];
        default: {
            pr_info("inst: %x", *epc);
            die("invalid opc in hamt_get_st_val");
        }
    }

    // would and should never reach here
    exit(1);
}

static uint64_t hamt_get_st_val_softmmu(CPUX86State *env,
        uint32_t *epc, int cpuid)
{
    uint32_t inst = *epc;
    uint32_t opc = inst >> 22 << 22;
    uint32_t rd = inst & 0x1f; /* rd */
    uint64_t native_fp_9_15[(15 - 9 + 1) << 1];

    switch(opc) {
    case OPC_ST_B:
        return ((uint64_t*)(data_storage_s[cpuid]))[rd] & (uint64_t)0xff;
    case OPC_ST_H:
        return ((uint64_t*)(data_storage_s[cpuid]))[rd] & (uint64_t)0xffff;
    case OPC_ST_W:
        return ((uint64_t*)(data_storage_s[cpuid]))[rd] & (uint64_t)0xffffffff;
    case OPC_ST_D:
        return ((uint64_t*)(data_storage_s[cpuid]))[rd];
    case OPC_FST_S:
        if (is_env_fpr(rd, 0)) {
            assert(option_lsfpu);
            assert(rd <= 7);
            rd = (rd + env->fpstt) & 0x7;
            return env->fpregs[rd].d.low & (uint64_t)0xffffffff;
        } else if (is_env_fpr(rd, 1)) {
            assert(16 <= rd && rd <= 23);
            return env->xmm_regs[rd - 16]._l_ZMMReg[0];
        } else {
            assert(9 <= rd && rd <= 15);
            __fst_native_9_15(native_fp_9_15);
            return native_fp_9_15[rd - 9] & (uint64_t)0xffffffff;
        }
    case OPC_FST_D:
        if (is_env_fpr(rd, 0)) {
            assert(option_lsfpu);
            assert(rd <= 7);
            rd = (rd + env->fpstt) & 0x7;
            return env->fpregs[rd].d.low;
        } else if (is_env_fpr(rd, 1)) {
            assert(16 <= rd && rd <= 23);
            return env->xmm_regs[rd - 16]._q_ZMMReg[0];
        } else {
            assert(9 <= rd && rd <= 15);
            __fst_native_9_15(native_fp_9_15);
            return native_fp_9_15[rd - 9];
        }
       break;
    case OPC_VST:
       if (is_env_fpr(rd, 1)) {
           env->temp_xmm._q_ZMMReg[0] = env->xmm_regs[rd - 16]._q_ZMMReg[0];
           env->temp_xmm._q_ZMMReg[1] = env->xmm_regs[rd - 16]._q_ZMMReg[1];
           return 0;
       } else {
           assert(9 <= rd && rd <= 15);
            __vst_native_9_15(native_fp_9_15);
            /*
             * 9  -  0, 1 - ( 9-9)=0
             * 10 -  2, 3 - (10-9)=1
             * 11 -  4, 5 - (11-9)=2
             * 12 -  6, 7 - (12-9)=3
             * 13 -  8, 9 - (13-9)=4
             * 14 - 10,11 - (14-9)=5
             * 15 - 12,13 - (15-9)=6
             */
            env->temp_xmm._q_ZMMReg[0] = native_fp_9_15[((rd - 9) << 1) + 0];
            env->temp_xmm._q_ZMMReg[1] = native_fp_9_15[((rd - 9) << 1) + 1];
            return 0;
       }
       break;
    default:
       pr_info("inst: %x", *epc);
       die("invalid opc in hamt_get_st_val_softmmu");
       break;
    }

    die("can not reach here in hamt_get_st_val_softmmu");
    return 0;
}

void hamt_invlpg_helper(uint32_t i386_addr)
{
    if (hamt_interpreter()) {
        return;
    }

    uint64_t hamt_vaddr = i386_addr + mapping_base_address;
    hamt_set_tlb((current_cpu->env_ptr), hamt_vaddr, 0x0, 0x0, false);
}

static uint32_t get_opc_from_epc(uint32_t *epc)
{
    uint32_t inst = *epc;
    uint32_t opc = (inst >> 22) << 22;
    return opc;
}

static TCGMemOpIdx hamt_get_oi(uint32_t *epc, int mmu_idx)
{
    uint32_t inst = *epc;
    uint32_t opc = (inst >> 22) << 22;

    switch(opc) {
        case OPC_LD_B:
        case OPC_ST_B:
            return make_memop_idx(MO_SB, mmu_idx);
        case OPC_LD_H:
        case OPC_ST_H:
            return make_memop_idx(MO_LESW, mmu_idx);
        case OPC_LD_W:
        case OPC_ST_W:
            return make_memop_idx(MO_LESL, mmu_idx);
        case OPC_LD_D:
        case OPC_ST_D:
        case OPC_FLD_D:
        case OPC_FST_D:
        case OPC_VLD:
            return make_memop_idx(MO_LEQ, mmu_idx);
        case OPC_LD_BU:
            return make_memop_idx(MO_UB, mmu_idx);
        case OPC_LD_HU:
            return make_memop_idx(MO_LEUW, mmu_idx);
        case OPC_LD_WU:
        case OPC_FLD_S:
        case OPC_FST_S:
        case OPC_VST:
            return make_memop_idx(MO_LEUL, mmu_idx);
        default: {
            pr_info("inst: %x", *epc);
            die("invalid opc in hamt_get_oi");
        }
    }

    // would and should never reach here
    exit(1);
}

static void load_into_reg(uint64_t val, uint32_t *epc, int cpuid)
{
    uint32_t inst = *epc;
    int rt = inst & 0x1f;
    uint32_t opc = inst >> 22 << 22;

    switch(opc) {
        case OPC_LD_B: {
            uint64_t temp;
            temp = val & 0xff;
            temp |= (val & 0x80) ? LB_MASK : 0;
            val = temp;
            break;
        }
        case OPC_LD_H: {
            uint64_t temp;
            temp = val & 0xffff;
            temp |= (val & 0x8000) ? LH_MASK : 0;
            val = temp;
            break;
        }
        case OPC_LD_W: {
            uint64_t temp;
            temp = val & 0xffffffff;
            temp |= (val & 0xffffffff80000000) ? LW_MASK : 0;
            val = temp;
            break;
        }
        case OPC_LD_BU: {
            val &= 0xff;
            break;
        }
        case OPC_LD_HU: {
            val &= 0xffff;
            break;
        }
        case OPC_LD_WU: {
            val &= 0xffffffff;
            break;
        }
        case OPC_LD_D: {
            val &= 0xffffffffffffffff;
            break;
        }
        default: {
            pr_info("inst: %x", *epc);
            die("invalid opc in load_into_reg");
        }
    }

    ((uint64_t *)(data_storage_s[cpuid]))[rt] = val;
}

static void load_into_reg_softmmu(CPUX86State *env,
        uint64_t val, uint32_t *epc, int cpuid)
{
    uint32_t inst = *epc;
    int rt = inst & 0x1f; /* fd */
    uint32_t opc = inst >> 22 << 22;

    switch(opc) {
    case OPC_FLD_S:
        if (is_env_fpr(rt, 0)) {
            assert(option_lsfpu);
            assert(0 <= rt && rt <= 7);
            rt = (rt + env->fpstt) & 0x7;
            *((uint32_t*)&env->fpregs[rt].d.low) = val & 0xffffffff;
        } else if (is_env_fpr(rt, 1)) {
            assert(16 <= rt && rt <= 23);
            env->xmm_regs[rt - 16]._l_ZMMReg[0] = val & 0xffffffff;
        } else {
            assert(9 <= rt && rt <= 15);
            __fld32_native_9_15(val, rt);
        }
        return;
    case OPC_FLD_D:
        if (is_env_fpr(rt, 0)) {
            assert(option_lsfpu);
            rt = (rt + env->fpstt) & 0x7;
            *((uint64_t*)&env->fpregs[rt].d.low) = val;
        } else if (is_env_fpr(rt, 1)) {
            assert(16 <= rt && rt <= 23);
            env->xmm_regs[rt - 16]._q_ZMMReg[0] = val;
        } else {
            assert(9 <= rt && rt <= 15);
            __fld64_native_9_15(val, rt);
        }
        return;
    case OPC_VLD:
        if (is_env_fpr(rt, 1)) {
            env->xmm_regs[rt-16]._q_ZMMReg[0] = env->temp_xmm._q_ZMMReg[0];
            env->xmm_regs[rt-16]._q_ZMMReg[1] = env->temp_xmm._q_ZMMReg[1];
        } else {
            assert(9 <= rt && rt <= 15);
            __vld128_native_9_15(&env->temp_xmm, rt);
        }
        return;
    case OPC_LD_B: {
        uint64_t temp;
        temp = val & 0xff;
        temp |= (val & 0x80) ? LB_MASK : 0;
        val = temp;
        break;
    }
    case OPC_LD_H: {
        uint64_t temp;
        temp = val & 0xffff;
        temp |= (val & 0x8000) ? LH_MASK : 0;
        val = temp;
        break;
    }
    case OPC_LD_W: {
        uint64_t temp;
        temp = val & 0xffffffff;
        temp |= (val & 0xffffffff80000000) ? LW_MASK : 0;
        val = temp;
        break;
    }
    case OPC_LD_BU: val &= 0xff;       break;
    case OPC_LD_HU: val &= 0xffff;     break;
    case OPC_LD_WU: val &= 0xffffffff; break;
    case OPC_LD_D:                     break;
    default:
        pr_info("inst: %x", *epc);
        die("invalid opc in load_into_reg_softmmu");
        break;
    }

    ((uint64_t *)(data_storage_s[cpuid]))[rt] = val;
}

//TRY
static void store_into_mem(uint64_t haddr, uint32_t *epc,
        int debug, int is_unalign_2, uint64_t haddr2,
        int cpuid)
{
    uint32_t inst = *epc;
    int rd = inst & 0x1f; 
    uint32_t opc = inst >> 22 << 22;
    
    uint64_t val = ((uint64_t*)data_storage_s[cpuid])[rd];

    if (!is_unalign_2) {
        /* emulate aligned access (inside one page) */
        switch(opc) {
            case OPC_ST_B: {
                *((uint8_t*)haddr) = val & 0xffULL; 
                return;
            }
            case OPC_ST_H: {
                *((uint16_t*)haddr) = val & 0xffffULL; 
                return;
            }
            case OPC_ST_W: {
                *((uint32_t*)haddr) = val & 0xffffffffULL;
                return;
            }
            case OPC_ST_D: {
                *((uint64_t*)haddr) = val;
                return;
            }
            default: {
                pr_info("inst: %x", *epc);
                die("invalid opc in store_into_mem");
            }
        }
    } else {
        /* emulate unaligned access (spans two page) */
        uint8_t *d1 = (void *)haddr;
        uint8_t *d2 = (void *)haddr2;
        uint8_t *d = NULL;
        uint8_t  b = 0;
        int i = 0;
        switch(opc) {
            case OPC_ST_B: {
                pr_info("inst: %x", *epc);
                die("store byte unalign");
                return;
            }
            case OPC_ST_H: {
                hamt_interpreter_debug("Store 16 " HHV_FMT "\n", haddr, haddr2, val);
                *d1 = val & 0xffULL; 
                *d2 = (val & 0xff00ULL) >> 8; 
                return;
            }
            case OPC_ST_W: {
                hamt_interpreter_debug("Store 32 " HHV_FMT "\n", haddr, haddr2, val);
                d = d1;
                b = val & 0xffULL; *d = b;
                
                for (i = 0; i < 3; ++i) {
                    val = val >> 8;
                    d += 1; if (((uint64_t)d & 0xfff) == 0) d = d2;
                    b = val & 0xffULL; *d = b;
                }
                return;
            }
            case OPC_ST_D: {
                hamt_interpreter_debug("Store 64 " HHV_FMT "\n", haddr, haddr2, val);
                d = d1;
                b = val & 0xffULL; *d = b;

                for (i = 0; i < 7; ++i) {
                    val = val >> 8;
                    d += 1; if (((uint64_t)d & 0xfff) == 0) d = d2;
                    b = val & 0xffULL; *d = b;
                }
                return;
            }
            default: {
                pr_info("inst: %x", *epc);
                die("invalid opc in store_into_mem");
            }
        }
    }

    // would and should never reach here
    exit(1);
}

static void hamt_store_helper(CPUArchState *env, target_ulong addr, uint64_t val,
        TCGMemOpIdx oi, uintptr_t retaddr, MemOp op,
        CPUTLBEntry *entry, CPUIOTLBEntry *iotlbentry,
        int prot, uint32_t *epc,
        int is_unalign_2, CPUTLBEntry *unalign_entry1, int unalign_size,
        int is_unalign_clean_ram)
{
    if (hamt_softmmu()) {
        die("%s : hamt softmmu\n", __func__);
    }

    int cpuid = 0;
    if (qemu_tcg_mttcg_enabled()) {
        cpuid = (env_cpu(env))->cpu_index;
    }

    uintptr_t mmu_idx = get_mmuidx(oi);
    target_ulong tlb_addr = tlb_addr_write(entry);
    unsigned a_bits = get_alignment_bits(get_memop(oi));
    uint64_t haddr, haddr2;
    target_ulong addr2;
    size_t size = memop_size(op);

    /* Handle CPU specific unaligned behaviour */
    if (addr & ((1 << a_bits) - 1)) {
        //pr_info("handle cpu specific unaligned behaviour");
    }

    /* Handle anything that isn't just a straight memory access.  */
    if (unlikely(tlb_addr & ~TARGET_PAGE_MASK)) {
        bool need_swap;

        //assert(!is_unalign_2); /* TODO */

        /* For anything that is unaligned, recurse through byte stores.  */
        if ((addr & (size - 1)) != 0) {
            //pr_info("unaligned behaviour");
        }

        /* Handle watchpoints.  */
//        if (unlikely(tlb_addr & TLB_WATCHPOINT)) {
//            /* On watchpoint hit, this will longjmp out.  */
//              pr_info("watchpoint");
//            cpu_check_watchpoint(env_cpu(env), addr, size,
//                                 iotlbentry->attrs, BP_MEM_WRITE, retaddr);
//        }

        need_swap = size > 1 && (tlb_addr & TLB_BSWAP);

        /* Handle I/O access.  */
        if (tlb_addr & TLB_MMIO) {

            ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;

            io_writex(env, iotlbentry, mmu_idx, val, addr, retaddr,
                      op ^ (need_swap * MO_BSWAP));

            return;
        }

        /* Ignore writes to ROM.  */
        if (unlikely(tlb_addr & TLB_DISCARD_WRITE)) {

            ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;

            return;
        }

        //TRY
        haddr = ((uintptr_t)addr + entry->addend);

        /* Handle clean RAM pages.  */
        if (tlb_addr & TLB_NOTDIRTY) {
            notdirty_write(env_cpu(env), addr, size, iotlbentry, retaddr);
            /* if cross-page clean RAM write */
            if (size > 1 &&
                (addr & ~TARGET_PAGE_MASK) + size - 1 >= TARGET_PAGE_SIZE) {
                if (!is_unalign_2) {
                    /*
                     * go to walk the second page with
                     * @is_unalign_2 = 1 and the TLB entry of the first page
                     * @is_unalign_clean_ram = 1
                     */
                    hamt_exception_handler(addr + mapping_base_address, env, epc,
                            1, entry, size, 1);
                } else {
                    assert(is_unalign_clean_ram == 1);
                    ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;
                    haddr = ((uintptr_t)addr + unalign_entry1->addend);
                    addr2 = (addr + size) & TARGET_PAGE_MASK;
                    haddr2 = ((uintptr_t)addr2 + entry->addend);
                    store_into_mem(haddr, epc, 0,
                            is_unalign_2, haddr2, cpuid);
                }
            } else {
                /* NOT a cross-page clean RAM write: write directly */
                ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;
                store_into_mem(haddr, epc, 0, 0, -1, cpuid);
            }
            return;
        }

        if (unlikely(need_swap)) {
            //pr_info("need swap");
        } else {

            if (!hamt_interpreter()) {
                hamt_set_tlb(env, addr + mapping_base_address, haddr, prot, true);
            }
        }

        if (hamt_interpreter()) {
            ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;
            store_into_mem(haddr, epc, 0,
                    0, -1, cpuid); /* not unalign */
        }
        return;
    }

    /* Handle slow unaligned access (it spans two pages or IO).  */
    if (size > 1
        && unlikely((addr & ~TARGET_PAGE_MASK) + size - 1
                     >= TARGET_PAGE_SIZE)) {
        //pr_info("store helper slow unaligned access vaddr: %llx", addr);
        if (hamt_interpreter()) {
            if (!is_unalign_2) {
                //pr_info("store unaligned access vaddr=0x%llx size=%d", addr, size);
                hamt_exception_handler(addr + mapping_base_address, env, epc,
                        1, entry, size, 0);
            } else {
                ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;

                /*
                 *              size
                 *      addr |--------|
                 * ---------------|--------------
                 *       page1          page2
                 * unalign_entry1       entry
                 */
                haddr = ((uintptr_t)addr + unalign_entry1->addend);
                addr2 = (addr + size) & TARGET_PAGE_MASK;
                haddr2 = ((uintptr_t)addr2 + entry->addend);
                //pr_info("store unaligned access vaddr=0x%llx haddr=0x%llx haddr2=0x%llx",
                        //addr, haddr, haddr2);
                store_into_mem(haddr, epc, 0,
                        is_unalign_2, haddr2, cpuid);
            }
            return;
        }
    }

    haddr = ((uintptr_t)addr + entry->addend);

    if (hamt_interpreter()) {
        ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;
        store_into_mem(haddr, epc, 0,
                0, -1, cpuid); /* not unalign access */
    } else {
        hamt_set_tlb(env, addr+mapping_base_address, haddr, prot, true);
    }
}

static void load_direct_into_reg(MemOp op,
        uint64_t vaddr, uint64_t haddr, uint32_t *epc,
        int debug, int is_unalign_2, uint64_t haddr2,
        int cpuid)
{
    int is_sign  = (op & (1<<2));
    size_t size  = op & 0x3;
    uint64_t val = 0;

    uint8_t *d1 = (void *)haddr;
    uint8_t *d2 = (void *)haddr2;
    uint8_t *d = NULL;
    uint8_t  b = 0;
    int i = 0;
    int s = 0;

    switch (size) {
    case 0:
        assert(!is_unalign_2);
        if (is_sign) {
            val = *((int8_t*)haddr);
            hamt_interpreter_debug("Read  8s " VHV_FMT "\n", vaddr, haddr, val);
            assert((val >> 8) == 0 || (int64_t)(val >> 8) == 1);
        } else {
            val = *((uint8_t*)haddr);
            hamt_interpreter_debug("Read  8u " VHV_FMT "\n", vaddr, haddr, val);
            assert((val >> 8) == 0);
        }
        break;
    case 1:
        if (is_unalign_2) {
            s = 0;
            b = *d1; val = val | (b << (8 * s));

            s += 1;
            b = *d2; val = val | (b << (8 * s));
            if (is_sign) {
                val = (int16_t)val;
                assert((val >> 16) == 0 || (int64_t)(val >> 16) == 1);
                hamt_interpreter_debug("Read 16s " VHV_FMT "\n", vaddr, haddr, val);
            } else {
                val = (uint16_t)val;
                assert((val >> 16) == 0);
                hamt_interpreter_debug("Read 16u " VHV_FMT "\n", vaddr, haddr, val);
            }
        } else {
            if (is_sign) {
                val = *((int16_t*)haddr);
                hamt_interpreter_debug("Read 16s " VHV_FMT "\n", vaddr, haddr, val);
                assert((val >> 16) == 0 || (int64_t)(val >> 16) == 1);
            } else {
                val = *((uint16_t*)haddr);
                hamt_interpreter_debug("Read 16u " VHV_FMT "\n", vaddr, haddr, val);
                assert((val >> 16) == 0);
            }
        }
        break;
    case 2:
        if (is_unalign_2) {
            /* 0 */
            d = d1; s = 0;
            b = *d; val = val | (b << (s * 8));
            /* 1,2,3 */
            for (i = 0; i < 3; ++i) {
                s += 1;
                d += 1; if (((uint64_t)d & 0xfff) == 0) d = d2;
                b = *d; val = val | (b << (s * 8));
            }
            /* final value */
            if (is_sign) {
                val = (int32_t)val;
                assert((val >> 32) == 0 || (int64_t)(val >> 32) == 1);
                hamt_interpreter_debug("Read 32s " VHV_FMT "\n", vaddr, haddr, val);
            } else {
                val = (uint32_t)val;
                assert((val >> 32) == 0);
                hamt_interpreter_debug("Read 32u " VHV_FMT "\n", vaddr, haddr, val);
            }
        } else {
            if (is_sign) {
                val = *((int32_t*)haddr);
                hamt_interpreter_debug("Read 32s " VHV_FMT "\n", vaddr, haddr, val);
                assert((val >> 32) == 0 || (int64_t)(val >> 32) == 1);
            } else {
                val = *((uint32_t*)haddr);
                hamt_interpreter_debug("Read 32u " VHV_FMT "\n", vaddr, haddr, val);
                assert((val >> 32) == 0);
            }
        }
        break;
    case 3:
        if (is_unalign_2) {
            /* 0 */
            d = d1; s = 0;
            b = *d; val = val | b;
            /* 1,2,3,4,5,6,7 */
            for (i = 0; i < 7; ++i) {
                s += 1;
                d += 1; if (((uint64_t)d & 0xfff) == 0) d = d2;
                b = *d; val = val | (b << (s * 8));
            }
            /* final value */
            val = (uint64_t)val;
            hamt_interpreter_debug("Read 64 " VHV_FMT "\n", vaddr, haddr, val);
        } else {
            val = *((uint64_t*)haddr);
            hamt_interpreter_debug("Read 64 " VHV_FMT "\n", vaddr, haddr, val);
        }
        break;
    default:
        hamt_interpreter_debug(">>>>> error memop 0x%x\n", op);
        break;
    }

    load_into_reg(val, epc, cpuid);
}

static void hamt_load_helper(CPUArchState *env, target_ulong addr, TCGMemOpIdx oi,
        uintptr_t retaddr, MemOp op, CPUTLBEntry *entry, CPUIOTLBEntry *iotlbentry,
        int prot, uint32_t *epc,
        int is_unalign_2, CPUTLBEntry *unalign_entry1, int unalign_size)
{
    if (hamt_softmmu()) {
        die("%s : hamt softmmu\n", __func__);
    }

    int cpuid = 0;
    if (qemu_tcg_mttcg_enabled()) {
        cpuid = (env_cpu(env))->cpu_index;
    }

    uintptr_t mmu_idx = get_mmuidx(oi);
    uint64_t tlb_addr = entry->addr_read;
    const MMUAccessType access_type = MMU_DATA_LOAD;
    unsigned a_bits = get_alignment_bits(get_memop(oi));
    uint64_t haddr, haddr2;
    target_ulong addr2;
    size_t size = memop_size(op);

    /* Handle CPU specific unaligned behaviour */
    if (addr & ((1 << a_bits) - 1)) {
        //pr_info("handle cpu specific unaligned behaviour");
    }

    /* Handle anything that isn't just a straight memory access.  */
    if (unlikely(tlb_addr & ~TARGET_PAGE_MASK)) {
        bool need_swap;

        assert(!is_unalign_2); /* TODO */

        /* For anything that is unaligned, recurse through full_load.  */
        if ((addr & (size - 1)) != 0) {
            //TODO
            //pr_info("unaligned behaviour");
        }

        /* Handle watchpoints.  */
//        if (unlikely(tlb_addr & TLB_WATCHPOINT)) {
//            /* On watchpoint hit, this will longjmp out.  */
//              pr_info("watchpoint");
//            cpu_check_watchpoint(env_cpu(env), addr, size,
//                                 iotlbentry->attrs, BP_MEM_READ, retaddr);
//        }

        need_swap = size > 1 && (tlb_addr & TLB_BSWAP);

        /* Handle I/O access.  */
        if (likely(tlb_addr & TLB_MMIO)) {

            ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;

            uint64_t retval;

            retval = io_readx(env, iotlbentry, mmu_idx, addr, retaddr,
                            access_type, op ^ (need_swap * MO_BSWAP));

            return load_into_reg(retval, epc, cpuid);
        }

        haddr = (uintptr_t)addr + entry->addend;

        if (unlikely(need_swap)) {
            //pr_info("need swap");
        }

        if (hamt_interpreter()) {
            ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;
            load_direct_into_reg(op, addr, haddr, epc, 0,
                    0, 0, cpuid); /* not unalign access */
        } else {
            hamt_set_tlb(env, addr+mapping_base_address, haddr, prot, true);
        }

        return;
    }

    /* Handle slow unaligned access (it spans two pages or IO).  */
    if (size > 1
        && unlikely((addr & ~TARGET_PAGE_MASK) + size - 1
                    >= TARGET_PAGE_SIZE)) {
        //pr_info("load helper slow unaligned access vaddr: %llx", addr);
        if (hamt_interpreter()) {
            if (!is_unalign_2) {
                //pr_info("load  unaligned access vaddr=0x%llx size=%d", addr, size);
                hamt_exception_handler(addr + mapping_base_address, env, epc,
                        1, entry, size, 0);
            } else {
                ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;

                /*
                 *              size
                 *      addr |--------|
                 * ---------------|--------------
                 *       page1          page2
                 * unalign_entry1       entry
                 */
                haddr = ((uintptr_t)addr + unalign_entry1->addend);
                addr2 = (addr + size) & TARGET_PAGE_MASK;
                haddr2 = ((uintptr_t)addr2 + entry->addend);
                //pr_info("load  unaligned access vaddr=0x%llx haddr=0x%llx haddr2=0x%llx",
                        //addr, haddr, haddr2);
                load_direct_into_reg(op, addr, haddr, epc, 0,
                        is_unalign_2, haddr2, cpuid); /* unalign access */
            }
            return;
        }
    }

    haddr = (uintptr_t)addr + entry->addend;

    if (hamt_interpreter()) {
        ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;
        load_direct_into_reg(op, addr, haddr, epc, 0,
                0, 0, cpuid); /* not unalign access */
    } else {
        hamt_set_tlb(env, addr+mapping_base_address, haddr, prot, true);
    }

    return;
}

static void hamt_process_addr_mapping(CPUState *cpu, uint64_t hamt_badvaddr,
        uint64_t paddr, MemTxAttrs attrs, int prot,
        int mmu_idx, target_ulong size, bool is_write, uint32_t *epc,
        int is_unalign_2, CPUTLBEntry *unalign_entry1, int unalign_size,
        int is_unalign_clean_ram)
{
    if (hamt_softmmu()) {
        die("%s : hamt softmmu\n", __func__);
    }

    int cpuid = 0;
    if (qemu_tcg_mttcg_enabled()) {
        cpuid = cpu->cpu_index;
    }

    CPUArchState *env = cpu->env_ptr;
    MemoryRegionSection *section;
    target_ulong address;
    target_ulong write_address;
    uintptr_t addend;
    CPUTLBEntry tn;
    hwaddr iotlb, xlat, sz, paddr_page;
    target_ulong vaddr_page;
    int asidx = cpu_asidx_from_attrs(cpu, attrs);
    bool is_ram, is_romd;
    CPUIOTLBEntry iotlbentry;

    if (size <= TARGET_PAGE_SIZE) {
        sz = TARGET_PAGE_SIZE;
    } else {
        /*
         * TODO:
         * support for huge page?
         */
        sz = size;
    }

    uint64_t addr = hamt_badvaddr - mapping_base_address;
    if (is_unalign_2) {
        /* process the second page for unaligned access */
        addr = (addr + unalign_size) & TARGET_PAGE_MASK;
    }
    vaddr_page = addr  & TARGET_PAGE_MASK;
    paddr_page = paddr & TARGET_PAGE_MASK;

    section = address_space_translate_for_iotlb(cpu, asidx, paddr_page,
                                                &xlat, &sz, attrs, &prot);
    assert(sz >= TARGET_PAGE_SIZE);

    address = vaddr_page;
    if (size < TARGET_PAGE_SIZE) {
        /* Repeat the MMU check and TLB fill on every access.  */
        address |= TLB_INVALID_MASK;
    }
    if (attrs.byte_swap) {
        address |= TLB_BSWAP;
    }

    is_ram = memory_region_is_ram(section->mr);
    is_romd = memory_region_is_romd(section->mr);

    if (is_ram || is_romd) {
        /* RAM and ROMD both have associated host memory. */
        addend = (uintptr_t)memory_region_get_ram_ptr(section->mr) + xlat;
    } else {
        /* I/O does not; force the host address to NULL. */
        addend = 0;
    }

    write_address = address;
    if (is_ram) {
        iotlb = memory_region_get_ram_addr(section->mr) + xlat;
        /*
         * Computing is_clean is expensive; avoid all that unless
         * the page is actually writable.
         */
        if (prot & PAGE_WRITE) {
            if (section->readonly) {
                write_address |= TLB_DISCARD_WRITE;
            } else if (cpu_physical_memory_is_clean(iotlb)) {
                write_address |= TLB_NOTDIRTY;
            }
        }
    } else {
        /* I/O or ROMD */
        iotlb = memory_region_section_get_iotlb(cpu, section) + xlat;
        /*
         * Writes to romd devices must go through MMIO to enable write.
         * Reads to romd devices go through the ram_ptr found above,
         * but of course reads to I/O must go through MMIO.
         */
        write_address |= TLB_MMIO;
        if (!is_romd) {
            address = write_address;
        }
    }

    iotlbentry.addr = iotlb - vaddr_page;
    iotlbentry.attrs = attrs;

    tn.addend = addend - vaddr_page;
    if (prot & PAGE_READ) {
        tn.addr_read = address;
    } else {
        tn.addr_read = -1;
    }

    if (prot & PAGE_EXEC) {
        tn.addr_code = address;
    } else {
        tn.addr_code = -1;
    }

    tn.addr_write = -1;
    if (prot & PAGE_WRITE) {
        tn.addr_write = write_address;
        //FIX
        if (write_address & TLB_NOTDIRTY) prot &= ~PAGE_WRITE;
        if (prot & PAGE_WRITE_INV) {
            tn.addr_write |= TLB_INVALID_MASK;
        }
    }

    if (is_write) {
        hamt_store_helper(env, hamt_badvaddr-mapping_base_address, hamt_get_st_val(epc, cpuid),
            hamt_get_oi(epc, mmu_idx), ((uint64_t *)(data_storage_s[cpuid]))[32]+4,
            hamt_get_memop(epc),
            &tn, &iotlbentry, prot, epc,
            is_unalign_2, unalign_entry1, unalign_size, is_unalign_clean_ram);
    } else {
        hamt_load_helper(env,hamt_badvaddr-mapping_base_address, hamt_get_oi(epc, mmu_idx),
            ((uint64_t *)(data_storage_s[cpuid]))[32]+4, hamt_get_memop(epc),
            &tn, &iotlbentry, prot, epc,
            is_unalign_2, unalign_entry1, unalign_size);
    }

    return;
}

#define HAMT_USE_LSFPU

/* save native context into ENV */
static void hamt_save_from_native(CPUX86State *env)
{
    int cpuid = 0;
    if (qemu_tcg_mttcg_enabled()) {
        cpuid = (env_cpu(env))->cpu_index;
    }

    int i;
    uint32_t *reg_pos = (uint32_t *)((uint64_t *)(data_storage_s[cpuid]) + 24);
    uint32_t *dest    = (uint32_t *)((uint8_t *)env + 964);

    for (i=0; i<8; ++i) {
        *dest++ = *reg_pos;
        reg_pos += 2;
    }

    __asm__ __volatile__ (
#ifdef HAMT_USE_LSFPU
            "x86mftop   $t1\n\r"
            "andi       $t1,  $t1, 0x7\n\r"
            "st.w       $t1,  %0,  1312\n\r"
            "x86clrtm\n\r"
#endif
            "fst.d      $fa0, %0,  1328\n\r"
            "fst.d      $fa1, %0,  1344\n\r"
            "fst.d      $fa2, %0,  1360\n\r"
            "fst.d      $fa3, %0,  1376\n\r"
            "fst.d      $fa4, %0,  1392\n\r"
            "fst.d      $fa5, %0,  1408\n\r"
            "fst.d      $fa6, %0,  1424\n\r"
            "fst.d      $fa7, %0,  1440\n\r"
            "vst        $vr16,%0,  0\n\r"
            "vst        $vr17,%0,  64\n\r"
            "vst        $vr18,%0,  128\n\r"
            "vst        $vr19,%0,  192\n\r"
            "vst        $vr20,%0,  256\n\r"
            "vst        $vr21,%0,  320\n\r"
            "vst        $vr22,%0,  384\n\r"
            "vst        $vr23,%0,  448\n\r"
//            "st.d       $a6,  %0,  528\n\r"

            /* save fcsr */
            "ld.w        $t0, %0,  824\n\r"
            "bnez        $t0, 1f\n\r"
            "movfcsr2gr  $t0, $r0\n\r"
            "andi        $t0, $t0, 0x300\n\r"
            "st.w        $t0, %0,  816\n\r"
            "b           2f\n\r"
            "1:\n\r"
            "movfcsr2gr  $t0, $r0\n\r"
            "andi        $t0, $t0, 0x300\n\r"
            "st.w        $t0, %0,  820\n\r"
            "2:\n\r"

            /* save eflags */
            "x86mfflag  $t0,  63\n\r"
            "st.w       $t0,  %0,  1008\n\r"
            "ori        $t0,  $zero,0x1\n\r"
            "st.w       $t0,  %0,  1016\n\r"

            /* load DBT fcsr */
            "ld.d       $t0,  $sp, 128\n\r"
            "movgr2fcsr $r0,  $t0\n\r"
            :
            :"r"(env)
            :"memory", "t1", "t0"
            );
}

/* restore native context from ENV */
static void hamt_restore_to_native(CPUX86State *env)              
{                                             
    __asm__ __volatile__ (                    
            "ld.w       $t0,  %0,1008\n\r"    
            "x86mtflag  $t0,  63\n\r"         
#ifdef HAMT_USE_LSFPU
            "x86clrtm\n\r"
#endif
            "fld.d      $fa0, %0,  1328\n\r"  
            "fld.d      $fa1, %0,  1344\n\r"  
            "fld.d      $fa2, %0,  1360\n\r"  
            "fld.d      $fa3, %0,  1376\n\r"  
            "fld.d      $fa4, %0,  1392\n\r"  
            "fld.d      $fa5, %0,  1408\n\r"  
            "fld.d      $fa6, %0,  1424\n\r"  
            "fld.d      $fa7, %0,  1440\n\r"  
#ifdef HAMT_USE_LSFPU
            "x86settm\n\r"
            "ld.h       $t0,  %0,  1312\n\r"    
            "x86mttop   0\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   1\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   2\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   3\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   4\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   5\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   6\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "addi.w     $t0, $t0, -1\n\r"
            "x86mttop   7\n\r"
            "beq        $t0, $zero, 1f\n\r"
            "1:\n\r"
#endif
            "vld        $vr16,%0,  0\n\r"     
            "vld        $vr17,%0,  64\n\r"    
            "vld        $vr18,%0,  128\n\r"   
            "vld        $vr19,%0,  192\n\r"   
            "vld        $vr20,%0,  256\n\r"   
            "vld        $vr21,%0,  320\n\r"   
            "vld        $vr22,%0,  384\n\r"   
            "vld        $vr23,%0,  448\n\r"   
                                              
            "ld.w        $t0, %0,    824\n\r" 
            "bnez        $t0, 1f\n\r"         
            "ld.w        $t0, %0,    816\n\r" 
            "b           2f\n\r"              
            "1:\n\r"                          
            "ld.w        $t0, %0,    820\n\r" 
            "2:\n\r"                          
//            "movgr2fcsr  fcsr3, $t0\n\r"    
            ".word 0x0114c183\n\r"            
//            "movgr2fcsr  fcsr1, zero\n\r"   
            ".word 0x0114c001\n\r"            
//            "movgr2fcsr  fcsr2, zero\n\r"   
            ".word 0x0114c002\n\r"            
#ifdef HAMT_USE_LSFPU
            "x86settm\n\r"
#endif
            :                                 
            :"r"(env)                         
            :"memory", "t1", "t0"             
            );                                
}                                             

//#define HAMT_COUNT_GUEST_TLB
#ifdef HAMT_COUNT_GUEST_TLB
#define CSR_TLBIDX_EHINV       (_ULCAST_(1) << 31)
#define ENTRYLO_G              (_ULCAST_(1) << 6)
static void count_guest_tlb(void)
{
    int i;
    unsigned int index;
    unsigned int s_index, s_asid;
    unsigned long s_entryhi;
    unsigned long entryhi, entrylo0, entrylo1;
    int count = 0, qemu_count = 0, x86_count = 0;

    disable_pg();

    s_entryhi = read_csr_tlbehi();
    s_asid = read_csr_asid();
    s_index = read_csr_tlbidx();

    for (i = 0; i < 2111; ++i) {
        write_csr_tlbidx(i);
        tlb_read();
        index = read_csr_tlbidx();
        entrylo0 = read_csr_tlbelo0();
        entrylo1 = read_csr_tlbelo1();

        if (index & CSR_TLBIDX_EHINV)
            continue;

        (entrylo0 | entrylo1) & ENTRYLO_G ? ++qemu_count : ++x86_count;

        ++count;
    }

    if (count > highest_guest_tlb)
        highest_guest_tlb = count;
    if (qemu_count > highest_qemu_tlb)
        highest_qemu_tlb = qemu_count;
    if (x86_count > highest_x86_tlb)
        highest_x86_tlb = x86_count;

    write_csr_tlbehi(s_entryhi);
    write_csr_asid(s_asid);
    write_csr_tlbidx(s_index);
    enable_pg();
}
#endif

static
void hamt_exception_handler_softmmu(uint64_t hamt_badvaddr,
        CPUX86State *env, uint32_t *epc)
{
    int cpuid = 0;
    if (qemu_tcg_mttcg_enabled()) {
        cpuid = (env_cpu(env))->cpu_index;
    }

    hamt_save_from_native(env);

    uint64_t mem_addr = hamt_get_mem_address(epc, cpuid);
    if (mem_addr != hamt_badvaddr) {
        hamt_badvaddr = mem_addr;
    }

    int is_write = is_write_inst(epc) & 1;
    uint64_t addr = hamt_badvaddr - mapping_base_address;
    int mmu_idx = cpu_mmu_index(env, false);
    TCGMemOpIdx oi = hamt_get_oi(epc, mmu_idx);
    uint64_t retaddr = ((uint64_t *)(data_storage_s[cpuid]))[32] + 4;

    uint32_t opc = get_opc_from_epc(epc);

    uint64_t val = 0;
    if (is_write) {
        val = hamt_get_st_val_softmmu(env, epc, cpuid);
        switch(opc) {
        case OPC_ST_B: helper_ret_stb_mmu(env, addr, val, oi, retaddr); break;
        case OPC_ST_H: helper_le_stw_mmu(env, addr, val, oi, retaddr);  break;
        case OPC_FST_S:
        case OPC_ST_W: helper_le_stl_mmu(env, addr, val, oi, retaddr);  break;
        case OPC_FST_D:
        case OPC_ST_D: helper_le_stq_mmu(env, addr, val, oi, retaddr);  break;
        /* @val is not used for VST, env->temp_xmm will be used */
        case OPC_VST: latxs_helper_le_stdq_mmu(env, addr, val, oi, retaddr); break;
        default: assert(0); break;
        }
    } else {
        switch(opc) {
        case OPC_LD_B:  val = helper_ret_ldsb_mmu(env, addr, oi, retaddr); break;
        case OPC_LD_BU: val = helper_ret_ldub_mmu(env, addr, oi, retaddr); break;
        case OPC_LD_H:  val = helper_le_ldsw_mmu(env, addr, oi, retaddr);  break;
        case OPC_LD_HU: val = helper_le_lduw_mmu(env, addr, oi, retaddr);  break;
        case OPC_LD_W:  val = helper_le_ldsl_mmu(env, addr, oi, retaddr);  break;
        case OPC_FLD_S:
        case OPC_LD_WU: val = helper_le_ldul_mmu(env, addr, oi, retaddr);  break;
        case OPC_FLD_D:
        case OPC_LD_D:  val = helper_le_ldq_mmu(env, addr, oi, retaddr);   break;
        /* no return for VLD, env->temp_xmm will be used */
        case OPC_VLD:  latxs_helper_le_lddq_mmu(env, addr, oi, retaddr);   break;
        default: assert(0); break;
        }
        load_into_reg_softmmu(env, val, epc, cpuid);
    }

    /* return to the next instruction in native */
    ((uint64_t *)(data_storage_s[cpuid]))[32] += 4;

    latxs_sigint_check_in_hamt(env, epc);
 
    hamt_restore_to_native(env);
}

static int hamt_delay_res[64];
static int __attribute__((noinline)) __hamt_do_delay(int n)
{
    int hamt_delay_num = 2;
    int loop = option_hamt_delay;
    int i = 1;
    for (; i < loop; ++i) {
        hamt_delay_num = hamt_delay_num * i + hamt_delay_res[i & 0x3f];
        hamt_delay_res[i & 0x3f] = n;
    }
    return hamt_delay_num;
}
static void __attribute__((noinline)) hamt_do_delay(int n)
{
    if (!hamt_delay()) return;

    hamt_delay_res[1] = hamt_delay_res[0];
    int res = __hamt_do_delay(n);
    hamt_delay_res[0] = res;
}

void hamt_exception_handler(uint64_t hamt_badvaddr,
        CPUX86State *env, uint32_t *epc,
        int is_unalign_2, CPUTLBEntry *unalign_entry1, int unalign_size,
        int is_unalign_clean_ram)
{
    env->sigint_hamt_flag = 1;
    hamt_do_delay(hamt_badvaddr);

    if (hamt_softmmu()) {
        hamt_exception_handler_softmmu(hamt_badvaddr, env, epc);
        env->sigint_hamt_flag = 0;
        return;
    }

    /*
     * JOBS
     *     1. get hpa from x86_addr
     *     2. fill x86_addr -> hpa transition into page table
     *     3. possible inject a PF fault
     *     4. dispatch MMIO function
     * DO NOT FORGET
     *     translate address to i386 vm address
     */

#ifdef HAMT_COUNT_GUEST_TLB
    count_guest_tlb();
#endif
    if (!is_unalign_2) {
        /* save native context into ENV */
        hamt_save_from_native(env);
    }


    tlb_lsm++;

    if (hamt_pg_asid()) {
        if (env->cr[3] == 0 && check_cr3_in_htable(0) == NULL) {
            hamt_set_context(0);
        }
    }

    uint64_t addr = hamt_badvaddr - mapping_base_address;
    if (is_unalign_2) {
        /* process the second page for unaligned access */
        addr = (addr + unalign_size) & TARGET_PAGE_MASK;
    }

    CPUState *cs = env_cpu(env);
    X86CPU *cpu = X86_CPU(cs);
    uint64_t ptep, pte;
    int32_t a20_mask;
    uint32_t pde_addr, pte_addr;
    int error_code = 0;
    int is_dirty, prot, page_size, is_write, is_user;
    hwaddr paddr;
    uint64_t rsvd_mask = PG_ADDRESS_MASK & ~MAKE_64BIT_MASK(0, cpu->phys_bits);
    uint32_t page_offset;
    int mmu_idx = cpu_mmu_index(env, false);

    is_user = mmu_idx == MMU_USER_IDX;

    /*
     * in accel/tcg/excp_helper.c handle_mmu_fault
     * parameter is_write1 could be:
     * 0: load data
     * 1: store data
     * 2: load instructions
     *
     * it is not necessary for hamt to deal with
     * is_write1 = 2, for such circumstances only happen
     * in disas_insn, while hamt_exception_handler will only be
     * triggered by addresses modified
     */
    is_write = is_write_inst(epc) & 1;

    a20_mask = x86_get_a20_mask(env);
    if (!(env->cr[0] & CR0_PG_MASK)) {
        pte = addr;
        prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
        page_size = 4096;
        goto do_mapping;
    }

    if (!(env->efer & MSR_EFER_NXE)) {
        rsvd_mask |= PG_NX_MASK;
    }

    if (env->cr[4] & CR4_PAE_MASK) {
        uint64_t pde, pdpe;
        target_ulong pdpe_addr;

        /* XXX: load them when cr3 is loaded ? */
        pdpe_addr = ((env->cr[3] & ~0x1f) + ((addr >> 27) & 0x18)) & a20_mask;
        pdpe_addr = get_hphys(cs, pdpe_addr, MMU_DATA_STORE, false);
        pdpe = x86_ldq_phys(cs, pdpe_addr);
        if (!(pdpe & PG_PRESENT_MASK)) {
            goto do_fault;
        }
        rsvd_mask |= PG_HI_USER_MASK;
        if (pdpe & (rsvd_mask | PG_NX_MASK)) {
            goto do_fault_rsvd;
        }
        ptep = PG_NX_MASK | PG_USER_MASK | PG_RW_MASK;

        pde_addr = ((pdpe & PG_ADDRESS_MASK) + (((addr >> 21) & 0x1ff) << 3)) &
            a20_mask;
        pde_addr = get_hphys(cs, pde_addr, MMU_DATA_STORE, NULL);
        pde = x86_ldq_phys(cs, pde_addr);
        if (!(pde & PG_PRESENT_MASK)) {
            goto do_fault;
        }
        if (pde & rsvd_mask) {
            goto do_fault_rsvd;
        }
        ptep &= pde ^ PG_NX_MASK;
        if (pde & PG_PSE_MASK) {
            /* 2 MB page */
            page_size = 2048 * 1024;
            pte_addr = pde_addr;
            pte = pde;
            goto do_check_protect;
        }
        /* 4 KB page */
        if (!(pde & PG_ACCESSED_MASK)) {
            pde |= PG_ACCESSED_MASK;
            x86_stl_phys_notdirty(cs, pde_addr, pde);
        }
        pte_addr = ((pde & PG_ADDRESS_MASK) + (((addr >> 12) & 0x1ff) << 3)) &
            a20_mask;
        pte_addr = get_hphys(cs, pte_addr, MMU_DATA_STORE, NULL);
        pte = x86_ldq_phys(cs, pte_addr);
        if (!(pte & PG_PRESENT_MASK)) {
            goto do_fault;
        }
        if (pte & rsvd_mask) {
            goto do_fault_rsvd;
        }
        /* combine pde and pte nx, user and rw protections */
        ptep &= pte ^ PG_NX_MASK;
        page_size = 4096;
    } else {
        uint32_t pde;

        /* page directory entry */
        pde_addr = ((env->cr[3] & ~0xfff) + ((addr >> 20) & 0xffc)) &
            a20_mask;
        pde_addr = get_hphys(cs, pde_addr, MMU_DATA_STORE, NULL);
        pde = x86_ldl_phys(cs, pde_addr);
        if (!(pde & PG_PRESENT_MASK)) {
            goto do_fault;
        }
        ptep = pde | PG_NX_MASK;

        /* if PSE bit is set, then we use a 4MB page */
        if ((pde & PG_PSE_MASK) && (env->cr[4] & CR4_PSE_MASK)) {
            page_size = 4096 * 1024;
            pte_addr = pde_addr;

            /* Bits 20-13 provide bits 39-32 of the address, bit 21 is reserved.
             * Leave bits 20-13 in place for setting accessed/dirty bits below.
             */
            pte = pde | ((pde & 0x1fe000LL) << (32 - 13));
            rsvd_mask = 0x200000;
            goto do_check_protect_pse36;
        }

        if (!(pde & PG_ACCESSED_MASK)) {
            pde |= PG_ACCESSED_MASK;
            x86_stl_phys_notdirty(cs, pde_addr, pde);
        }

        /* page directory entry */
        pte_addr = ((pde & ~0xfff) + ((addr >> 10) & 0xffc)) &
            a20_mask;
        pte_addr = get_hphys(cs, pte_addr, MMU_DATA_STORE, NULL);
        pte = x86_ldl_phys(cs, pte_addr);
        if (!(pte & PG_PRESENT_MASK)) {
            goto do_fault;
        }
        /* combine pde and pte user and rw protections */
        ptep &= pte | PG_NX_MASK;
        page_size = 4096;
        rsvd_mask = 0;
    }
do_check_protect:
    rsvd_mask |= (page_size - 1) & PG_ADDRESS_MASK & ~PG_PSE_PAT_MASK;
do_check_protect_pse36:
    if (pte & rsvd_mask) {
        goto do_fault_rsvd;
    }
    ptep ^= PG_NX_MASK;

    /* can the page can be put in the TLB?  prot will tell us */
    if (is_user && !(ptep & PG_USER_MASK)) {
        goto do_fault_protect;
    }

    prot = 0;
    if (mmu_idx != MMU_KSMAP_IDX || !(ptep & PG_USER_MASK)) {
        prot |= PAGE_READ;
        if ((ptep & PG_RW_MASK) || (!is_user && !(env->cr[0] & CR0_WP_MASK))) {
            prot |= PAGE_WRITE;
        }
    }
    if (!(ptep & PG_NX_MASK) &&
        (mmu_idx == MMU_USER_IDX ||
         !((env->cr[4] & CR4_SMEP_MASK) && (ptep & PG_USER_MASK)))) {
        prot |= PAGE_EXEC;
    }
    if ((env->cr[4] & CR4_PKE_MASK) && (env->hflags & HF_LMA_MASK) &&
        (ptep & PG_USER_MASK) && env->pkru) {
        uint32_t pk = (pte & PG_PKRU_MASK) >> PG_PKRU_BIT;
        uint32_t pkru_ad = (env->pkru >> pk * 2) & 1;
        uint32_t pkru_wd = (env->pkru >> pk * 2) & 2;
        uint32_t pkru_prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;

        if (pkru_ad) {
            pkru_prot &= ~(PAGE_READ | PAGE_WRITE);
        } else if (pkru_wd && (is_user || env->cr[0] & CR0_WP_MASK)) {
            pkru_prot &= ~PAGE_WRITE;
        }

        prot &= pkru_prot;
        if ((pkru_prot & (1 << is_write)) == 0) {
            assert(is_write != 2);
            error_code |= PG_ERROR_PK_MASK;
            goto do_fault_protect;
        }
    }

    if ((prot & (1 << is_write)) == 0) {
        goto do_fault_protect;
    }

    /* yes, it can! */
    is_dirty = is_write && !(pte & PG_DIRTY_MASK);
    if (!(pte & PG_ACCESSED_MASK) || is_dirty) {
        pte |= PG_ACCESSED_MASK;
        if (is_dirty) {
            pte |= PG_DIRTY_MASK;
        }
        x86_stl_phys_notdirty(cs, pte_addr, pte);
    }

    if (!(pte & PG_DIRTY_MASK)) {
        /* only set write access if already dirty... otherwise wait
           for dirty access */
        assert(!is_write);
        prot &= ~PAGE_WRITE;
    }
do_mapping:
    pte = pte & a20_mask;

    /* align to page_size */
    pte &= PG_ADDRESS_MASK & ~(page_size - 1);
    page_offset = addr & (page_size - 1);
    paddr = get_hphys(cs, pte + page_offset, is_write, &prot);

    /* Even if 4MB pages, we map only one 4KB page in the cache to
       avoid filling it too fast */
    // vaddr = addr & TARGET_PAGE_MASK;
    paddr &= TARGET_PAGE_MASK;

    assert(prot & (1 << is_write));

    hamt_process_addr_mapping(cs, hamt_badvaddr, paddr, cpu_get_mem_attrs(env),
            prot, mmu_idx, page_size, is_write, epc,
            is_unalign_2, unalign_entry1, unalign_size, is_unalign_clean_ram);

    /* restore native context from ENV */
    hamt_restore_to_native(env);
    return;

do_fault_rsvd:
    error_code |= PG_ERROR_RSVD_MASK;
do_fault_protect:
    error_code |= PG_ERROR_P_MASK;
do_fault:
    error_code |= (is_write << PG_ERROR_W_BIT);
    if (is_user)
        error_code |= PG_ERROR_U_MASK;
    if (is_write == 2 &&
        (((env->efer & MSR_EFER_NXE) &&
          (env->cr[4] & CR4_PAE_MASK)) ||
         (env->cr[4] & CR4_SMEP_MASK)))
        error_code |= PG_ERROR_I_D_MASK;
    if (env->intercept_exceptions & (1 << EXCP0E_PAGE)) {
        /* cr2 is not modified in case of exceptions */
        x86_stq_phys(cs,
                 env->vm_vmcb + offsetof(struct vmcb, control.exit_info_2),
                 addr);
    } else {
        env->cr[2] = addr;
    }
    env->error_code = error_code;
    cs->exception_index = EXCP0E_PAGE;

    /* save native context into ENV */
    hamt_save_from_native(env);
    /*
     * insert PF fault
     */
    page_fault++;
    raise_exception_err_ra(env, cs->exception_index,
                               env->error_code, (uint64_t)epc);

    return;
}

static uint32_t gen_inst(uint32_t opc, uint32_t rd, uint32_t rj, uint32_t imm)
{
    uint32_t inst;

    inst = opc;
    inst |= (imm & 0xfff) << 10;
    inst |= (rj & 0x1f) << 5;
    inst |= (rd & 0x1f);

    return inst;
}

/* Cache operations. */
static void local_flush_icache_range(void)
{
    asm volatile ("\tibar 0\n"::);
}

/*
 * DATA_STORAGE:
 * 0-31: native context
 *   32: EPC
 *   33: badvaddr
 *   40: tlbr_ehi
 */

static int hamt_fast_exception_handler(void)
{
    /* TODO: tlb fast exception in hamt multi vcpu mode */
    int cpuid = 0;

    uint64_t *data = (void *)(data_storage_s[cpuid]);

    uint32_t *epc = (void *)(data[32] & ~0x1);
    uint64_t badv = data[33];
    uint32_t opc = get_opc_from_epc(epc);
    void *env = (void *)(data[23]); /* $s0 */

    latxs_counter_hamt_fast(env_cpu(env));

    if (!badv) {
        latxs_counter_hamt_fast_badv0(env_cpu(env));
        return 0;
    }

    int is_store = 0;
    int is_load = 0;
    switch(opc) {
    case OPC_ST_B:
    case OPC_ST_H:
    case OPC_ST_W:
    case OPC_ST_D:
    case OPC_FST_S: /* ffldst */
    case OPC_FST_D: /* ffldst */
    case OPC_VST:   /* ffldst */
        is_store = 1;
        break;
    case OPC_LD_B:
    case OPC_LD_BU:
    case OPC_LD_H:
    case OPC_LD_HU:
    case OPC_LD_W:
    case OPC_LD_WU:
    case OPC_LD_D:
    case OPC_FLD_S: /* ffldst */
    case OPC_FLD_D: /* ffldst */
    case OPC_VLD:   /* ffldst */
        is_load = 1;
        break;
    default:
        break;
    }

    if (!is_load && !is_store) {
        latxs_counter_hamt_fast_undef(env_cpu(env));
        return 0;
    }

    if (is_store) {
        latxs_counter_hamt_fast_st(env_cpu(env));
    } else {
        latxs_counter_hamt_fast_ld(env_cpu(env));
    }

    int res = 0;
    if (is_store) {
        res = hamt_fast_store(env, badv);
    } else {
        res = hamt_fast_load(env, badv);
    }

    if (res == 2) {
        if (is_store) {
            latxs_counter_hamt_fast_st_stlb_ok(env_cpu(env));
        } else {
            latxs_counter_hamt_fast_ld_stlb_ok(env_cpu(env));
        }
    }

    if (res == 1) {
        if (is_store) {
            latxs_counter_hamt_fast_st_ok(env_cpu(env));
        } else {
            latxs_counter_hamt_fast_ld_ok(env_cpu(env));
        }
    }

    return res;
}

static void build_tlb_fast_exception(void)
{
    /* TODO: tlb fast exception in hamt multi vcpu mode */
    int cpuid = 0;

    uint32_t *p = (uint32_t *)fastcode_storage_s[cpuid];
    int i = 0, j = 0;

    /* $t0 already points to DATA_STORAGE */
    /* save all integer registers */
    for (j=0; j<32; ++j) {
        /* t0, t1, t2 */
        if (j==12 || j==13 || j==14) continue;
        /* st.d reg, t0, offset */
        p[i++] = gen_inst(OPC_ST_D, j, 12, 8*j); 
    }

    /* goto handler */
    uint64_t f = (uint64_t)hamt_fast_exception_handler;
    p[i++] = 0x0015b18c;
    p[i++] = 0x1400000c | (((f >> 12) & 0xfffff) << 5);
    p[i++] = 0x0380018c | (((f) & 0xfff) << 10);
    p[i++] = 0x1600000c | (((f >> 32) & 0xfffff) << 5);
    p[i++] = 0x0300018c | (((f >> 52) & 0xfff) << 10);
    /* jirl $ra, $t0, 0 */
    p[i++] = 0x4c000181;
    /* move a0 to t2 */
    p[i++] = 0x15008e;

    /* t0 = 0x81000000000 (DATA_STORAGE) */
    p[i++] = 0x0015b18c;
    p[i++] = 0x1600102c;
    /* restore all integer registers */
    for (j=0; j<32; ++j) {
        /* t0, t1, t2 */
        if (j==12 || j==13 || j==14) continue;
        /* ld.d reg, t0, offset */
        p[i++] = gen_inst(OPC_LD_D, j, 12, 8*j); 
    }

    /* jirl zero, ra, 0 */
    p[i++] = 0x4c000020;

    assert(i <= (0x4000 >> 2));
    local_flush_icache_range();
}

static void build_tlb_invalid_trampoline_mt(int cpuid)
{
    uint32_t *p = (uint32_t *)code_storage_s[cpuid];
    int i=0, j=0;

    /* 
     * save $t0 in stack
     *     addi.d $sp, $sp, -8
     *     st.d   $t0, $sp, 0
     */
    p[i++] = 0x02ffe063;
    p[i++] = 0x29c0006c;
    /*
     * load $t0 with DATA_STORAGE_ADDRESS_S ## cpuid
     *     xor $t0, $t0, $t0
     *     lu32i.d $t0, 0x8100000000>>32
     */
    p[i++] = 0x0015b18c;
    switch (cpuid) {
    case 0: p[i++] = 0x1600102c; break; /* 0x81_0000_0000 */
    case 1: p[i++] = 0x1600104c; break; /* 0x82_0000_0000 */
    case 2: p[i++] = 0x1600106c; break; /* 0x83_0000_0000 */
    case 3: p[i++] = 0x1600108c; break; /* 0x84_0000_0000 */
    default: assert(0); break;
    }
    /*
     * save all but $t0 and $sp
     */
    for (j=0; j<32; ++j) {
        if (j == 12 || j == 3) continue;
        p[i++] = gen_inst(OPC_ST_D, j, 12, 8*j); 
    }
    /*
     * save original $t0 in data_storage
     * & restore and save $sp
     *     ld.d   $t1, $sp, 0
     *     st.d   $t1, $t0, 96
     *     addi.d $sp, $sp, 8
     *     st.d   $sp, $t0, 24
     */
    p[i++] = 0x28c0006d;
    p[i++] = 0x29c1818d;
    p[i++] = 0x02c02063;
    p[i++] = 0x29c06183;

    /* TILL NOW, WE PRESERVE ALL REGISTERS IN DATA_STORAGE */

    /*
     * prepare parameters for hamt_exception_handler
     * hamt_badvaddr(a0)
     *     ld.d $a0, $t0, 264
     * CPUX86State *env(a1)
     *     ld.d $a1, $t0, 184 
     * epc(a2)
     *     ld.d $a2, $t0, 256
     * is_unalign_2(a3)
     *     or   $a3, zero, zero
     */
    p[i++] = 0x28c42184;
    p[i++] = 0x28c2e185;
    p[i++] = 0x28c40186;
    p[i++] = 0x00150007;

    uint64_t f = (uint64_t)hamt_exception_handler;
    /*
     * put hamt_exception_handler in $t0
     *     xor     $t0, $t0, $t0
     *     lu12i.w $t0, 0 | ((f >> 12) & 0xfffff)
     *     ori     $t0, $t0, 0 | ((f) & 0xfff)
     *     lu32i.d $t0, 0 | ((f >> 32) & 0xfffff)
     *     lu52i.d $t0, $t0, 0 | ((f >> 52) & 0xfff)
     */
    p[i++] = 0x0015b18c;
    p[i++] = 0x1400000c | (((f >> 12) & 0xfffff) << 5);
    p[i++] = 0x0380018c | (((f) & 0xfff) << 10);
    p[i++] = 0x1600000c | (((f >> 32) & 0xfffff) << 5);
    p[i++] = 0x0300018c | (((f >> 52) & 0xfff) << 10);

    /*
     * jump to hamt_exception_handler
     *     jirl $ra, $t0, 0
     */
    p[i++] = 0x4c000181;

    /*
     * use exception to restore regs & resume execution
     *     xor     $t0, $t0, $t0
     *     lu32i.d $t0, 0x81
     *     ori     $t0, $t0, 0xfd0
     *     addi.d  $t1, $zero, 0x3
     *     st.d    $t1, $t0, 0x0
     *     break 0
     */
    p[i++] = 0x0015b18c;
    /*p[i++] = 0x1600102c;*/
    switch (cpuid) {
    case 0: p[i++] = 0x1600102c; break; /* 0x81_0000_0000 */
    case 1: p[i++] = 0x1600104c; break; /* 0x82_0000_0000 */
    case 2: p[i++] = 0x1600106c; break; /* 0x83_0000_0000 */
    case 3: p[i++] = 0x1600108c; break; /* 0x84_0000_0000 */
    default: assert(0); break;
    }
    p[i++] = 0x03bf418c;
    p[i++] = 0x02c00c0d;
    p[i++] = 0x29c0018d;
    p[i++] = 0x002a0000;

    local_flush_icache_range();
}

static void build_tlb_invalid_trampoline(void)
{
    uint32_t *p = (uint32_t *)code_storage_s[0];
    int i=0, j=0;

    /* 
     * save $t0 in stack
     *     addi.d $sp, $sp, -8
     *     st.d   $t0, $sp, 0
     */
    p[i++] = 0x02ffe063;
    p[i++] = 0x29c0006c;
    /*
     * load $t0 with DATA_STORAGE_ADDRESS
     *     xor $t0, $t0, $t0
     *     lu32i.d $t0, 0x8100000000>>32
     */
    p[i++] = 0x0015b18c;
    p[i++] = 0x1600102c;
    /*
     * save all but $t0 and $sp
     */
    for (j=0; j<32; ++j) {
        if (j == 12 || j == 3) continue;
        p[i++] = gen_inst(OPC_ST_D, j, 12, 8*j); 
    }
    /*
     * save original $t0 in data_storage
     * & restore and save $sp
     *     ld.d   $t1, $sp, 0
     *     st.d   $t1, $t0, 96
     *     addi.d $sp, $sp, 8
     *     st.d   $sp, $t0, 24
     */
    p[i++] = 0x28c0006d;
    p[i++] = 0x29c1818d;
    p[i++] = 0x02c02063;
    p[i++] = 0x29c06183;

    /* TILL NOW, WE PRESERVE ALL REGISTERS IN DATA_STORAGE */

    /*
     * prepare parameters for hamt_exception_handler
     * hamt_badvaddr(a0)
     *     ld.d $a0, $t0, 264
     * CPUX86State *env(a1)
     *     ld.d $a1, $t0, 184 
     * epc(a2)
     *     ld.d $a2, $t0, 256
     * is_unalign_2(a3)
     *     or   $a3, zero, zero
     */
    p[i++] = 0x28c42184;
    p[i++] = 0x28c2e185;
    p[i++] = 0x28c40186;
    p[i++] = 0x00150007;

    uint64_t f = (uint64_t)hamt_exception_handler;
    /*
     * put hamt_exception_handler in $t0
     *     xor     $t0, $t0, $t0
     *     lu12i.w $t0, 0 | ((f >> 12) & 0xfffff)
     *     ori     $t0, $t0, 0 | ((f) & 0xfff)
     *     lu32i.d $t0, 0 | ((f >> 32) & 0xfffff)
     *     lu52i.d $t0, $t0, 0 | ((f >> 52) & 0xfff)
     */
    p[i++] = 0x0015b18c;
    p[i++] = 0x1400000c | (((f >> 12) & 0xfffff) << 5);
    p[i++] = 0x0380018c | (((f) & 0xfff) << 10);
    p[i++] = 0x1600000c | (((f >> 32) & 0xfffff) << 5);
    p[i++] = 0x0300018c | (((f >> 52) & 0xfff) << 10);

    /*
     * jump to hamt_exception_handler
     *     jirl $ra, $t0, 0
     */
    p[i++] = 0x4c000181;

    /*
     * use exception to restore regs & resume execution
     *     xor     $t0, $t0, $t0
     *     lu32i.d $t0, 0x81
     *     ori     $t0, $t0, 0xfd0
     *     addi.d  $t1, $zero, 0x3
     *     st.d    $t1, $t0, 0x0
     *     break 0
     */
    p[i++] = 0x0015b18c;
    p[i++] = 0x1600102c;
    p[i++] = 0x03bf418c;
    p[i++] = 0x02c00c0d;
    p[i++] = 0x29c0018d;
    p[i++] = 0x002a0000;

    local_flush_icache_range();
}

void hamt_flush_all(void)
{
    if (hamt_interpreter()) {
        return;
    }

    hamt_flush_tlb_all();
    other_source++;
}

static void enable_x86vm_hamt_rr(void)
{
    int i;
    data_storage_s[0] = (uint64_t)mmap(
            (void *)DATA_STORAGE_ADDRESS, 4096,
            PROT_RWX, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    code_storage_s[0] = (uint64_t)mmap(
            (void *)CODE_STORAGE_ADDRESS, 4096,
            PROT_RWX, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    fastcode_storage_s[0] = (uint64_t)mmap(
            (void *)FASTCODE_STORAGE_ADDRESS, 4096,
            PROT_RWX, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    assert(data_storage_s[0] == DATA_STORAGE_ADDRESS);
    assert(code_storage_s[0] == CODE_STORAGE_ADDRESS);

    latx_perfmap_insert((void *)code_storage_s[0], 4096,
            "hamt_codes_to_excphandler");
    latx_perfmap_flush();

    for(i = 0; i < MAX_ASID; ++i) {
        QLIST_INIT(&(hamt_cr3_htable[i].pgtables_list));
        hamt_cr3_htable[i].pgtable_num = 0;
    }

    memset(asid_map, 0, sizeof(uint64_t) * 16);

    build_tlb_invalid_trampoline();
    build_tlb_fast_exception();
}

#define HAMT_MTTCG_STORAGE_ALLOC(id, val) do {          \
if (id == val) {                                        \
    data_storage_s[id] = (uint64_t)mmap(                \
        (void *)DATA_STORAGE_ADDRESS_S ## val, 4096,    \
        PROT_RWX, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);  \
    code_storage_s[id] = (uint64_t)mmap(                \
        (void *)CODE_STORAGE_ADDRESS_S ## val, 4096,    \
        PROT_RWX, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);  \
    assert(data_storage_s[id] == DATA_STORAGE_ADDRESS_S ## val);    \
    assert(code_storage_s[id] == CODE_STORAGE_ADDRESS_S ## val);    \
    latx_perfmap_insert((void *)code_storage_s[id], 4096,           \
            "hamt_codes_to_excphandler_" #val);                     \
    pr_info("data[%d] %p", id, (void *)(data_storage_s[id]));       \
    pr_info("code[%d] %p", id, (void *)code_storage_s[id]);         \
} } while (0)

static void enable_x86vm_hamt_mt(int cpuid)
{
    HAMT_MTTCG_STORAGE_ALLOC(cpuid, 0);
    HAMT_MTTCG_STORAGE_ALLOC(cpuid, 1);
    HAMT_MTTCG_STORAGE_ALLOC(cpuid, 2);
    HAMT_MTTCG_STORAGE_ALLOC(cpuid, 3);
    latx_perfmap_flush();
    build_tlb_invalid_trampoline_mt(cpuid);
    /* TODO: asdid map in hamt multi vcpu mode */
    /* TODO: tlb fast exception in hamt multi vcpu mode */
    assert(!hamt_have_tlbr_fastpath());
}

void enable_x86vm_hamt(int mode, int cpuid)
{
    /* single vcpu thread */
    if (mode == HAMT_MODE_RR) {
        static int enabled = 0;
        assert(!enabled);
        enable_x86vm_hamt_rr();
        enabled = 1;
    }
    /* multi vcpu thread */
    if (mode == HAMT_MODE_MT) {
        enable_x86vm_hamt_mt(cpuid);
    }
}

static void __attribute__((constructor)) in_hamt_init(void)
{
    pthread_key_create(&in_hamt, NULL);
}

bool hamt_started(void)
{
    return (bool)pthread_getspecific(in_hamt);
}

void start_hamt(bool *enable)
{
    pthread_setspecific(in_hamt, enable);
}

void stop_hamt(bool *disable)
{
    pthread_setspecific(in_hamt, disable);
}
