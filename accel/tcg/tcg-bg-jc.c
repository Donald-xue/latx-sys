#include "qemu/osdep.h"
#include "qemu-common.h"
#include "sysemu/tcg.h"
#include "sysemu/replay.h"
#include "qemu/main-loop.h"
#include "qemu/guest-random.h"
#include "exec/exec-all.h"

#include "tcg-accel-ops.h"
#include "tcg-accel-ops-rr.h"
#include "tcg-accel-ops-icount.h"
#include "tcg/tcg-bg-thread.h"

#include "tcg/tcg-bg-log.h"
#include "tcg/tcg-bg-jc.h"
#include "latx-counter-sys.h"

//#define BG_JMP_CACHE_DEBUG

#define BG_JMP_CACHE_ENABLE

/* ================== BG thread TB Jmp Cache ======================= */

#ifdef BG_JMP_CACHE_ENABLE

static void tcg_bg_worker_jc_clear(int jc_id);

#define TCG_BG_JC_MAX     16
#define TCG_BG_JC_MASK    0xF
TranslationBlock *tcg_bg_jc[TCG_BG_JC_MAX][TB_JMP_CACHE_SIZE];

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
    unsigned int i;
    for (i = 0; i < TB_JMP_CACHE_SIZE; i++) {
        qatomic_set(&tcg_bg_jc[id][i], NULL);
    }

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

        /* set jmp cache to a new one */
        qatomic_set(&cpu->tcg_bg_jc, tcg_bg_jc[free_id]);
        qatomic_set(&cpu->tcg_bg_jc_id, free_id);
        env->tb_jmp_cache_ptr = tcg_bg_jc[free_id];

        /* flush jmp cache flags */
        memset(cpu->tb_jc_flag, 0, TB_JC_FLAG_SIZE);

        /* let bg thread flush the old jmp cache */
        tcg_bg_jc_wake(tcg_bg_worker_jc_clear, old_id);
    } else {
#ifdef BG_JMP_CACHE_DEBUG
    fprintf(stderr, "[vCPU][%d] clear: by vCPU-self.\n",
            cpu->cpu_index);
#endif
        unsigned int i;
        for (i = 0; i < TB_JMP_CACHE_SIZE; i++) {
            qatomic_set(&cpu->tcg_bg_jc[i], NULL);
        }
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

    qemu_mutex_lock(tcg_bg_jc_mutex);
    free_id = tcg_bg_jc_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_jc_mutex);

    if (free_id >= 0) {
        cpu->tcg_bg_jc = tcg_bg_jc[free_id];
        cpu->tcg_bg_jc_id = free_id;
    }

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
}
#else

void tcg_bg_jc_init_static(void) {}
void tcg_bg_init_jc(void *cpu) {}

#endif
