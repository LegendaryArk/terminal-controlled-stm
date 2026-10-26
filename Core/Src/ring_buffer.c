#include "ring_buffer.h"

_Static_assert((RB_SIZE & (RB_SIZE - 1)) == 0 && RB_SIZE <= 256, "RB_SIZE must be a power of 2 <= 256");

size_t rb_push(ring_buffer_t *rb, const uint8_t *data, size_t len) {
    if (rb_is_full(rb)) return 0;

    size_t n = 0;
    while (n < len && !rb_is_full(rb)) {
        rb->buf[rb->head] = data[n++];
        rb->head = (rb->head + 1) & (RB_SIZE - 1);
    }
    return n;
}

bool rb_pop(ring_buffer_t *rb, uint8_t *out) {
    if (rb_is_empty(rb)) return false;

    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & (RB_SIZE - 1);
    return true;
}

bool rb_is_empty(ring_buffer_t *rb) {
    return rb->tail == rb->head;
}

bool rb_is_full(ring_buffer_t *rb) {
    return ((rb->head + 1) & (RB_SIZE - 1)) == rb->tail;
}

void rb_reset(ring_buffer_t *rb) {
    rb->head = rb->tail = 0;
}
