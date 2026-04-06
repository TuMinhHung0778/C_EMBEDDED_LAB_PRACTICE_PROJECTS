/**
 * Cooperative task scheduler simulator — Project 1 (embedded-simulator-practice.md).
 * Each loop iteration models 1 ms; total horizon SCHEDULER_SIM_DURATION_TICKS.
 */
#include "scheduler.h"

#include <stdint.h>
#include <stdio.h>

int main(void)
{
    Scheduler scheduler;

    scheduler_init(&scheduler);

    for (uint32_t tick = 0U; tick < SCHEDULER_SIM_DURATION_TICKS; tick++) {
        scheduler_tick(&scheduler);
    }

    scheduler_print_report(&scheduler);
    return 0;
}
