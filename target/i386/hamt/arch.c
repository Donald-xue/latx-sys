#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "arch.h"
#include "internal.h"
#include "interface.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>

#include "hamt.h"
#include "hamt-assert.h"
#include "latx-perfmap.h"

void arch_dump_regs(int debug_fd, struct kvm_regs regs);
void arch_dump_regs(int debug_fd, struct kvm_regs regs)
{
    dprintf(debug_fd, "\n Registers:\n");
    dprintf(debug_fd, " ----------\n");
    dprintf(debug_fd, "$0   : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[0],
        (unsigned long long)regs.gpr[1],
        (unsigned long long)regs.gpr[2],
        (unsigned long long)regs.gpr[3]);
    dprintf(debug_fd, "$4   : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[4],
        (unsigned long long)regs.gpr[5],
        (unsigned long long)regs.gpr[6],
        (unsigned long long)regs.gpr[7]);
    dprintf(debug_fd, "$8   : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[8],
        (unsigned long long)regs.gpr[9],
        (unsigned long long)regs.gpr[10],
        (unsigned long long)regs.gpr[11]);
    dprintf(debug_fd, "$12  : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[12],
        (unsigned long long)regs.gpr[13],
        (unsigned long long)regs.gpr[14],
        (unsigned long long)regs.gpr[15]);
    dprintf(debug_fd, "$16  : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[16],
        (unsigned long long)regs.gpr[17],
        (unsigned long long)regs.gpr[18],
        (unsigned long long)regs.gpr[19]);
    dprintf(debug_fd, "$20  : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[20],
        (unsigned long long)regs.gpr[21],
        (unsigned long long)regs.gpr[22],
        (unsigned long long)regs.gpr[23]);
    dprintf(debug_fd, "$24  : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[24],
        (unsigned long long)regs.gpr[25],
        (unsigned long long)regs.gpr[26],
        (unsigned long long)regs.gpr[27]);
    dprintf(debug_fd, "$28  : %016llx %016llx %016llx %016llx\n",
        (unsigned long long)regs.gpr[28],
        (unsigned long long)regs.gpr[29],
        (unsigned long long)regs.gpr[30],
        (unsigned long long)regs.gpr[31]);

    dprintf(debug_fd, "pc  : %016llx\n", (unsigned long long)regs.pc);

    dprintf(debug_fd, "\n");
}

enum ACCESS_OP {
    GET = KVM_GET_ONE_REG,
    SET = KVM_SET_ONE_REG,
};

static void kvm_access_reg(const struct kvm_cpu *cpu,
        struct kvm_one_reg *reg, enum ACCESS_OP op)
{
    if (ioctl(cpu->vcpu_fd, op, reg) < 0) {
        die("kvm_access_reg");
    }
}

static u64 kvm_access_csr_reg(const struct kvm_cpu *cpu,
        u64 id, enum ACCESS_OP op, u64 value)
{
    struct kvm_one_reg reg;
    u64 v = (op == GET) ? 0 : value;
    reg.addr = (u64) & (v);
    reg.id = id;

    kvm_access_reg(cpu, &reg, op);
    return v;
}

static u64 kvm_get_csr_reg(const struct kvm_cpu *cpu, u64 id)
{
    return kvm_access_csr_reg(cpu, id, GET, 0);
}

#define USE_KVM_SET_CSR_FUNC
#ifdef USE_KVM_SET_CSR_FUNC
static void kvm_set_csr_reg(const struct kvm_cpu *cpu, u64 id, u64 v)
{
    kvm_access_csr_reg(cpu, id, SET, v);
}
#endif

static void kvm_enable_fpu(struct kvm_cpu *cpu)
{
    struct kvm_enable_cap cap;
    memset(&cap, 0, sizeof(cap));
    cap.cap = KVM_CAP_LOONGARCH_FPU;

    if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
        die("Unable enable fpu in guest");
    }

    cap.cap = KVM_CAP_LOONGARCH_LSX;
    if (ioctl(cpu->vcpu_fd, KVM_ENABLE_CAP, &cap) < 0) {
        die("Unable enable msa in guest");
    }

    // 从 kvm_vcpu_ioctl_enable_cap 可以看到不需要手动打开 lasx
}

static void kvm_access_fpu_regs(struct kvm_cpu *cpu,
        const struct kvm_fpu *fpu_regs, enum ACCESS_OP op)
{
    ioctl(cpu->vcpu_fd, op == SET ? KVM_SET_FPU : KVM_GET_FPU, &(cpu->info.fpu));
}

static void kvm_get_fpu_regs(struct kvm_cpu *cpu,
        const struct kvm_fpu *fpu_regs)
{
    kvm_access_fpu_regs(cpu, fpu_regs, GET);
}

static void kvm_set_fpu_regs(struct kvm_cpu *cpu,
        const struct kvm_fpu *fpu_regs)
{
    kvm_access_fpu_regs(cpu, fpu_regs, SET);
}

static void dup_fpu(struct kvm_cpu *child_cpu,
        const struct kvm_fpu *parent_fpu)
{
    kvm_enable_fpu(child_cpu);
    kvm_set_fpu_regs(child_cpu, parent_fpu);
}

void kvm_get_parent_thread_info(struct kvm_cpu *parent_cpu)
{
    if (ioctl(parent_cpu->vcpu_fd, KVM_GET_REGS, &(parent_cpu->info.regs))
            < 0) {
        die("KVM_GET_REGS");
    }

    parent_cpu->info.era = kvm_get_csr_reg(parent_cpu, KVM_CSR_EPC);

    kvm_get_fpu_regs(parent_cpu, &parent_cpu->info.fpu);
}

extern void get_fpu_regs(struct kvm_fpu *);

static void init_fpu(struct kvm_cpu *cpu)
{
    kvm_enable_fpu(cpu);

    struct kvm_fpu fpu_regs;

    get_fpu_regs(&fpu_regs);

    BUILD_ASSERT(offsetof(struct kvm_fpu, fcsr)    == VCPU_FCSR0);
   // BUILD_ASSERT(offsetof(struct kvm_fpu, vcsr)    == VCPU_VCSR);
    BUILD_ASSERT(offsetof(struct kvm_fpu, fcc)     == VCPU_FCC);
    BUILD_ASSERT(offsetof(struct kvm_fpu, fpr[0])  == VCPU_FPR0);
    BUILD_ASSERT(offsetof(struct kvm_fpu, fpr[31]) == VCPU_FPR0 + 31 * VCPU_FPR_LEN);

    kvm_set_fpu_regs(cpu, &fpu_regs);
}

// extern void err_entry_begin(void);
// extern void err_entry_end(void);
extern void fpe_entry_begin(void);
extern void fpe_entry_end(void);
extern void tlb_refill_entry_begin(void);
extern void tlb_refill_entry_end(void);
extern void tlb_ultra_fast_refill_entry_begin(void);
extern void tlb_ultra_fast_refill_entry_end(void);
extern void tlb_fast_refill_entry_begin(void);
extern void tlb_fast_refill_entry_end(void);
extern void tlb_ifetch_entry_begin(void);
extern void tlb_ifetch_entry_end(void);
extern void tlb_read_inhibit_entry_begin(void);
extern void tlb_read_inhibit_entry_end(void);
extern void tlb_exe_inhibit_entry_begin(void);
extern void tlb_exe_inhibit_entry_end(void);
extern void tlb_privilege_err_entry_begin(void);
extern void tlb_privilege_err_entry_end(void);
extern void addr_err_entry_begin(void);
extern void addr_err_entry_end(void);
extern void unaligned_access_entry_begin(void);
extern void unaligned_access_entry_end(void);
extern void syscall_entry_begin(void);
extern void syscall_entry_end(void);
extern void ine_entry_begin(void);
extern void ine_entry_end(void);

#define HAMT_MTTCG_ENTRY_DEF(n)                         \
extern void tlb_load_entry_s ## n ## _begin(void);      \
extern void tlb_load_entry_s ## n ## _end(void);        \
extern void tlb_store_entry_s ## n ## _begin(void);     \
extern void tlb_store_entry_s ## n ## _end(void);       \
extern void tlb_modified_entry_s ## n ## _begin(void);  \
extern void tlb_modified_entry_s ## n ## _end(void);    \
extern void break_entry_s ## n ## _begin(void);         \
extern void break_entry_s ## n ## _end(void);

HAMT_MTTCG_ENTRY_DEF(0)
HAMT_MTTCG_ENTRY_DEF(1)
HAMT_MTTCG_ENTRY_DEF(2)
HAMT_MTTCG_ENTRY_DEF(3)

#define HAMT_MTTCG_INIT_EBASE(cpuid, val) do {          \
if (cpuid == val) {                                     \
    void * st = NULL;                                   \
    void * ed = NULL;                                   \
    /* tlb load exception entry */                      \
    st = tlb_load_entry_s ## val ## _begin;             \
    ed = tlb_load_entry_s ## val ## _end;               \
    pr_info("cpu %d tlb load  : 0x%llx", cpuid, st);    \
    assert((ed - st) < 512);                            \
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBL,   \
            st, ed - st);                               \
    latx_perfmap_insert( \
        cpu->info.ebase + VEC_SIZE * EXCCODE_TLBL, \
        ed - st, \
        "hamt_tlb_load_entry_" #cpuid ); \
    /* tlb store exception entry */                     \
    st = tlb_store_entry_s ## val ## _begin;            \
    ed = tlb_store_entry_s ## val ## _end;              \
    pr_info("cpu %d tlb store : 0x%llx", cpuid, st);    \
    assert((ed - st) < 512);                            \
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBS,   \
            st, ed - st);                               \
    latx_perfmap_insert( \
        cpu->info.ebase + VEC_SIZE * EXCCODE_TLBS, \
        ed - st, \
        "hamt_tlb_store_entry_" #cpuid ); \
    /* tlb modify exception entry */                    \
    st = tlb_modified_entry_s ## val ## _begin;         \
    ed = tlb_modified_entry_s ## val ## _end;           \
    pr_info("cpu %d tlb modify: 0x%llx", cpuid, st);    \
    assert((ed - st) < 512);                            \
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBM,   \
            st, ed - st);                               \
    latx_perfmap_insert( \
        cpu->info.ebase + VEC_SIZE * EXCCODE_TLBM, \
        ed - st, \
        "hamt_tlb_modify_entry_" #cpuid ); \
    /* break entry */                                   \
    st = break_entry_s ## val ## _begin;                \
    ed = break_entry_s ## val ## _end;                  \
    pr_info("cpu %d break     : 0x%llx", cpuid, st);    \
    assert((ed - st) < 512);                            \
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_BP,     \
            st, ed - st);                               \
    latx_perfmap_insert( \
        cpu->info.ebase + VEC_SIZE * EXCCODE_BP, \
        ed - st, \
        "hamt_tlb_break_entry_" #cpuid ); \
} } while (0)

static void init_ebase(struct kvm_cpu *cpu,
        int mode, int cpuid)
{
    BUILD_ASSERT(512 == VEC_SIZE);
    BUILD_ASSERT(INT_OFFSET * VEC_SIZE == PAGESIZE * 2);
    BUILD_ASSERT(VEC_SIZE * 14 < PAGESIZE);

    cpu->info.ebase = mmap_pages(4);
    pr_info("cpu %d mode %d ebase address : %llx",
            cpuid, mode, cpu->info.ebase);
    for (int i = 0; i < PAGESIZE; ++i) {
        int *x = (int *)cpu->info.ebase;
        x = x + i;
        *x = (0x002b8000 | INVALID_EBASE_POSITION);
    }

    /* multi vcpu thread */
    if (mode == HAMT_MODE_MT) {
        HAMT_MTTCG_INIT_EBASE(cpuid, 0);
        HAMT_MTTCG_INIT_EBASE(cpuid, 1);
        HAMT_MTTCG_INIT_EBASE(cpuid, 2);
        HAMT_MTTCG_INIT_EBASE(cpuid, 3);
        assert(!hamt_have_tlbr_fastpath());
    }

    // tlb refill exception
    if (hamt_have_tlbr_ultra_fastpath()) {
        hamt_spt_assert_ultra_fastpath();
        assert((tlb_ultra_fast_refill_entry_end - tlb_ultra_fast_refill_entry_begin) < 512);
        memcpy(cpu->info.ebase, tlb_ultra_fast_refill_entry_begin,
               tlb_ultra_fast_refill_entry_end - tlb_ultra_fast_refill_entry_begin);
        latx_perfmap_insert(
                cpu->info.ebase,
                tlb_ultra_fast_refill_entry_end - tlb_ultra_fast_refill_entry_begin,
                "hamt_tlb_ultra_fast_refill_entry");
    } else if (hamt_have_tlbr_fastpath()) {
        assert((tlb_fast_refill_entry_end - tlb_fast_refill_entry_begin) < 512);
        memcpy(cpu->info.ebase, tlb_fast_refill_entry_begin,
               tlb_fast_refill_entry_end - tlb_fast_refill_entry_begin);
        latx_perfmap_insert(
                cpu->info.ebase,
                tlb_fast_refill_entry_end - tlb_fast_refill_entry_begin,
                "hamt_tlb_fast_refill_entry");
    } else {
        assert((tlb_refill_entry_end - tlb_refill_entry_begin) < 512);
        memcpy(cpu->info.ebase, tlb_refill_entry_begin,
               tlb_refill_entry_end - tlb_refill_entry_begin);
        latx_perfmap_insert(
                cpu->info.ebase,
                tlb_refill_entry_end - tlb_refill_entry_begin,
                "hamt_tlb_refill_entry");
    }

    // EXCCODE_TLBL 1
    if (mode == HAMT_MODE_RR) { /* single vcpu thread */
        assert((tlb_load_entry_s0_end - tlb_load_entry_s0_begin) < 512);
        memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBL,
                tlb_load_entry_s0_begin,
                tlb_load_entry_s0_end - tlb_load_entry_s0_begin);
        latx_perfmap_insert(
                cpu->info.ebase + VEC_SIZE * EXCCODE_TLBL,
                tlb_load_entry_s0_end - tlb_load_entry_s0_begin,
                "hamt_tlb_load_entry");
    }

    // EXCCODE_TLBS 2
    if (mode == HAMT_MODE_RR) { /* single vcpu thread  */
        assert((tlb_store_entry_s0_end - tlb_store_entry_s0_begin) < 512);
        memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBS,
                tlb_store_entry_s0_begin,
                tlb_store_entry_s0_end - tlb_store_entry_s0_begin);
        latx_perfmap_insert(
                cpu->info.ebase + VEC_SIZE * EXCCODE_TLBS,
                tlb_store_entry_s0_end - tlb_store_entry_s0_begin,
                "hamt_tlb_store_entry");
    }

    // EXCCODE_TLBI 3
    assert((tlb_ifetch_entry_end - tlb_ifetch_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBI, tlb_ifetch_entry_begin,
            tlb_ifetch_entry_end - tlb_ifetch_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_TLBI,
            tlb_ifetch_entry_end - tlb_ifetch_entry_begin,
            "hamt_tlb_ifetch_entry");

    // EXCCODE_TLBM 4
    if (mode == HAMT_MODE_RR) { /* single vcpu thread  */
        assert((tlb_modified_entry_s0_end - tlb_modified_entry_s0_begin) < 512);
        memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBM,
                tlb_modified_entry_s0_begin,
                tlb_modified_entry_s0_end - tlb_modified_entry_s0_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_TLBM,
            tlb_modified_entry_s0_end - tlb_modified_entry_s0_begin,
            "hamt_tlb_modify_entry");
    }

    // EXCCODE_TLBRI 5
    assert((tlb_read_inhibit_entry_end - tlb_read_inhibit_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBRI, tlb_read_inhibit_entry_begin,
            tlb_read_inhibit_entry_end - tlb_read_inhibit_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_TLBRI,
            tlb_read_inhibit_entry_end - tlb_read_inhibit_entry_begin,
            "hamt_tlb_read_inhibit_entry");

    // EXCCODE_TLBXI 6
    assert((tlb_exe_inhibit_entry_end - tlb_exe_inhibit_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBXI, tlb_exe_inhibit_entry_begin,
            tlb_exe_inhibit_entry_end - tlb_exe_inhibit_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_TLBXI,
            tlb_exe_inhibit_entry_end - tlb_exe_inhibit_entry_begin,
            "hamt_tlb_exec_inhibit_entry");

    // EXCCODE_TLBPE 7
    assert((tlb_privilege_err_entry_end - tlb_privilege_err_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_TLBPE, tlb_privilege_err_entry_begin,
            tlb_privilege_err_entry_end - tlb_privilege_err_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_TLBPE,
            tlb_privilege_err_entry_end - tlb_privilege_err_entry_begin,
            "hamt_tlb_privilege_err_entry");

    // EXCCODE_ADE 8
    assert((addr_err_entry_end - addr_err_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_ADE, addr_err_entry_begin,
            addr_err_entry_end - addr_err_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_ADE,
            addr_err_entry_end - addr_err_entry_begin,
            "hamt_addr_err_entry");

    // EXCCODE_ALE 9
    assert((unaligned_access_entry_end - unaligned_access_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_ALE, unaligned_access_entry_begin,
            unaligned_access_entry_end - unaligned_access_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_ALE,
            unaligned_access_entry_end - unaligned_access_entry_begin,
            "hamt_unaligned_access_entry");

    // EXCCODE_SYS 11
    assert((syscall_entry_end - syscall_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_SYS, syscall_entry_begin,
           syscall_entry_end - syscall_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_SYS,
            syscall_entry_end - syscall_entry_begin,
            "hamt_syscall_entry");

    // EXCCODE_BP 12 
    if (mode == HAMT_MODE_RR) { /* single vcpu thread  */
        assert((break_entry_s0_end - break_entry_s0_begin) < 512);
        memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_BP,
                break_entry_s0_begin,
                break_entry_s0_end - break_entry_s0_begin);
        latx_perfmap_insert(
                cpu->info.ebase + VEC_SIZE * EXCCODE_BP,
                break_entry_s0_end - break_entry_s0_begin,
                "hamt_break_entry");
    }

    // EXCCODE_INE 13
    assert((ine_entry_end - ine_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE * EXCCODE_INE, ine_entry_begin,
            ine_entry_end - ine_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE * EXCCODE_INE,
            ine_entry_end - ine_entry_begin,
            "hamt_ine_entry");

    //EXCCODE_FPE 18
    assert((fpe_entry_end - fpe_entry_begin) < 512);
    memcpy(cpu->info.ebase + VEC_SIZE*18, fpe_entry_begin, 
           fpe_entry_end - fpe_entry_begin);
    latx_perfmap_insert(
            cpu->info.ebase + VEC_SIZE*18,
            fpe_entry_end - fpe_entry_begin,
            "hamt_fpe_entry");

    latx_perfmap_flush();
    // pr_info("ebase address : %llx", cpu->info.ebase);
}

struct csr_reg {
    struct kvm_one_reg reg;
    char name[100];
    u64 v;
};

#define CSR_INIT_REG(X)                                             \
{                                                                   \
    .reg = { .id = KVM_CSR_##X }, .name = #X, .v = INIT_VALUE_##X   \
}

static void init_csr(struct kvm_cpu *cpu)
{
    if (!cpu->info.ebase) {
        die("You forget to init ebase");
    }

    u64 INIT_VALUE_DMWIN1 = CSR_DMW1_INIT;
    u64 INIT_VALUE_KSCRATCH5 = (u64)cpu->syscall_parameter + CSR_DMW1_BASE;
    u64 INIT_VALUE_KSCRATCH6 = TLBRELO0_STANDARD_BITS;
    u64 INIT_VALUE_KSCRATCH7 = TLBRELO1_STANDARD_BITS;

    u64 INIT_VALUE_TLBREBASE = (u64)cpu->info.ebase;
    u64 INIT_VALUE_EBASE = (u64)cpu->info.ebase + CSR_DMW1_BASE;

    // TODO 并没有什么意义
    u64 INIT_VALUE_CPUNUM = cpu->cpu_id;

    struct csr_reg one_regs[] = {
        CSR_INIT_REG(CRMD),
        // CSR_INIT_REG(PRMD),
        CSR_INIT_REG(EUEN), CSR_INIT_REG(MISC), CSR_INIT_REG(ECFG),
        // CSR_INIT_REG(ESTAT),
        // CSR_INIT_REG(EPC),
        // CSR_INIT_REG(BADV),
        // CSR_INIT_REG(BADI),
        CSR_INIT_REG(EBASE),
        // CSR_INIT_REG(TLBIDX),
        // CSR_INIT_REG(TLBHI),
        // CSR_INIT_REG(TLBLO0),
        // CSR_INIT_REG(TLBLO1),
        // CSR_INIT_REG(GTLBC),
        // CSR_INIT_REG(TRGP),
        // CSR_INIT_REG(ASID),
        // CSR_INIT_REG(PGDL),
        // CSR_INIT_REG(PGDH),
        // CSR_INIT_REG(PGD),
        CSR_INIT_REG(PWCTL0), CSR_INIT_REG(PWCTL1),
        CSR_INIT_REG(STLBPS), CSR_INIT_REG(RVACFG),
        CSR_INIT_REG(CPUNUM),
        // CSR_INIT_REG(PRCFG1),
        // CSR_INIT_REG(PRCFG2),
        // CSR_INIT_REG(PRCFG3),
        // CSR_INIT_REG(KSCRATCH0),
        // CSR_INIT_REG(KSCRATCH1),
        // CSR_INIT_REG(KSCRATCH2),
        // CSR_INIT_REG(KSCRATCH3),
        // CSR_INIT_REG(KSCRATCH4),
        CSR_INIT_REG(KSCRATCH5), CSR_INIT_REG(KSCRATCH6),
        CSR_INIT_REG(KSCRATCH7),
        // CSR_INIT_REG(TIMERID), // kvm 会初始化
        // 从 kvm_vz_queue_timer_int_cb 看，disable 掉 TIMERCFG::EN 的确可以不被注入
        // 时钟中断
        CSR_INIT_REG(TIMERCFG),
        // CSR_INIT_REG(TIMERTICK),
        // CSR_INIT_REG(TIMEROFFSET),
        // CSR_INIT_REG(GSTAT), gcsr
        // CSR_INIT_REG(GCFG),  gcsr
        // CSR_INIT_REG(GINTC), gcsr
        // CSR_INIT_REG(GCNTC), gcsr
        // CSR_INIT_REG(LLBCTL), // 没有这个入口
        // CSR_INIT_REG(IMPCTL1),
        // CSR_INIT_REG(IMPCTL2),
        // CSR_INIT_REG(GNMI),
        CSR_INIT_REG(TLBREBASE),
        // CSR_INIT_REG(TLBRBADV),
        // CSR_INIT_REG(TLBREPC),
        // CSR_INIT_REG(TLBRSAVE),
        // CSR_INIT_REG(TLBRELO0),
        // CSR_INIT_REG(TLBRELO1),
        // CSR_INIT_REG(TLBREHI),
        // CSR_INIT_REG(TLBRPRMD),
        // CSR_INIT_REG(ERRCTL),
        // CSR_INIT_REG(ERRINFO1),
        // CSR_INIT_REG(ERRINFO2),
        // CSR_INIT_REG(ERREBASE),
        // CSR_INIT_REG(ERREPC),
        // CSR_INIT_REG(ERRSAVE),
        // CSR_INIT_REG(CTAG),
        // CSR_INIT_REG(MCSR0),
        // CSR_INIT_REG(MCSR1),
        // CSR_INIT_REG(MCSR2),
        // CSR_INIT_REG(MCSR3),
        // CSR_INIT_REG(MCSR8),
        // CSR_INIT_REG(MCSR9),
        // CSR_INIT_REG(MCSR10),
        // CSR_INIT_REG(MCSR24),
        // CSR_INIT_REG(UCWIN),
        // CSR_INIT_REG(UCWIN0_LO),
        // CSR_INIT_REG(UCWIN0_HI),
        // CSR_INIT_REG(UCWIN1_LO),
        // CSR_INIT_REG(UCWIN1_HI),
        // CSR_INIT_REG(UCWIN2_LO),
        // CSR_INIT_REG(UCWIN2_HI),
        // CSR_INIT_REG(UCWIN3_LO),
        // CSR_INIT_REG(UCWIN3_HI),
        // CSR_INIT_REG(DMWIN0),
        CSR_INIT_REG(DMWIN1),
        // CSR_INIT_REG(DMWIN2),
        // CSR_INIT_REG(DMWIN3),
        // FIXME : 没有办法控制 perf 寄存器, 最后会不会导致 perf 其实默认是打开的
        // 等到可以进入到 guest 打开试试
        // CSR_INIT_REG(PERF0_EVENT),
        // CSR_INIT_REG(PERF0_COUNT),
        // CSR_INIT_REG(PERF1_EVENT),
        // CSR_INIT_REG(PERF1_COUNT),
        // CSR_INIT_REG(PERF2_EVENT),
        // CSR_INIT_REG(PERF2_COUNT),
        // CSR_INIT_REG(PERF3_EVENT),
        // CSR_INIT_REG(PERF3_COUNT),
        // CSR_INIT_REG(DEBUG),
        // CSR_INIT_REG(DEPC),
        // CSR_INIT_REG(DESAVE),
    };

    for (int i = 0; i < sizeof(one_regs) / sizeof(struct csr_reg); ++i) {
        one_regs[i].reg.addr = (u64) & (one_regs[i].v);
    }

    for (int i = 0; i < sizeof(one_regs) / sizeof(struct csr_reg); ++i) {
#ifdef USE_KVM_SET_CSR_FUNC
        kvm_set_csr_reg(cpu, one_regs[i].reg.id, one_regs[i].v);
#else
        if (ioctl(cpu->vcpu_fd, KVM_SET_ONE_REG, &(one_regs[i].reg))
                < 0) {
            die("KVM_SET_ONE_REG %s", one_regs[i].name);
        } else {
            // pr_info("KVM_SET_ONE_REG %s : %llx", one_regs[i].name,
            // one_regs[i].v);
        }
#endif
    }
}

static int __attribute__((noinline))
kvm_launch(struct kvm_cpu *cpu, struct kvm_regs *regs,
        int mode, int cpuid)
{
    uint64_t val;
    BUILD_ASSERT(offsetof(struct kvm_regs, pc) == 256);
    asm goto("\n\t"
         "st.d $r0,  $r5, 0\n\t"
         "st.d $r1,  $r5, 8\n\t"
         "st.d $r2,  $r5, 16\n\t"
         "st.d $r3,  $r5, 24\n\t"
         "st.d $r4,  $r5, 32\n\t"
         "st.d $r5,  $r5, 40\n\t"
         "st.d $r6,  $r5, 48\n\t"
         "st.d $r7,  $r5, 56\n\t"
         "st.d $r8,  $r5, 64\n\t"
         "st.d $r9,  $r5, 72\n\t"
         "st.d $r10, $r5, 80\n\t"
         "st.d $r11, $r5, 88\n\t"
         "st.d $r12, $r5, 96\n\t"
         "st.d $r13, $r5, 104\n\t"
         "st.d $r14, $r5, 112\n\t"
         "st.d $r15, $r5, 120\n\t"
         "st.d $r16, $r5, 128\n\t"
         "st.d $r17, $r5, 136\n\t"
         "st.d $r18, $r5, 144\n\t"
         "st.d $r19, $r5, 152\n\t"
         "st.d $r20, $r5, 160\n\t"
         "st.d $r21, $r5, 168\n\t"
         "st.d $r22, $r5, 176\n\t"
         "st.d $r23, $r5, 184\n\t"
         "st.d $r24, $r5, 192\n\t"
         "st.d $r25, $r5, 200\n\t"
         "st.d $r26, $r5, 208\n\t"
         "st.d $r27, $r5, 216\n\t"
         "st.d $r28, $r5, 224\n\t"
         "st.d $r29, $r5, 232\n\t"
         "st.d $r30, $r5, 240\n\t"
         "st.d $r31, $r5, 248\n\t"

         "la.local $r6, %l[guest_entry]\n\t"
         "st.d $r6, $r5, 256\n\t"
         "ld.d $r6, $r5, 48\n\t" // restore $6
         :
         :
         : "memory"
         : guest_entry);

    arch_dump_regs(STDOUT_FILENO, *regs);

    init_ebase(cpu, mode, cpuid);
    init_csr(cpu);
    init_fpu(cpu);

    /*
     * Currently, kernel lbt lazy enabling mechanism is not 100% functional:
     * a process's euen lbt status bit may reset out of no reason, 
     * causing guest executing lbt instructions fall back to host and shut down.
     *
     * Temporarily, kernel simply enable lbt all the time.
     *
     * When the time come and lazy enabling machanism is back, following instructions
     * should be uncommented to enable qemu lbt bit before entring guest
     */
    /*
    __asm__ __volatile__ (
        "addi.d    $sp, $sp, -8\n"
        "st.d      $t0, $sp, 0\n"
        "x86mfflag $t0, 0xff\n"
        "ld.d      $t0, $sp, 0\n"
        "addi.d    $sp, $sp, 8\n"
        :
    );
    */

    if (ioctl(cpu->vcpu_fd, KVM_SET_REGS, regs) < 0) {
        die("KVM_SET_REGS failed");
    }

    vacate_current_stack(cpu);
    die("host never reach here\n");
guest_entry:
    // return expression is needed by guest_entry, otherwise gcc inline asm would
    // complaint
    __asm__ __volatile__("csrrd %0, 0x80 \n\t" : "=r"(val));
    printf("CSR[0x80]: 0x%lx Bit[8]=%d\n", val, (int)((val >> 8) & 0x1));
    __asm__ __volatile__("li.d $t1, 0x0 \n\t"
                   "ori $t0, $zero, %0 \n\t"
                   "csrxchg $t1, $t0, %1 \n\t"
                   :
                   : "i"(0x100), "i"(0x80)
                   : "$t0", "$t1");
    __asm__ __volatile__("csrrd %0, 0x80 \n\t" : "=r"(val));
    printf("CSR[0x80]: 0x%lx Bit[8]=%d\n", val, (int)((val >> 8) & 0x1));
    return 0;
}

void arch_dune_enter(struct kvm_cpu *cpu,
        int mode, int cpuid)
{
    pr_info("%s cpu %d mode %d", __func__, cpuid, mode);
    struct kvm_regs regs;
    BUILD_ASSERT(256 == offsetof(struct kvm_regs, pc));
    kvm_launch(cpu, &regs, mode, cpuid);
}

// a7($r11) 是作为 syscall number
u64 arch_get_sysno(const struct kvm_cpu *cpu)
{
    return cpu->syscall_parameter[7];
}

enum CLONE_TYPE arch_get_clone_type(const struct kvm_cpu *parent_cpu, int sysno)
{
    // If CLONE_VM is set, the calling process and the child process run in the same memory  space.
    if (sysno != SYS_CLONE) {
        die("impossible sysno");
    }

    if (parent_cpu->syscall_parameter[0] & CLONE_VM) {
        return SAME_VM;
    }

    if (parent_cpu->syscall_parameter[1] != 0) {
        return DIFF_VM_NEW_STACK;
    }

    return DIFF_VM_OLD_STACk;
}

#define __SYSCALL_CLOBBERS                                                     \
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "memory"

bool arch_do_syscall(struct kvm_cpu *cpu, bool is_fork)
{
    register long int __a7 asm("$a7") = cpu->syscall_parameter[7];
    register long int __a0 asm("$a0") = cpu->syscall_parameter[0];
    register long int __a1 asm("$a1") = cpu->syscall_parameter[1];
    register long int __a2 asm("$a2") = cpu->syscall_parameter[2];
    register long int __a3 asm("$a3") = cpu->syscall_parameter[3];
    register long int __a4 asm("$a4") = cpu->syscall_parameter[4];
    register long int __a5 asm("$a5") = cpu->syscall_parameter[5];
    register long int __a6 asm("$a6") = cpu->syscall_parameter[6];

    __asm__ volatile("syscall    0\n\t"
             : "+r"(__a0)
             : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3),
               "r"(__a4), "r"(__a5), "r"(__a6)
             : __SYSCALL_CLOBBERS);

    if (is_fork && __a0 == 0) {
        return true;
    }

    cpu->syscall_parameter[0] = __a0;
    return false;
}

void init_child_thread_info(struct kvm_cpu *child_cpu,
                const struct kvm_cpu *parent_cpu, int sysno)
{
    struct kvm_regs child_regs;
    ebase_share(child_cpu, parent_cpu);

    memcpy(&child_regs, &parent_cpu->info.regs, sizeof(struct kvm_regs));

    // #define v0 $r4
    child_regs.gpr[4] = 0;
    // see arch/loongarch/kvm/loongisa.c:kvm_arch_vcpu_ioctl_run
    child_cpu->kvm_run->hypercall.ret = child_regs.gpr[4];

    if (parent_cpu->syscall_parameter[0] & CLONE_SETTLS) {
        child_regs.gpr[2] = parent_cpu->syscall_parameter[4];
    }

    if (sysno == SYS_CLONE) {
        // see linux kernel fork.c:copy_thread
        // #define sp $r3
        if (parent_cpu->syscall_parameter[1] != 0)
            child_regs.gpr[3] = parent_cpu->syscall_parameter[1];
    } else if (sysno == SYS_CLONE3) {
        die("No support for clone3");
    }

    child_regs.pc = parent_cpu->info.era + 4;

    if (ioctl(child_cpu->vcpu_fd, KVM_SET_REGS, &child_regs) < 0)
        die("KVM_SET_REGS");

    dup_fpu(child_cpu, &parent_cpu->info.fpu);
    init_csr(child_cpu);
}

void arch_set_thread_area(struct kvm_cpu *vcpu)
{
    // loongarch 上没有 SYS_SET_THREAD_AREA
}

// 应该没有特殊的 syscall 需要处理
bool arch_handle_special_syscall(struct kvm_cpu *vcpu, u64 sysno)
{
    return false;
}

void escape(void)
{
    die("unimp");
}

u64 __do_simulate_clone(u64, u64, u64, u64, u64);

void do_simulate_clone(struct kvm_cpu *parent_cpu, u64 child_host_stack)
{
    u64 arg0 = parent_cpu->syscall_parameter[0];
    // u64 a1 = parent_cpu->syscall_parameter[1];
    u64 arg2 = parent_cpu->syscall_parameter[2];
    u64 arg3 = parent_cpu->syscall_parameter[3];
    u64 arg4 = parent_cpu->syscall_parameter[4];

    // parent 原路返回，child 进入到 child_entry 中间
    long child_pid =
        __do_simulate_clone(arg0, child_host_stack, arg2, arg3, arg4);

    if (child_pid > 0) {
        parent_cpu->syscall_parameter[0] = child_pid;
    } else {
        parent_cpu->syscall_parameter[0] = -child_pid;
    }
}
