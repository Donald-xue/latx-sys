#include "qemu/osdep.h"
#include "qemu-common.h"
#include "sysemu/tcg.h"
#include "sysemu/replay.h"
#include "qemu/main-loop.h"
#include "qemu/guest-random.h"
#include "exec/exec-all.h"
#include "exec/tb-hash.h"

#include "tcg-accel-ops.h"
#include "tcg-accel-ops-rr.h"
#include "tcg-accel-ops-icount.h"
#include "tcg/tcg-bg-thread.h"

#include "tcg/tcg-bg-log.h"
#include "tcg/tcg-bg-jc.h"
#include "latx-counter-sys.h"

#ifdef TCG_BG_JC_CONFIG_SIZE

int tcg_bg_jc_bits;
int tcg_bg_jc_page_bits;
/*
 * eg. JC 12 Page 6
 *                                     11 ... 6
 *                                      |     |
 * page mask = 0000 0000 0000 0000 0000 1111 1100 0000
 * addr mask = 0000 0000 0000 0000 0000 0000 0011 1111
 *                                             |     |
 *                                             5 ... 0
 */
uint64_t tcg_bg_jc_page_mask;
uint64_t tcg_bg_jc_addr_mask;

static inline void tcg_bg_jc_update_mask(void)
{
    tcg_bg_jc_page_mask = (1 << tcg_bg_jc_bits) - (1 << tcg_bg_jc_page_bits);
    tcg_bg_jc_addr_mask = (1 << tcg_bg_jc_page_bits) - 1;
}

uint64_t tcg_bg_jc_size_real;
uint64_t tcg_bg_jc_flag_size_real;

static inline void tcg_bg_jc_update_size(void)
{
    tcg_bg_jc_size_real      = (1 << tcg_bg_jc_bits);
    tcg_bg_jc_flag_size_real = (1 << (tcg_bg_jc_bits - tcg_bg_jc_page_bits));
}

void tcg_bg_jc_init_jmp_cache_bits(int n)
{
    assert(TCG_BG_JC_BITS_MIN <= n &&
           TCG_BG_JC_BITS_MAX >=n);

    tcg_bg_jc_bits = n;
    tcg_bg_jc_update_mask();
    tcg_bg_jc_update_size();
}

void tcg_bg_jc_init_jmp_cache_page_bits(int n)
{
    assert(TCG_BG_JC_PAGE_BITS_MIN <= n &&
           TCG_BG_JC_PAGE_BITS_MAX >= n);

    tcg_bg_jc_page_bits = n;
    tcg_bg_jc_update_mask();
    tcg_bg_jc_update_size();
}

#else

void tcg_bg_jc_init_jmp_cache_bits(int n)
{
    printf("%s use static JC %d\n", __func__, TB_JMP_CACHE_BITS);
}
void tcg_bg_jc_init_jmp_cache_page_bits(int n)
{
    printf("%s use static JC Page %d\n", __func__, TB_JMP_PAGE_BITS);
}

#endif /* TCG_BG_JC_CONFIG_SIZE */

//#define BG_JMP_CACHE_DEBUG

#define BG_JMP_CACHE_ENABLE

/* ================== BG thread TB Jmp Cache ======================= */

#ifdef BG_JMP_CACHE_ENABLE

static void tcg_bg_worker_jc_clear(int jc_id);

#define TCG_BG_JC_MAX     16
#define TCG_BG_JC_MASK    0xF

#ifdef TCG_BG_JC_CONFIG_SIZE
TranslationBlock *tcg_bg_jc[TCG_BG_JC_MAX][TCG_BG_JC_SIZE_MAX];
uint8_t tcg_bg_jc_flag[TCG_BG_JC_MAX][TCG_BG_JC_FLAG_MAX];
#else
TranslationBlock *tcg_bg_jc[TCG_BG_JC_MAX][TB_JMP_CACHE_SIZE];
uint8_t tcg_bg_jc_flag[TCG_BG_JC_MAX][TB_JC_FLAG_SIZE];
#endif

static QemuMutex  *tcg_bg_jc_mutex;
static int tcg_bg_jc_free_ids[TCG_BG_JC_MAX];
static int tcg_bg_jc_free_head;
static int tcg_bg_jc_free_tail;
static int tcg_bg_jc_free_num;

static int tcg_bg_jc_get_freeid_locked(void)
{
    int free_id = -1;
    int free_head = tcg_bg_jc_free_head;
    int free_tail = tcg_bg_jc_free_tail;
    int *free_ids = tcg_bg_jc_free_ids;

    if (free_head != free_tail) {
        assert(tcg_bg_jc_free_num > 0);
        free_id = free_ids[free_head];
        free_ids[free_head] = -1;
        assert(free_id >= 0);

        tcg_bg_jc_free_head += 1;
        tcg_bg_jc_free_head &= 0xF;
        tcg_bg_jc_free_num -= 1;
    } else {
        assert(tcg_bg_jc_free_num == 0);
    }

#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[BG] jc: free %d head %d tail %d return %d.\n",
            tcg_bg_jc_free_num,
            tcg_bg_jc_free_head, tcg_bg_jc_free_tail,
            free_id);
#endif

    return free_id;
}

static void tcg_bg_jc_add_id_locked(int jc_id)
{
    assert(tcg_bg_jc_free_num < (TCG_BG_JC_MAX - 1));
    if (tcg_bg_jc_free_num == 0) {
        assert(tcg_bg_jc_free_head == tcg_bg_jc_free_tail);
    }

    tcg_bg_jc_free_ids[tcg_bg_jc_free_tail] = jc_id;

    tcg_bg_jc_free_tail += 1;
    tcg_bg_jc_free_tail &= 0xF;

    tcg_bg_jc_free_num += 1;

#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[BG] JC: free %d head %d tail %d.\n",
            tcg_bg_jc_free_num,
            tcg_bg_jc_free_head, tcg_bg_jc_free_tail);
#endif
}

void tcg_bg_worker_jc_clear(int id)
{
#ifdef TCG_BG_JC_CONFIG_SIZE
    memset(tcg_bg_jc[id], 0, sizeof(TranslationBlock*) * tcg_bg_jc_size_real);
    memset(tcg_bg_jc_flag[id], 0, sizeof(uint8_t) * tcg_bg_jc_flag_size_real);
#else
    memset(tcg_bg_jc[id], 0, sizeof(TranslationBlock*) * TB_JMP_CACHE_SIZE);
    memset(tcg_bg_jc_flag[id], 0, sizeof(uint8_t) * TB_JC_FLAG_SIZE);
#endif

#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[BG] clear: add new empty %d.\n", id);
#endif
    qemu_mutex_lock(tcg_bg_jc_mutex);
    tcg_bg_jc_add_id_locked(id);
    qemu_mutex_unlock(tcg_bg_jc_mutex);
}

static void tcg_bg_func_jc_clear(void *_cpu)
{
    CPUState *cpu = _cpu;
    CPUX86State *env = cpu->env_ptr;
#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[vCPU][%d] clear: jmp cache.\n",
            cpu->cpu_index);
#endif

    qemu_mutex_lock(tcg_bg_jc_mutex);
    int free_id = tcg_bg_jc_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_jc_mutex);

    if (free_id >= 0) {
        int old_id = cpu->tcg_bg_jc_id;
#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[vCPU][%d] clear: send %d to bg thread and get %d.\n",
            cpu->cpu_index, old_id, free_id);
#endif

        latxs_counter_jc_flush_bg(cpu);

        /* set jmp cache to a new one */
        qatomic_set(&cpu->tcg_bg_jc, tcg_bg_jc[free_id]);
        qatomic_set(&cpu->tb_jc_flag, tcg_bg_jc_flag[free_id]);
        qatomic_set(&cpu->tcg_bg_jc_id, free_id);
        env->tb_jmp_cache_ptr = tcg_bg_jc[free_id];

        /* let bg thread flush the old jmp cache */
        tcg_bg_jc_wake(tcg_bg_worker_jc_clear, old_id);
    } else {
#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[vCPU][%d] clear: by vCPU-self.\n",
            cpu->cpu_index);
#endif
        unsigned int i;
#ifdef TCG_BG_JC_CONFIG_SIZE
        for (i = 0; i < tcg_bg_jc_size_real; i++) {
            qatomic_set(&cpu->tcg_bg_jc[i], NULL);
        }
        memset(cpu->tb_jc_flag, 0, sizeof(uint8_t) * tcg_bg_jc_flag_size_real);
#else
        for (i = 0; i < TB_JMP_CACHE_SIZE; i++) {
            qatomic_set(&cpu->tcg_bg_jc[i], NULL);
        }
        memset(cpu->tb_jc_flag, 0, sizeof(uint8_t) * TB_JC_FLAG_SIZE);
#endif
    }
}

static void tcg_bg_init_jc_clear_func(CPUState *cpu)
{
    cpu->tcg_bg_jc_clear = tcg_bg_func_jc_clear;
}

static int tcg_bg_init_jc_cpu(CPUState *cpu)
{
    int free_id = -1;

    tcg_bg_init_jc_clear_func(cpu);

    if (cpu->tb_jc_flag != NULL) {
        g_free(cpu->tb_jc_flag);
        cpu->tb_jc_flag = NULL;
    }

    qemu_mutex_lock(tcg_bg_jc_mutex);
    free_id = tcg_bg_jc_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_jc_mutex);

    if (free_id >= 0) {
        cpu->tcg_bg_jc = tcg_bg_jc[free_id];
        cpu->tb_jc_flag = tcg_bg_jc_flag[free_id];
        cpu->tcg_bg_jc_id = free_id;
    }

#ifdef TCG_BG_JC_CONFIG_SIZE
    printf("%s JC %d Page %d\n", __func__,
            tcg_bg_jc_bits, tcg_bg_jc_page_bits);
#else
    printf("%s JC %d Page %d\n", __func__,
            TB_JMP_CACHE_BITS, TB_JC_PAGE_BITS);
#endif

    return free_id;
}

void tcg_bg_init_jc(void *cpu)
{
    int ret = tcg_bg_init_jc_cpu((CPUState *)cpu);
    if (ret < 0) {
        fprintf(stderr, "%s fail\n", __func__);
        assert(0);
    }
}

void tcg_bg_jc_init_static(void)
{
    int i = 0;
    for (; i < TCG_BG_JC_MAX; ++i) {
        tcg_bg_jc_free_ids[i] = i;
    }
    tcg_bg_jc_free_ids[i - 1] = -1;

    tcg_bg_jc_free_head = 0;
    tcg_bg_jc_free_tail = TCG_BG_JC_MAX - 1;
    tcg_bg_jc_free_num  = TCG_BG_JC_MAX - 1;

    tcg_bg_jc_mutex = g_malloc0(sizeof(QemuMutex));
    qemu_mutex_init(tcg_bg_jc_mutex);

#ifdef TCG_BG_JC_CONFIG_SIZE
    tcg_bg_jc_bits = 14;
    tcg_bg_jc_page_bits = 6;
    tcg_bg_jc_update_mask();
    tcg_bg_jc_update_size();
#endif
}

#else

void tcg_bg_jc_init_static(void) {}
void tcg_bg_init_jc(void *cpu) {}

#endif
