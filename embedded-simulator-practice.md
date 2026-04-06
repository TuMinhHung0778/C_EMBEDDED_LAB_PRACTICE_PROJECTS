# Embedded Practice Projects - Simulator-Based Implementation Guide

## Overview

Build 2 simple C simulator projects to demonstrate firmware fundamentals without needing real hardware. Focus on code quality, reliability, and measurable outcomes.

---

## Project 1: Cooperative Task Scheduler Simulator

### Purpose

**ATS Keywords**: Real-time scheduling, deterministic behavior, task management, deadline verification, timing accuracy.

### Problem Statement

Simulate a simple cooperative (non-preemptive) task scheduler that manages 3 periodic tasks with different periods. The scheduler must:

- Run a 1ms tick timer (simulated via loop counter)
- Execute tasks at their scheduled periods
- Detect and log missed deadlines
- Calculate jitter and timing accuracy

### Implementation Details

#### Core Concept

```c
// Task structure
typedef struct {
    const char *name;
    uint16_t period_ms;        // Task period in milliseconds
    uint16_t deadline_ms;      // Deadline relative to period start
    uint16_t max_duration_ms;  // Max execution time allowed
    uint16_t next_run_tick;    // When task should run
    uint32_t exec_count;       // How many times executed
    uint32_t missed_deadlines; // Deadline miss counter
} Task;

// Scheduler state
typedef struct {
    uint32_t current_tick;     // Running time counter (ms)
    Task tasks[3];
    uint32_t total_cycles;
} Scheduler;
```

#### Key Features to Implement

1. **Task Definition**: 3 tasks with different periods:
   - Task A: 10ms period, 5ms deadline
   - Task B: 25ms period, 12ms deadline
   - Task C: 50ms period, 25ms deadline

2. **Tick Loop**: Simulate main loop:

   ```c
   for (uint32_t tick = 0; tick < 10000; tick++) {  // Run 10 seconds
       scheduler_tick(&scheduler);
       // Each iteration = 1ms
   }
   ```

3. **Deadline Detection**:
   - When task runs late (current_tick > deadline), increment miss counter
   - Log missed deadline with task name, expected time, actual time

4. **Metrics to Calculate**:
   - Total cycles completed
   - Per-task execution count
   - Per-task missed deadlines
   - Jitter (variance in task start time)

#### Example Output

```
=== Task Scheduler Simulation (10000 ms) ===
Task A (10ms period, 5ms deadline):
  - Executed: 1000 times
  - Missed Deadlines: 0
  - Jitter: ±0.2ms

Task B (25ms period, 12ms deadline):
  - Executed: 400 times
  - Missed Deadlines: 0
  - Jitter: ±0.5ms

Task C (50ms period, 25ms deadline):
  - Executed: 200 times
  - Missed Deadlines: 0
  - Jitter: ±1.2ms

Total simulation time: 10000ms
Determinism score: 100% (all tasks met deadlines)
```

#### CV-Ready Outcome Bullets

- Designed cooperative scheduler simulator achieving 100% deterministic deadline compliance over 10-second run (1000+ task executions).
- Implemented jitter analysis and deadline violation detection; verified real-time scheduling principles without hardware.
- Created structured logging to trace task timing behavior; facilitated debugging and performance analysis.

---

## Project 2: UART Ring Buffer Simulator (with Command Parser)

### Purpose

**ATS Keywords**: Ring buffer, producer/consumer, state-machine parser, overflow handling, fault recovery, defensive programming.

### Problem Statement

Simulate a UART receive ring buffer with a command parser that:

- Accepts character-by-character input (simulating UART RX interrupt)
- Buffers data in a fixed-size ring buffer without data loss (safe overflow)
- Parses incoming AT-like commands (e.g., `AT+NAME?`, `AT+BAUD=9600`)
- Recovers gracefully from malformed input
- Logs all events (overflow, parse error, valid command) with timestamps

### Implementation Details

#### Core Concept

```c
// Ring buffer for UART RX
typedef struct {
    uint8_t buffer[128];       // Fixed size = 128 bytes
    uint16_t head;             // Write pointer
    uint16_t tail;             // Read pointer
    uint16_t overflow_count;   // How many bytes dropped
    uint32_t total_rx_bytes;
} RingBuffer;

// Command parser state machine
typedef enum {
    STATE_IDLE,
    STATE_CMD_WAIT_PLUS,       // Waiting for '+' after 'AT'
    STATE_CMD_NAME,            // Reading command name
    STATE_CMD_PARAM,           // Reading parameter (after '=' or '?')
    STATE_COMPLETE
} ParserState;

typedef struct {
    char cmd_name[16];         // e.g., "NAME", "BAUD"
    char param[32];            // e.g., "9600"
    uint8_t is_query;          // 1 if ends with '?'
    uint32_t cmd_count;        // Valid commands parsed
    uint32_t parse_errors;     // Malformed input count
} Parser;
```

#### Key Features to Implement

1. **Ring Buffer Operations**:
   - `rb_put_char(RingBuffer *rb, uint8_t ch)`: Add character, handle overflow by dropping oldest
   - `rb_get_char(RingBuffer *rb, uint8_t *ch)`: Retrieve oldest character
   - `rb_available_bytes(RingBuffer *rb)`: Return bytes ready to read

2. **Parser State Machine**:
   - State transitions on each character
   - Recognize patterns: `AT+NAME=value\r\n`, `AT+BAUD?\r\n`, `AT\r\n`
   - On malformed input (e.g., `@#$%`), skip to next `\r\n` and count error

3. **Test Scenarios**:
   - Valid command: `"AT+NAME=ESP32\r\n"` → parse as NAME=ESP32
   - Query: `"AT+BAUD?\r\n"` → parse as BAUD query
   - Malformed: `"AT!!!???\r\n"` → count as error, stay stable
   - Overflow: Send 200 bytes into 128-byte buffer → log overflow, retain last 128 bytes
   - Mixed: Valid + malformed + valid sequence

#### Example Event Log

```
=== UART Ring Buffer Simulator ===
Buffer size: 128 bytes

Input sequence:
  [1ms] RX: 'AT+NAME=SystemA\r\n'  → PARSED: NAME = SystemA [CMD #1]
  [5ms] RX: 'AT+BAUD?\r\n'         → PARSED: BAUD query [CMD #2]
  [8ms] RX: '@@@###\r\n'           → PARSE ERROR [Error #1]
  [10ms] RX: [200 byte overflow]   → OVERFLOW: 72 bytes dropped [Overflow #1]
  [12ms] RX: 'AT+OK\r\n'           → PARSED: OK [CMD #3]

=== Statistics ===
Total RX bytes: 250
Total overflow: 72 bytes
Valid commands: 3
Parse errors: 1
Buffer integrity: MAINTAINED
Recovery success: 100%
```

#### CV-Ready Outcome Bullets

- Implemented UART-style ring buffer with overflow-safe circular storage; maintained buffer integrity under stress conditions (200-byte burst into 128-byte buffer).
- Built command parser using finite-state-machine (FSM) pattern; recovered gracefully from malformed input without crashing or losing buffer state.
- Synthesized event logging and error metrics; provided clear visibility into buffer behavior for debugging and reliability validation.

---

## Implementation Roadmap (Estimate: 3-5 days)

### Day 1-2: Task Scheduler

- [ ] Define Task and Scheduler structures
- [ ] Implement tick loop and task execution logic
- [ ] Add deadline detection
- [ ] Calculate jitter over 10-second run
- [ ] Print results table
- [ ] Commit to GitHub with README

### Day 3-4: UART Ring Buffer

- [ ] Implement ring buffer (put, get, available)
- [ ] Build parser state machine
- [ ] Test with valid/malformed/overflow inputs
- [ ] Generate event log output
- [ ] Commit to GitHub with README

### Day 5: Documentation & CV Ready

- [ ] Write concise README for each project
- [ ] Create summary test output (copy/paste into CV later)
- [ ] List measurable outcomes (determinism %, overflow recovery time, etc.)
- [ ] Note which JD keywords each project maps to

---

## File Organization (Suggested)

```
embedded-simulators/
├── scheduler/
│   ├── scheduler.c
│   ├── scheduler.h
│   ├── main.c
│   └── README.md
├── uart-ringbuffer/
│   ├── ringbuffer.c
│   ├── ringbuffer.h
│   ├── parser.c
│   ├── parser.h
│   ├── main.c
│   └── README.md
└── Makefile
```

---

## Compilation & Testing (No Dependencies)

```bash
# Scheduler
gcc -Wall -Wextra -std=c11 scheduler/scheduler.c scheduler/main.c -o scheduler_sim
./scheduler_sim

# Ring Buffer
gcc -Wall -Wextra -std=c11 uart-ringbuffer/ringbuffer.c uart-ringbuffer/parser.c uart-ringbuffer/main.c -o uart_sim
./uart_sim
```

---

## Tips for Quality Code

1. **Use assert() for invariants**: Buffer indices should never overlap incorrectly.
2. **Log all events**: Every task run, every buffer operation, every parse state change.
3. **Test edge cases**: Empty buffer, full buffer, single byte, rapid input, etc.
4. **Separate concerns**: Buffer logic ≠ Parser logic ≠ Scheduler logic.
5. **Add metrics**: Count successes, failures, edge cases—these become CV bullets.

---

## Next Steps

1. Implement both projects (commit to GitHub).
2. Send mình output screenshots + summary metrics.
