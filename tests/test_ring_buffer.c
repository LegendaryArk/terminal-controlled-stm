#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ring_buffer.h"

static void test_starts_empty(ring_buffer_t *rb) {
    uint8_t out = 0xAA;
    assert(rb_is_empty(rb));
    assert(!rb_is_full(rb));
    assert(!rb_pop(rb, &out));
    assert(out == 0xAA);
}

static void test_fifo_order(ring_buffer_t *rb) {
    const uint8_t in[] = {1, 2, 3, 4};
    uint8_t out;
    assert(rb_push(rb, in, sizeof in) == sizeof in);
    for (size_t i = 0; i < sizeof in; i++) {
        assert(rb_pop(rb, &out));
        assert(out == in[i]);
    }
    assert(rb_is_empty(rb));
}

static void test_fill_to_capacity(ring_buffer_t *rb) {
    uint8_t in[RB_SIZE - 1];
    for (size_t i = 0; i < RB_SIZE - 1; i++) in[i] = (uint8_t) i;
    assert(rb_push(rb, in, sizeof in) == RB_SIZE -1);
    assert(rb_is_full(rb));
    uint8_t extra = 0x55;
    assert(rb_push(rb, &extra, 1) == 0);

    uint8_t out;
    for (size_t i = 0; i < RB_SIZE - 1; i++) {
        assert(rb_pop(rb, &out));
        assert(out == (uint8_t) i);
    }
    assert(!rb_pop(rb, &out));
    assert(out != 0x55);
}

static void test_overflow_keeps_data(ring_buffer_t *rb) {
    uint8_t in[RB_SIZE + 10];
    for (size_t i = 0; i < sizeof in; i++) in[i] = (uint8_t) i;
    assert(rb_push(rb, in, sizeof in) == RB_SIZE - 1);
    assert(rb_is_full(rb));
    assert(!rb_is_empty(rb));

    uint8_t out;
    for (size_t i = 0; i < RB_SIZE - 1; i++) {
        assert(rb_pop(rb, &out));
        assert(out == (uint8_t) i);
    }
    assert(rb_is_empty(rb));
}

static void test_wraparound(ring_buffer_t *rb) {
    uint8_t out;
    for (unsigned i = 0; i < 3 * RB_SIZE; i++) {
        uint8_t b = (uint8_t) i;
        assert(rb_push(rb, &b, 1) == 1);
        assert(rb_pop(rb, &out));
        assert(out == b);
    }
    assert(rb_is_empty(rb));
}

static void test_push_zero_len(ring_buffer_t *rb) {
    uint8_t b = 1;
    assert(rb_push(rb, &b, 0) == 0);
    assert(rb_is_empty(rb));
}

static void test_refill_after_wrap(ring_buffer_t *rb) {
    uint8_t out;
    for (unsigned i = 0; i < 100; i++) {
        uint8_t b = (uint8_t) i;
        assert(rb_push(rb, &b, 1) == 1);
        assert(rb_pop(rb, &out));
    }
    assert(rb_is_empty(rb));

    uint8_t in[RB_SIZE + 10];
    for (size_t i = 0; i < sizeof in; i++) in[i] = (uint8_t) i;
    assert(rb_push(rb, in, sizeof in) == RB_SIZE - 1);
    assert(rb_is_full(rb));

    for (size_t i = 0; i < RB_SIZE - 1; i++) {
        assert(rb_pop(rb, &out));
        assert(out == (uint8_t) i);
    }
    assert(rb_is_empty(rb));
}

static void test_partial_push_mid_buffer(ring_buffer_t *rb) {
    uint8_t in[RB_SIZE];
    uint8_t out;

    for (size_t i = 0; i < RB_SIZE; i++) in[i] = (uint8_t) i;
    assert(rb_push(rb, in, 50) == 50);
    for (size_t i = 0; i < 20; i++) {
        assert(rb_pop(rb, &out));
        assert(out == (uint8_t) i);
    }

    size_t space = RB_SIZE - 1 - 30;
    for (size_t i = 0; i < sizeof in; i++) in[i] = (uint8_t) (100 + i);
    assert(rb_push(rb, in, sizeof in) == space);
    assert(rb_is_full(rb));

    for (size_t i = 20; i < 50; i++) {
        assert(rb_pop(rb, &out));
        assert(out == (uint8_t) i);
    }
    for (size_t i = 0; i < space; i++) {
        assert(rb_pop(rb, &out));
        assert(out == (uint8_t) (100 + i));
    }
    assert(rb_is_empty(rb));
}

static void test_interleaved_vs_reference(ring_buffer_t *rb) {
    static uint8_t ref[4096];
    size_t ref_head = 0, ref_tail = 0;
    uint8_t next = 0;

    const int pushes[] = { 5, 10, 127, 60 };
    const int pops[]   = { 3, 12, 50, 200 };

    for (int round = 0; round < 10; round++) {
        for (size_t s = 0; s < sizeof pushes / sizeof pushes[0]; s++) {
            uint8_t in[RB_SIZE];
            for (int i = 0; i < pushes[s]; i++) in[i] = next++;
            size_t n = rb_push(rb, in, (size_t) pushes[s]);
            assert(n <= (size_t) pushes[s]);
            for (size_t i = 0; i < n; i++) ref[ref_head++] = in[i];
            next = (uint8_t) (next - (pushes[s] - (int) n));

            for (int i = 0; i < pops[s]; i++) {
                uint8_t out;
                bool got = rb_pop(rb, &out);
                assert(got == (ref_tail < ref_head));
                if (got) assert(out == ref[ref_tail++]);
            }
        }
    }
    assert(rb_is_empty(rb) == (ref_tail == ref_head));
}

static void test_reset_nonempty(ring_buffer_t *rb) {
    const uint8_t in[] = { 1, 2, 3, 4, 5 };
    uint8_t out = 0xAA;
    assert(rb_push(rb, in, sizeof in) == sizeof in);
    assert(!rb_is_empty(rb));

    rb_reset(rb);
    assert(rb_is_empty(rb));
    assert(!rb_is_full(rb));
    assert(!rb_pop(rb, &out));
    assert(out == 0xAA);
}

static void test_independent_instances(ring_buffer_t *rb) {
    (void) rb; // Uses its own a/b instances
    ring_buffer_t a, b;
    rb_reset(&a);
    rb_reset(&b);

    const uint8_t in[] = { 7, 8, 9 };
    uint8_t out;
    assert(rb_push(&a, in, sizeof in) == sizeof in);
    assert(!rb_is_empty(&a));
    assert(rb_is_empty(&b));
    assert(!rb_pop(&b, &out));

    uint8_t x = 42;
    assert(rb_push(&b, &x, 1) == 1);
    for (size_t i = 0; i < sizeof in; i++) {
        assert(rb_pop(&a, &out));
        assert(out == in[i]);
    }
    assert(rb_is_empty(&a));
    assert(rb_pop(&b, &out) && out == 42);
}

static void test_random_vs_reference(ring_buffer_t *rb) {
    enum { CAP = RB_SIZE - 1, OPS = 100000 };
    uint8_t ref[RB_SIZE];
    size_t ref_head = 0, ref_tail = 0, ref_count = 0;
    uint8_t next = 0;

    srand(1);

    for (int op = 0; op < OPS; op++) {
        if (rand() & 1) {
            size_t want = (size_t) (rand() % (RB_SIZE + 8));

            uint8_t in[RB_SIZE + 8];
            for (size_t i = 0; i < want; i++) in[i] = (uint8_t) (next + i);

            size_t n = rb_push(rb, in, want);
            size_t space = CAP - ref_count;
            assert(n == (want < space ? want : space));

            for (size_t i = 0; i < n; i++) {
                ref[ref_head] = in[i];
                ref_head = (ref_head + 1) % RB_SIZE;
            }
            ref_count += n;
            next = (uint8_t) (next + n);
        } else {
            size_t want = (size_t) (rand() % (RB_SIZE + 8));
            for (size_t i = 0; i < want; i++) {
                uint8_t out;
                bool got = rb_pop(rb, &out);
                assert(got == (ref_count > 0));
                if (!got) break;
                assert(out == ref[ref_tail]);
                ref_tail = (ref_tail + 1) % RB_SIZE;
                ref_count--;
            }
        }

        assert(rb_is_empty(rb) == (ref_count == 0));
        assert(rb_is_full(rb) == (ref_count == CAP));
    }
}

typedef struct {
    const char *name;
    void (*fn)(ring_buffer_t *rb);
} test_t;

#define T(name) { #name, name }

static const test_t tests[] = {
    T(test_starts_empty),
    T(test_fifo_order),
    T(test_fill_to_capacity),
    T(test_overflow_keeps_data),
    T(test_wraparound),
    T(test_push_zero_len),
    T(test_refill_after_wrap),
    T(test_partial_push_mid_buffer),
    T(test_interleaved_vs_reference),
    T(test_reset_nonempty),
    T(test_independent_instances),
    T(test_random_vs_reference),
};

#define NUM_TESTS (sizeof tests / sizeof tests[0])

static void run(const test_t *t) {
    ring_buffer_t rb;
    memset(&rb, 0xCD, sizeof rb);
    rb_reset(&rb);
    t->fn(&rb);
    printf("PASS %s\n", t->name);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        for (size_t i = 0; i < NUM_TESTS; i++) {
            if (strcmp(argv[1], tests[i].name) == 0) {
                run(&tests[i]);
                return 0;
            }
        }
        fprintf(stderr, "unknown test '%s'\n", argv[1]);
        return 2;
    }

    for (size_t i = 0; i < NUM_TESTS; i++) run(&tests[i]);
    printf("All %zu tests passed\n", NUM_TESTS);
    return 0;
}