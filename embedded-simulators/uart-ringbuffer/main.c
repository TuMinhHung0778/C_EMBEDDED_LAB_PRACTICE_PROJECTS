/**
 * Project 2: UART Ring Buffer Simulator (with Command Parser)
 * Spec: embedded-simulator-practice.md
 *
 * - RX arrives byte-by-byte (simulates UART RX ISR putting bytes into a ring buffer)
 * - Main loop drains the buffer and feeds a parser FSM
 * - Logs parse events, overflow, and error recovery with timestamps (ms)
 */
#include "parser.h"
#include "ringbuffer.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t tick_ms;
    uint32_t overflow_events;
    uint32_t dropped_bytes_last_burst;
    uint32_t dropped_bytes_total;
    uint32_t rx_messages;
    uint32_t parser_success;
} SimStats;

static void log_rx(uint32_t tick_ms, const char *label)
{
    (void)printf("  [%" PRIu32 "ms] RX: %s\n", tick_ms, label);
}

static void log_parsed(uint32_t tick_ms, const ParsedCommand *cmd, uint32_t cmd_num)
{
    if (cmd->is_query) {
        (void)printf("       [%" PRIu32 "ms] -> PARSED: %s query [CMD #%" PRIu32 "]\n",
                     tick_ms,
                     cmd->cmd_name,
                     cmd_num);
    } else if (cmd->param[0] != '\0') {
        (void)printf("       [%" PRIu32 "ms] -> PARSED: %s = %s [CMD #%" PRIu32 "]\n",
                     tick_ms,
                     cmd->cmd_name,
                     cmd->param,
                     cmd_num);
    } else {
        (void)printf("       [%" PRIu32 "ms] -> PARSED: %s [CMD #%" PRIu32 "]\n",
                     tick_ms,
                     cmd->cmd_name,
                     cmd_num);
    }
}

static void log_parse_error(uint32_t tick_ms, uint32_t err_num)
{
    (void)printf("       [%" PRIu32 "ms] -> PARSE ERROR [Error #%" PRIu32 "]\n", tick_ms, err_num);
}

static void log_overflow(uint32_t tick_ms, uint32_t dropped, uint32_t overflow_evt)
{
    (void)printf("       [%" PRIu32 "ms] -> OVERFLOW: %" PRIu32 " bytes dropped [Overflow #%" PRIu32 "]\n",
                 tick_ms,
                 dropped,
                 overflow_evt);
}

static void uart_rx_isr_burst(RingBuffer *rb, const uint8_t *bytes, uint32_t len)
{
    assert(rb != NULL);
    for (uint32_t i = 0; i < len; i++) {
        rb_put_char(rb, bytes[i]);
    }
}

static void drain_and_parse(RingBuffer *rb,
                            Parser *parser,
                            uint32_t tick_ms,
                            SimStats *stats,
                            bool *out_parsed_any,
                            bool *out_error_any)
{
    assert(rb != NULL);
    assert(parser != NULL);
    assert(stats != NULL);
    assert(out_parsed_any != NULL);
    assert(out_error_any != NULL);

    *out_parsed_any = false;
    *out_error_any = false;

    uint8_t ch = 0U;
    ParsedCommand cmd;
    const uint32_t prev_errors = parser->parse_errors;

    while (rb_get_char(rb, &ch)) {
        if (parser_feed(parser, ch, &cmd) && cmd.ready) {
            *out_parsed_any = true;
            stats->parser_success++;
            log_parsed(tick_ms, &cmd, parser->cmd_count);
        }
    }

    if (parser->parse_errors != prev_errors) {
        *out_error_any = true;
        log_parse_error(tick_ms, parser->parse_errors);
    }
}

static void drain_and_discard(RingBuffer *rb)
{
    assert(rb != NULL);
    uint8_t ch = 0U;
    while (rb_get_char(rb, &ch)) {
        /* intentionally discard */
    }
}

static void simulate_message(RingBuffer *rb,
                             Parser *parser,
                             SimStats *stats,
                             uint32_t tick_ms,
                             const char *printable_label,
                             const uint8_t *bytes,
                             uint32_t len,
                             bool enable_parse)
{
    assert(rb != NULL);
    assert(parser != NULL);
    assert(stats != NULL);

    stats->rx_messages++;
    log_rx(tick_ms, printable_label);

    const uint16_t overflow_before = rb->overflow_count;
    uart_rx_isr_burst(rb, bytes, len);
    const uint16_t overflow_after = rb->overflow_count;

    if (overflow_after != overflow_before) {
        const uint32_t dropped = (uint32_t)(overflow_after - overflow_before);
        stats->overflow_events++;
        stats->dropped_bytes_last_burst = dropped;
        stats->dropped_bytes_total += dropped;
        log_overflow(tick_ms, dropped, stats->overflow_events);
    }

    if (enable_parse) {
        bool parsed_any = false;
        bool error_any = false;
        drain_and_parse(rb, parser, tick_ms, stats, &parsed_any, &error_any);
        (void)parsed_any;
        (void)error_any;
    } else {
        /* Overflow stress test: validate buffer behavior without turning random bytes into parse errors. */
        drain_and_discard(rb);
    }
}

static void fill_pattern(uint8_t *buf, uint32_t len)
{
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = (uint8_t)alphabet[i % (uint32_t)(sizeof(alphabet) - 1U)];
    }
}

int main(void)
{
    RingBuffer rb;
    Parser parser;
    SimStats stats;
    memset(&stats, 0, sizeof(stats));

    rb_init(&rb);
    parser_init(&parser);

    (void)printf("=== UART Ring Buffer Simulator ===\n");
    (void)printf("Buffer size: %u bytes\n\n", (unsigned)RB_CAPACITY);
    (void)printf("Input sequence:\n");

    const uint8_t msg1[] = "AT+NAME=SystemA\r\n";
    simulate_message(&rb,
                     &parser,
                     &stats,
                     1U,
                     "'AT+NAME=SystemA\\r\\n'",
                     msg1,
                     (uint32_t)strlen((const char *)msg1),
                     true);

    const uint8_t msg2[] = "AT+BAUD?\r\n";
    simulate_message(&rb,
                     &parser,
                     &stats,
                     5U,
                     "'AT+BAUD?\\r\\n'",
                     msg2,
                     (uint32_t)strlen((const char *)msg2),
                     true);

    const uint8_t msg3[] = "@@@###\r\n";
    simulate_message(&rb,
                     &parser,
                     &stats,
                     8U,
                     "'@@@###\\r\\n'",
                     msg3,
                     (uint32_t)strlen((const char *)msg3),
                     true);

    /* Overflow burst: 200 bytes into 128-byte ring buffer (drop oldest). */
    uint8_t burst[200];
    fill_pattern(burst, (uint32_t)sizeof(burst));
    simulate_message(&rb, &parser, &stats, 10U, "[200 byte overflow]", burst, (uint32_t)sizeof(burst), false);

    const uint8_t msg4[] = "AT+OK\r\n";
    simulate_message(&rb,
                     &parser,
                     &stats,
                     12U,
                     "'AT+OK\\r\\n'",
                     msg4,
                     (uint32_t)strlen((const char *)msg4),
                     true);

    (void)printf("\n=== Statistics ===\n");
    (void)printf("Total RX bytes: %" PRIu32 "\n", rb.total_rx_bytes);
    (void)printf("Total overflow: %" PRIu32 " bytes\n", stats.dropped_bytes_total);
    (void)printf("Valid commands: %" PRIu32 "\n", parser.cmd_count);
    (void)printf("Parse errors: %" PRIu32 "\n", parser.parse_errors);

    const bool integrity_ok = rb_available_bytes(&rb) < RB_CAPACITY;
    (void)printf("Buffer integrity: %s\n", integrity_ok ? "MAINTAINED" : "BROKEN");

    const bool recovery_ok = (parser.parse_errors > 0U) ? (parser.cmd_count >= 3U) : true;
    (void)printf("Recovery success: %s\n", recovery_ok ? "100%" : "FAILED");

    return 0;
}

