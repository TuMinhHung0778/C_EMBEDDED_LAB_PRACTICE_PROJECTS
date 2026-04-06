#include "ringbuffer.h"

#include <assert.h>

static uint16_t rb_next(uint16_t idx)
{
    return (uint16_t)((idx + 1U) % RB_CAPACITY);
}

void rb_init(RingBuffer *rb)
{
    assert(rb != NULL);
    rb->head = 0U;
    rb->tail = 0U;
    rb->count = 0U;
    rb->overflow_count = 0U;
    rb->total_rx_bytes = 0U;
}

uint16_t rb_available_bytes(const RingBuffer *rb)
{
    assert(rb != NULL);
    return rb->count;
}

bool rb_is_full(const RingBuffer *rb)
{
    assert(rb != NULL);
    return rb->count == RB_CAPACITY;
}

void rb_put_char(RingBuffer *rb, uint8_t ch)
{
    assert(rb != NULL);
    rb->total_rx_bytes++;

    if (rb_is_full(rb)) {
        /* Overflow policy (per spec): drop oldest, keep newest. */
        rb->tail = rb_next(rb->tail);
        rb->overflow_count++;
        rb->count--;
    }

    rb->buffer[rb->head] = ch;
    rb->head = rb_next(rb->head);
    rb->count++;
}

bool rb_get_char(RingBuffer *rb, uint8_t *out_ch)
{
    assert(rb != NULL);
    assert(out_ch != NULL);

    if (rb->count == 0U) {
        return false;
    }

    *out_ch = rb->buffer[rb->tail];
    rb->tail = rb_next(rb->tail);
    rb->count--;
    return true;
}

