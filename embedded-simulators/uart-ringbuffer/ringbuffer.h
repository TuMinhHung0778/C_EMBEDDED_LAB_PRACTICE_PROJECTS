#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>
#include <stdint.h>

#define RB_CAPACITY 128U

typedef struct {
    uint8_t buffer[RB_CAPACITY];
    uint16_t head;           /* write index */
    uint16_t tail;           /* read index */
    uint16_t count;          /* buffered bytes (0..RB_CAPACITY) */
    uint16_t overflow_count; /* bytes dropped due to overflow */
    uint32_t total_rx_bytes; /* total bytes offered to rb_put_char */
} RingBuffer;

void rb_init(RingBuffer *rb);

/* Add one byte. If full, drop oldest byte (tail++) and count overflow. */
void rb_put_char(RingBuffer *rb, uint8_t ch);

/* Pop one byte from the buffer; returns false if empty. */
bool rb_get_char(RingBuffer *rb, uint8_t *out_ch);

/* Number of bytes currently buffered. */
uint16_t rb_available_bytes(const RingBuffer *rb);

/* True if buffer currently full (count == RB_CAPACITY). */
bool rb_is_full(const RingBuffer *rb);

#endif
