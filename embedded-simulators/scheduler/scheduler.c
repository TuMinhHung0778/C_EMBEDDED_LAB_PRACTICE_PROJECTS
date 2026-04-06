#include "scheduler.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


/* Welford's online variance for jitter (ms) of start time vs ideal grid. */
typedef struct {
    double mean;
    double m2;
    uint64_t n;
} JitterAcc;

static JitterAcc g_jitter[3];

static void jitter_reset(void)
{
    memset(g_jitter, 0, sizeof(g_jitter));
}

static void jitter_push(unsigned task_index, double sample_ms)
{
    JitterAcc *a = &g_jitter[task_index];
    a->n++;
    const double delta = sample_ms - a->mean;
    a->mean += delta / (double)a->n;
    const double delta2 = sample_ms - a->mean;
    a->m2 += delta * delta2;
}

static double jitter_stddev_ms(unsigned task_index)
{
    const JitterAcc *a = &g_jitter[task_index];
    if (a->n == 0U) {
        return 0.0;
    }
    if (a->n == 1U) {
        return 0.0;
    }
    /* Population σ of start-time error vs ideal grid (ms). */
    return sqrt(a->m2 / (double)a->n);
}

/** Population variance of start-time error (ms²), per practice.md jitter metric. */
static double jitter_variance_ms2(unsigned task_index)
{
    const JitterAcc *a = &g_jitter[task_index];
    if (a->n == 0U) {
        return 0.0;
    }
    return a->m2 / (double)a->n;
}

void scheduler_init(Scheduler *s)
{
    assert(s != NULL);
    jitter_reset();
    s->current_tick = 0U;
    s->total_cycles = 0U;

    s->tasks[0] = (Task){
        .name = "Task A",
        .period_ms = 10U,
        .deadline_ms = 5U,
        .max_duration_ms = 1U,
        .next_run_tick = 0U,
        .exec_count = 0U,
        .missed_deadlines = 0U,
    };
    s->tasks[1] = (Task){
        .name = "Task B",
        .period_ms = 25U,
        .deadline_ms = 12U,
        .max_duration_ms = 2U,
        .next_run_tick = 0U,
        .exec_count = 0U,
        .missed_deadlines = 0U,
    };
    s->tasks[2] = (Task){
        .name = "Task C",
        .period_ms = 50U,
        .deadline_ms = 25U,
        .max_duration_ms = 3U,
        .next_run_tick = 0U,
        .exec_count = 0U,
        .missed_deadlines = 0U,
    };
}

void scheduler_tick(Scheduler *s)
{
    assert(s != NULL);
    s->total_cycles++;
    s->current_tick++;

    /* Virtual time within this 1 ms tick: cooperative tasks run back-to-back. */
    uint32_t vt = s->current_tick;

    for (int i = 0; i < 3; i++) {
        Task *t = &s->tasks[i];
        if (s->current_tick < t->next_run_tick) {
            continue;
        }
        if (t->next_run_tick >= SCHEDULER_SIM_DURATION_TICKS) {
            continue;
        }

        const uint32_t release_tick = t->next_run_tick;
        const uint32_t start_tick = vt;
        const uint32_t complete_tick = start_tick + (uint32_t)t->max_duration_ms;

        /* Relative deadline: must finish by release + deadline_ms. */
        const uint32_t abs_deadline = release_tick + (uint32_t)t->deadline_ms;
        if (complete_tick > abs_deadline) {
            t->missed_deadlines++;
            (void)fprintf(stderr,
                          "[MISS] %s: release=%ums start=%ums complete=%ums deadline=%ums\n",
                          t->name,
                          (unsigned)release_tick,
                          (unsigned)start_tick,
                          (unsigned)complete_tick,
                          (unsigned)abs_deadline);
        }

        const uint32_t ideal_start = (uint32_t)t->exec_count * (uint32_t)t->period_ms;
        const double err_ms = (double)start_tick - (double)ideal_start;
        jitter_push((unsigned)i, err_ms);

        t->exec_count++;
        vt = complete_tick;

        t->next_run_tick = (uint16_t)(t->next_run_tick + t->period_ms);
    }
}

void scheduler_print_report(const Scheduler *s)
{
    assert(s != NULL);

    uint64_t total_exec = 0U;
    uint64_t total_missed = 0U;

    (void)printf("=== Task Scheduler Simulation (%u ms) ===\n", SCHEDULER_SIM_DURATION_TICKS);

    for (int i = 0; i < 3; i++) {
        const Task *t = &s->tasks[i];
        total_exec += (uint64_t)t->exec_count;
        total_missed += (uint64_t)t->missed_deadlines;

        const double sigma_ms = jitter_stddev_ms((unsigned)i);
        const double var_ms2 = jitter_variance_ms2((unsigned)i);
        (void)printf("%s (%ums period, %ums deadline):\n", t->name, t->period_ms, t->deadline_ms);
        (void)printf("  - Executed: %lu times\n", (unsigned long)t->exec_count);
        (void)printf("  - Missed Deadlines: %lu\n", (unsigned long)t->missed_deadlines);
        /* Match embedded-simulator-practice.md example style (±σ ms). */
        (void)printf("  - Jitter: ±%.2fms\n", sigma_ms);
        (void)printf("  - Start-time variance: %.4f ms²\n", var_ms2);
    }

    (void)printf("\nTotal simulation time: %lums\n", (unsigned long)s->current_tick);
    (void)printf("Total cycles completed: %lu\n", (unsigned long)s->total_cycles);

    if (total_missed == 0U) {
        (void)printf("Determinism score: 100%% (all tasks met deadlines)\n");
    } else if (total_exec > 0U) {
        const double pct = 100.0 * (1.0 - (double)total_missed / (double)total_exec);
        (void)printf("Determinism score: %.1f%% (%lu deadline misses / %lu executions)\n",
                      pct,
                      (unsigned long)total_missed,
                      (unsigned long)total_exec);
    } else {
        (void)printf("Determinism score: n/a (no executions)\n");
    }
}
