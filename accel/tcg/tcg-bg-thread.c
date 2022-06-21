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
#include "tcg-bg-thread.h"

//#define BG_THREAD_DEBUG

static QemuCond   *tcg_bg_thread_cond;
static QemuMutex  *tcg_bg_thread_mutex;
static int tcg_bg_thread_todo;
static int tcg_bg_thread_goon;

/* ================== BG thread Work list ======================= */
static QemuMutex  *tcg_bg_work_mutex;
struct tcg_bg_work_item {
    QSIMPLEQ_ENTRY(tcg_bg_work_item) node;
    void (*jc_func)(int data);
    int jc_id;
};
static QSIMPLEQ_HEAD(, tcg_bg_work_item) tcg_bg_work_list;

static void tcg_bg_worker_jc_clear(int jc_id);

static struct tcg_bg_work_item *tcg_bg_worker_jc_build(int jc_id)
{
    struct tcg_bg_work_item *wi;

    wi = g_malloc0(sizeof(struct tcg_bg_work_item));
    wi->jc_func = tcg_bg_worker_jc_clear;
    wi->jc_id = jc_id;

#ifdef BG_THREAD_DEBUG
    fprintf(stderr, "[BG] worker: create %d.\n", id);
#endif

    return wi;
}

/* ================== BG thread TB Jmp Cache ======================= */
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

#ifdef BG_THREAD_DEBUG
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

#ifdef BG_THREAD_DEBUG
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

#ifdef BG_THREAD_DEBUG
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
#ifdef BG_THREAD_DEBUG
    fprintf(stderr, "[vCPU] clear: jmp cache.\n");
#endif

    qemu_mutex_lock(tcg_bg_jc_mutex);
    int free_id = tcg_bg_jc_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_jc_mutex);

    if (free_id >= 0) {
        int old_id = cpu->tcg_bg_jc_id;
#ifdef BG_THREAD_DEBUG
    fprintf(stderr, "[vCPU] clear: send %d to bg thread and get %d.\n",
            old_id, free_id);
#endif

        /* build worker for bg thread */
        struct tcg_bg_work_item *wi =
            tcg_bg_worker_jc_build(old_id);

        /* insert worker into bg work list */
        qemu_mutex_lock(tcg_bg_work_mutex);
        QSIMPLEQ_INSERT_TAIL(&tcg_bg_work_list, wi, node);
        qemu_mutex_unlock(tcg_bg_work_mutex);

        /* set jmp cache to a new one */
        qatomic_set(&cpu->tcg_bg_jc, tcg_bg_jc[free_id]);
        qatomic_set(&cpu->tcg_bg_jc_id, free_id);
        env->tb_jmp_cache_ptr = tcg_bg_jc[free_id];

        /* signal bg thread */
        qemu_mutex_lock(tcg_bg_thread_mutex);
        tcg_bg_thread_todo = 1;
        qemu_cond_signal(tcg_bg_thread_cond);
        qemu_mutex_unlock(tcg_bg_thread_mutex);
    } else {
#ifdef BG_THREAD_DEBUG
    fprintf(stderr, "[vCPU] clear: by vCPU-self.\n");
#endif
        unsigned int i;
        for (i = 0; i < TB_JMP_CACHE_SIZE; i++) {
            qatomic_set(&cpu->tcg_bg_jc[i], NULL);
        }
    }
}

static int tcg_bg_init_cpu_jc(void *_cpu, int check)
{
    CPUState *cpu = _cpu;
    int free_id = -1;

    qemu_mutex_lock(tcg_bg_jc_mutex);
    free_id = tcg_bg_jc_get_freeid_locked();
    qemu_mutex_unlock(tcg_bg_jc_mutex);

    if (free_id >= 0) {
        cpu->tcg_bg_jc = tcg_bg_jc[free_id];
        cpu->tcg_bg_jc_id = free_id;
    } else {
        assert(!check);
    }

    return free_id;
}

static void tcg_bg_init_cpu_rr(void *cpu, int check)
{
    int ret = tcg_bg_init_cpu_jc(cpu, check);
    if (ret < 0) {
        fprintf(stderr, "%s fail\n", __func__);
        assert(0);
    }
}

/* ================== BG thread Main ======================= */
static QemuThread *tcg_bg_thread;

static void tcg_bg_work_processing(void)
{
    struct tcg_bg_work_item *wi;

    qemu_mutex_lock(tcg_bg_work_mutex);
    if (QSIMPLEQ_EMPTY(&tcg_bg_work_list)) {
        qemu_mutex_unlock(tcg_bg_work_mutex);
        return;
    }
    while (!QSIMPLEQ_EMPTY(&tcg_bg_work_list)) {
        wi = QSIMPLEQ_FIRST(&tcg_bg_work_list);
        QSIMPLEQ_REMOVE_HEAD(&tcg_bg_work_list, node);
        qemu_mutex_unlock(tcg_bg_work_mutex);

        if (wi->jc_func) {
            wi->jc_func(wi->jc_id);
        }

        qemu_mutex_lock(tcg_bg_work_mutex);
    }
    qemu_mutex_unlock(tcg_bg_work_mutex);
}

static void *tcg_bg_thread_rr_func(void *arg)
{
    while (tcg_bg_thread_goon) {
        qemu_mutex_lock(tcg_bg_thread_mutex);
        while (!tcg_bg_thread_todo) {
            qemu_cond_wait(tcg_bg_thread_cond, tcg_bg_thread_mutex);
        }
        tcg_bg_thread_todo = 0;
        qemu_mutex_unlock(tcg_bg_thread_mutex);

        tcg_bg_work_processing();
    }

    return NULL;
}

static void tcg_bg_init_thread(void)
{
    tcg_bg_thread_cond   = g_malloc0(sizeof(QemuCond));
    tcg_bg_thread_mutex  = g_malloc0(sizeof(QemuMutex));
    qemu_cond_init(tcg_bg_thread_cond);
    qemu_mutex_init(tcg_bg_thread_mutex);

    tcg_bg_thread = g_malloc0(sizeof(QemuThread));
    qemu_thread_create(tcg_bg_thread, "Background vCPU thread",
            tcg_bg_thread_rr_func,
            NULL, QEMU_THREAD_JOINABLE);
}

static void tcg_bg_init_work(void)
{
    QSIMPLEQ_INIT(&tcg_bg_work_list);
    tcg_bg_work_mutex = g_malloc0(sizeof(QemuMutex));
    qemu_mutex_init(tcg_bg_work_mutex);
}

void tcg_bg_init_rr(CPUState *cpu)
{
    if (!tcg_bg_thread) {

        cpu->tcg_bg_jc_clear = tcg_bg_func_jc_clear;
        tcg_bg_jc_mutex = g_malloc0(sizeof(QemuMutex));
        qemu_mutex_init(tcg_bg_jc_mutex);

        tcg_bg_init_work();
        tcg_bg_init_thread();

        tcg_bg_init_cpu_rr(cpu, 1);
    } else {
        cpu->tcg_bg_jc_clear = tcg_bg_func_jc_clear;
        tcg_bg_init_cpu_rr(cpu, 1);
    }
}

static void __attribute__((__constructor__)) tcg_bg_init_static(void)
{
    int i = 0;
    for (; i < TCG_BG_JC_MAX; ++i) {
        tcg_bg_jc_free_ids[i] = i;
    }
    tcg_bg_jc_free_ids[i - 1] = -1;

    tcg_bg_jc_free_head = 0;
    tcg_bg_jc_free_tail = TCG_BG_JC_MAX - 1;
    tcg_bg_jc_free_num  = TCG_BG_JC_MAX - 1;

    tcg_bg_thread_goon  = 1;
    tcg_bg_thread_todo = 0; /* no work to do yet */
}
