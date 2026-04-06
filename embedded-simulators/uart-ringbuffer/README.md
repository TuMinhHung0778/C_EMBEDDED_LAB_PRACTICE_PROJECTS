# Project 2: UART Ring Buffer Simulator (with Command Parser)

Simulates a UART RX data path (PC-run C code, embedded-style logic):

- RX bytes arrive **one-by-one** (ISR-like producer) into a fixed-size ring buffer
- Main loop drains the buffer (consumer) and feeds a **finite-state-machine (FSM)** parser
- Logs important events with timestamps:
  - valid command parsed
  - parse error + recovery (skip until `\r\n`)
  - ring buffer overflow (drop-oldest policy)

Spec: [embedded-simulator-practice.md](../../embedded-simulator-practice.md) (Project 2).

## What is implemented

### Ring buffer (`ringbuffer.c/.h`)

- Capacity: 128 bytes (`RB_CAPACITY`)
- `rb_put_char()` overflow behavior: **drop the oldest byte** (advance tail), keep the newest
- Metrics:
  - `total_rx_bytes`
  - `overflow_count` (dropped bytes)

### Parser FSM (`parser.c/.h`)

Supported patterns:
- `AT\r\n` or `AT\n` → parsed as `OK`
- `AT+NAME=value\r\n`
- `AT+BAUD?\r\n`

Recovery:
- On malformed input, parser enters `STATE_SKIP_TO_EOL` and ignores bytes until `\r\n`, then returns to idle.

## Build & run

From `embedded-simulators/`:

```bash
gcc -Wall -Wextra -std=c11 -O2 uart-ringbuffer/ringbuffer.c uart-ringbuffer/parser.c uart-ringbuffer/main.c -o uart_sim
./uart_sim
```

## Demo scenarios in `main.c`

- Valid: `AT+NAME=SystemA\r\n`
- Query: `AT+BAUD?\r\n`
- Malformed: `@@@###\r\n`
- Overflow stress: 200-byte burst into 128-byte buffer (drop-oldest)
- Mixed: `AT+OK\r\n`

Expected high-level outcomes:
- Overflow event shows **72 bytes dropped** (200 - 128)
- Parser recovers and continues parsing subsequent valid commands

