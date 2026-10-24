#include <assert.h>
#include <stdio.h>
#include "ring_buffer.h"

static void test_starts_empty(void) {
    uint8_t out = 0xAA;
    assert(rb_is_empty());
    assert(!rb_is_full());
    assert(!rb_pop(&out));
    assert(out == 0xAA);
}

static void test_fifo_order(void) {
    const uint8_t in[] = {1, 2, 3, 4};
    uint8_t out;
    assert(rb_push(in, sizeof in) == sizeof in);
    for (size_t i = 0; i < sizeof in; i++) {
        assert(rb_pop(&out));
        assert(out == in[i]);
    }
    assert(rb_is_empty());
}

static void test_fill_to_capacity(void) {
    uint8_t in[RB_SIZE - 1] = {0};
    assert(rb_push(in, sizeof in) == RB_SIZE -1);
    assert(rb_is_full());
    uint8_t extra = 0x55;
    assert(rb_push(&extra, 1) == 0);
}

static void test_overflow_keeps_data(void) {
    uint8_t in[RB_SIZE + 10];
    for (size_t i = 0; i < sizeof in; i++) in[i] = (uint8_t) i;
    assert(rb_push(in, sizeof in) == RB_SIZE - 1);
    assert(rb_is_full());
    assert(!rb_is_empty());
    uint8_t out;
    assert(rb_pop(&out) && out == 0);
}

static void test_wraparound(void) {
    uint8_t out;
    for (unsigned i = 0; i < 3 * RB_SIZE; i++) {
        uint8_t b = (uint8_t) i;
        assert(rb_push(&b, 1) == 1);
        assert(rb_pop(&out));
        assert(out == b);
    }
    assert(rb_is_empty());
}

static void test_push_zero_len(void) {
    uint8_t b = 1;
    assert(rb_push(&b, 0) == 0);
    assert(rb_is_empty());
}

#define RUN(t) do { rb_reset(); t(); printf("PASS %s\n", #t); } while (0);

int main(void) {
    RUN(test_starts_empty);
    RUN(test_fifo_order);
    RUN(test_fill_to_capacity);
    RUN(test_overflow_keeps_data);
    RUN(test_wraparound);
    RUN(test_push_zero_len);
    printf("All tests passed\n");
    return 0;
}