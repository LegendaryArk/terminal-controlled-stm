#include "ring_buffer.h"

_Static_assert((RB_SIZE & (RB_SIZE - 1)) == 0 && RB_SIZE <= 256, "RB_SIZE must be a power of 2 <= 256");

static volatile uint8_t buf[RB_SIZE];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;

size_t rb_push(const uint8_t *data, size_t len) {
    if (rb_is_full()) return false;

    size_t n = 0;
    while (n < len && !rb_is_full()) {
        buf[head] = data[n++];
        head = (head + 1) & (RB_SIZE - 1);
    }
    return n;
}

bool rb_pop(uint8_t *out) {
    if (rb_is_empty()) return false;

    *out = buf[tail];
    tail = (tail + 1) & (RB_SIZE - 1);
    return true;
}

bool rb_is_empty(void) {
    return tail == head;
}

bool rb_is_full(void) {
    return ((head + 1) & (RB_SIZE - 1)) == tail;
}

void rb_reset(void) {
    head = tail = 0;
}
