#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

/** Simulated wall time in ms (tick count == ms). */
#define SCHEDULER_SIM_DURATION_TICKS 10000U

typedef struct {
    const char *name;
    uint16_t period_ms;
    uint16_t deadline_ms;
    uint16_t max_duration_ms;
    uint16_t next_run_tick;
    uint32_t exec_count;
    uint32_t missed_deadlines;
} Task;

typedef struct {
    uint32_t current_tick;
    Task tasks[3];
    uint32_t total_cycles;
} Scheduler;

void scheduler_init(Scheduler *s);
void scheduler_tick(Scheduler *s);
void scheduler_print_report(const Scheduler *s);

#endif
