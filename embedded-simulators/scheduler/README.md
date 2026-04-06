# Project 1: Cooperative Task Scheduler Simulator

Simulator for a **non-preemptive (cooperative)** periodic task scheduler: 1 ms tick counter, three tasks with fixed periods/deadlines, deadline checks, and **jitter** statistics (variance / standard deviation of start time vs an ideal period grid).

Spec: [embedded-simulator-practice.md](../../embedded-simulator-practice.md) (Project 1).

## Tasks (WCET simulated)

| Task   | Period | Relative deadline | Simulated `max_duration_ms` |
|--------|--------|-------------------|-----------------------------|
| Task A | 10 ms  | 5 ms              | 1 ms                        |
| Task B | 25 ms  | 12 ms             | 2 ms                        |
| Task C | 50 ms  | 25 ms             | 3 ms                        |

Within each wall-clock tick, tasks run in fixed order **A → B → C**; virtual finish time advances by each task’s `max_duration_ms` so later tasks in the same tick can start late (cooperative queuing).

## Build & run

From `embedded-simulators/`:

```bash
gcc -Wall -Wextra -std=c11 -O2 scheduler/scheduler.c scheduler/main.c -o scheduler_sim -lm
./scheduler_sim
```

Or: `make` (then `./scheduler_sim` or `scheduler_sim.exe` on Windows).

**Note:** `-lm` is required for `sqrt()` (jitter σ).

## Metrics

- **Total cycles completed** — `scheduler_tick()` calls (equals simulated ms).
- **Per task:** execution count, missed deadlines, **jitter ±σ (ms)**, **start-time variance (ms²)**.
- **Determinism score** — 100% when every instance meets its completion deadline (`complete ≤ release + deadline_ms`).

## CV-style outcomes

- Cooperative scheduler over 10 s simulated time; deadline compliance and jitter metrics without hardware.
- Structured miss logging on `stderr` when a completion exceeds `release + deadline_ms`.
