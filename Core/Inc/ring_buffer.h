#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RB_SIZE 128u

typedef struct {
    volatile uint8_t buf[RB_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} ring_buffer_t;

size_t rb_push(ring_buffer_t *rb, const uint8_t *data, size_t len);
bool rb_pop(ring_buffer_t *rb, uint8_t *out);
bool rb_is_empty(ring_buffer_t *rb);
bool rb_is_full(ring_buffer_t *rb);
void rb_reset(ring_buffer_t *rb);

#endif
