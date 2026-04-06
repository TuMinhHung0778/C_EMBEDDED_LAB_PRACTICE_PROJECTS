# C_EMBEDDED_LAB_PRACTICE_PROJECTS

PC-run C11 simulators that demonstrate **embedded firmware fundamentals** without hardware:

- **Real-time thinking**: periodic scheduling, deadlines, timing analysis
- **Reliability**: overflow-safe buffering, robust parsing, graceful recovery
- **Measurable outcomes**: deterministic metrics, counters, event logs with timestamps

This repo is designed to be **CV-ready**: a recruiter can quickly understand what you built, why it matters, and how you validated it.

## Tech stack

- **Language**: C11
- **Compiler**: GCC / MinGW (Windows), GCC/Clang (Linux/macOS)
- **Style**: small modules, clear state machines, defensive checks (`assert`)
- **Spec**: `embedded-simulator-practice.md`

## Quick start (build & run)

From the repo root:

```bash
cd embedded-simulators

# Project 1
gcc -Wall -Wextra -std=c11 -O2 scheduler/scheduler.c scheduler/main.c -o scheduler_sim -lm
./scheduler_sim

# Project 2
gcc -Wall -Wextra -std=c11 -O2 uart-ringbuffer/ringbuffer.c uart-ringbuffer/parser.c uart-ringbuffer/main.c -o uart_sim
./uart_sim
```

Notes:
- `-lm` links the math library (needed for `sqrt()` in Project 1 on some platforms).
- On Windows, binaries are typically `*.exe`.

## Repository structure

```
embedded-simulators/
├── scheduler/              # Project 1: Cooperative Scheduler
│   ├── scheduler.c
│   ├── scheduler.h
│   ├── main.c
│   └── README.md
├── uart-ringbuffer/        # Project 2: UART Ring Buffer + Parser FSM
│   ├── ringbuffer.c
│   ├── ringbuffer.h
│   ├── parser.c
│   ├── parser.h
│   ├── main.c
│   └── README.md
└── Makefile
```

## Project 1 — Cooperative Task Scheduler Simulator

### What it demonstrates

A **cooperative (non-preemptive)** scheduler running on a simulated 1 ms tick, managing 3 periodic tasks. It measures timing behavior and detects missed deadlines.

### Task set (from spec)

| Task | Period | Relative deadline |
|------|--------|-------------------|
| A    | 10 ms  | 5 ms              |
| B    | 25 ms  | 12 ms             |
| C    | 50 ms  | 25 ms             |

### Metrics (measurable outcomes)

- **Execution count** per task over 10 seconds
- **Deadline misses** (with structured miss logs on `stderr`)
- **Jitter statistics**: start-time error vs ideal period grid
  - **±σ (ms)** and **variance (ms²)**
- **Determinism score**: % of task instances meeting deadlines

### Demo output (real run)

```
=== Task Scheduler Simulation (10000 ms) ===
Task A (10ms period, 5ms deadline):
  - Executed: 1000 times
  - Missed Deadlines: 0
  - Jitter: ±0.03ms
  - Start-time variance: 0.0010 ms²
Task B (25ms period, 12ms deadline):
  - Executed: 400 times
  - Missed Deadlines: 0
  - Jitter: ±0.50ms
  - Start-time variance: 0.2550 ms²
Task C (50ms period, 25ms deadline):
  - Executed: 200 times
  - Missed Deadlines: 0
  - Jitter: ±0.07ms
  - Start-time variance: 0.0050 ms²

Total simulation time: 10000ms
Total cycles completed: 10000
Determinism score: 100% (all tasks met deadlines)
```

More details: `embedded-simulators/scheduler/README.md`

## Project 2 — UART Ring Buffer Simulator (with Command Parser)

### What it demonstrates

An embedded-style **producer/consumer** UART RX path:

- Producer (ISR-like): pushes bytes into a fixed-size **ring buffer (128 bytes)**
- Consumer (main loop): drains bytes and feeds a **parser FSM**
- Robustness:
  - Overflow-safe behavior: **drop-oldest, keep-newest**
  - Malformed input recovery: skip to end-of-line (`\r\n`) and continue
- Observability: event logs with timestamps + summary statistics

### Demo scenarios (from spec)

- Valid: `AT+NAME=SystemA\r\n`
- Query: `AT+BAUD?\r\n`
- Malformed: `@@@###\r\n` → parse error + recovery
- Overflow: 200-byte burst into 128-byte buffer → **72 bytes dropped**
- Mixed: `AT+OK\r\n`

### Demo output (real run)

```
=== UART Ring Buffer Simulator ===
Buffer size: 128 bytes

Input sequence:
  [1ms] RX: 'AT+NAME=SystemA\r\n'
       [1ms] -> PARSED: NAME = SystemA [CMD #1]
  [5ms] RX: 'AT+BAUD?\r\n'
       [5ms] -> PARSED: BAUD query [CMD #2]
  [8ms] RX: '@@@###\r\n'
       [8ms] -> PARSE ERROR [Error #1]
  [10ms] RX: [200 byte overflow]
       [10ms] -> OVERFLOW: 72 bytes dropped [Overflow #1]
  [12ms] RX: 'AT+OK\r\n'
       [12ms] -> PARSED: OK [CMD #3]

=== Statistics ===
Total RX bytes: 242
Total overflow: 72 bytes
Valid commands: 3
Parse errors: 1
Buffer integrity: MAINTAINED
Recovery success: 100%
```

More details: `embedded-simulators/uart-ringbuffer/README.md`

## CV-ready bullets (copy/paste)

- Built a cooperative scheduler simulator (1 ms tick) with deadline detection and jitter analysis; achieved **100% deadline compliance** over a 10 s run (1600 total task executions).
- Implemented a UART RX ring buffer with overflow-safe drop policy; validated behavior under stress (**200-byte burst into 128-byte buffer**, 72 bytes dropped as expected).
- Developed an AT-like command parser using a finite-state-machine (FSM); demonstrated **graceful recovery** from malformed input with clear event logs and reliability metrics.
