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
#include "tcg/tcg-bg-tlb.h"
#include "latx-counter-sys.h"

/* ================== BG thread Work list ======================= */

#define TCG_BG_WORK_TYPE_INT    1
#define TCG_BG_WORK_TYPE_VOID   2
#define TCG_BG_WORK_TYPE_PTR    3
struct tcg_bg_work_int {
    void (*func)(int data);
    int data;
};
struct tcg_bg_work_void {
    void (*func)(void);
};
struct tcg_bg_work_ptr {
    void (*func)(void *data);
    void *data;
};
struct tcg_bg_work_item {
    QSIMPLEQ_ENTRY(tcg_bg_work_item) node;
    int type;
    union {
        struct tcg_bg_work_int  fn_i;
        struct tcg_bg_work_void fn_v;
        struct tcg_bg_work_ptr  fn_p;
    } work;
};

static QemuMutex  *tcg_bg_work_mutex;
static QSIMPLEQ_HEAD(, tcg_bg_work_item) tcg_bg_work_list;

static struct tcg_bg_work_item *tcg_bg_worker_build_int(void *func, int data)
{
    struct tcg_bg_work_item *wi;
    wi = g_malloc0(sizeof(struct tcg_bg_work_item));
    wi->type = TCG_BG_WORK_TYPE_INT;
    wi->work.fn_i.func = func;
    wi->work.fn_i.data = data;
    return wi;
}

static void tcg_bg_work_run(struct tcg_bg_work_item *wi)
{
    switch (wi->type) {
    case TCG_BG_WORK_TYPE_INT:
        if (wi->work.fn_i.func) {
            wi->work.fn_i.func(wi->work.fn_i.data);
        }
        break;
    case TCG_BG_WORK_TYPE_VOID:
        if (wi->work.fn_v.func) {
            wi->work.fn_v.func();
        }
        break;
    case TCG_BG_WORK_TYPE_PTR:
        if (wi->work.fn_p.func) {
            wi->work.fn_p.func(wi->work.fn_p.data);
        }
        break;
    default:
        break;
    }
}

/* ================== BG thread Main ======================= */

static QemuCond   *tcg_bg_thread_cond;
static QemuMutex  *tcg_bg_thread_mutex;
static QemuThread *tcg_bg_thread;

static int tcg_bg_thread_todo;
static int tcg_bg_thread_goon;

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

        tcg_bg_work_run(wi);

        g_free(wi);

        qemu_mutex_lock(tcg_bg_work_mutex);
    }
    qemu_mutex_unlock(tcg_bg_work_mutex);
}

static void *tcg_bg_thread_func(void *arg)
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
    char bg_thread_name[64];
    sprintf(bg_thread_name, "BG thread");
    /*qemu_thread_create(tcg_bg_thread, "Background vCPU thread",*/
    qemu_thread_create(tcg_bg_thread, bg_thread_name,
            tcg_bg_thread_func,
            NULL, QEMU_THREAD_JOINABLE);
}

static void tcg_bg_init_work_list(void)
{
    QSIMPLEQ_INIT(&tcg_bg_work_list);
    tcg_bg_work_mutex = g_malloc0(sizeof(QemuMutex));
    qemu_mutex_init(tcg_bg_work_mutex);
}

static void __tcg_bg_init(CPUState *cpu)
{
    if (!tcg_bg_thread) {
        tcg_bg_init_work_list();
        tcg_bg_init_thread();
    }

    tcg_bg_init_jc(cpu);
}
void tcg_bg_init_rr(CPUState *cpu)
{
    __tcg_bg_init(cpu);
}

void tcg_bg_init_mt(CPUState *cpu)
{
    __tcg_bg_init(cpu);
}

static void __attribute__((__constructor__)) tcg_bg_init_static(void)
{
    tcg_bg_jc_init_static();
    tcg_bg_tlb_init_static();

    tcg_bg_thread_goon  = 1;
    tcg_bg_thread_todo = 0; /* no work to do yet */
}

/* ================== BG thread Wake ======================= */

static void __tcg_bg_insert_worker(struct tcg_bg_work_item *wi)
{
    qemu_mutex_lock(tcg_bg_work_mutex);
    QSIMPLEQ_INSERT_TAIL(&tcg_bg_work_list, wi, node);
    qemu_mutex_unlock(tcg_bg_work_mutex);
}

static void __tcg_bg_wake_up(void)
{
    qemu_mutex_lock(tcg_bg_thread_mutex);
    tcg_bg_thread_todo = 1;
    qemu_cond_signal(tcg_bg_thread_cond);
    qemu_mutex_unlock(tcg_bg_thread_mutex);
}

static void tcg_bg_wake_int(void *func, int data)
{
    /* build worker for bg thread */
    struct tcg_bg_work_item *wi = tcg_bg_worker_build_int(func, data);

    /* insert worker into bg work list */
    __tcg_bg_insert_worker(wi);

    /* signal bg thread */
    __tcg_bg_wake_up();
}

/* ================== BG thread TB Jmp Cache ======================= */

void tcg_bg_jc_wake(void *func, int jc_id)
{
    tcg_bg_wake_int(func, jc_id);
}

/* ================== BG thread CPU TLB ======================= */

void tcg_bg_tlb_wake(void *func, int tlb_id)
{
    tcg_bg_wake_int(func, tlb_id);
}

/* ================ BG thread log file ========================== */

static const char *bglogfilename;
QemuLogFile *qemu_bglogfile;

void qemu_set_bglog_filename(const char *filename)
{
    QemuLogFile *logfile;

    bglogfilename = filename;
    logfile = g_new0(QemuLogFile, 1);

    if (bglogfilename) {
        logfile->fd = fopen(bglogfilename, "w");
        if (!logfile->fd) {
            fprintf(stderr, "bg log open fail. output to stderr.\n");
            logfile->fd = stderr;
        }
    } else {
        logfile->fd = stderr;
    }

    qemu_bglogfile = logfile;
    fprintf(stderr, "BG thread log file %s\n", filename);
}

int qemu_bglog(const char *fmt, ...)
{
    int ret = 0;
    if (qemu_bglogfile) {
        va_list ap;
        va_start(ap, fmt);
        ret = vfprintf(qemu_bglogfile->fd, fmt, ap);
        va_end(ap);

        /* Don't pass back error results.  */
        if (ret < 0) {
            ret = 0;
        }
    }
    return ret;
}

void qemu_bglog_flush(void)
{
    if (qemu_bglogfile) {
        fflush(qemu_bglogfile->fd);
    }
}

/* ================== BG thread Counter ======================= */

void tcg_bg_counter_wake(void *func, int sec)
{
    if (!qemu_tcg_bg_enabled()) {
        fprintf(stderr, "Warning: %s bg thead NOT enable in\n", __func__);
        return;
    }

    tcg_bg_wake_int(func, sec);
}
