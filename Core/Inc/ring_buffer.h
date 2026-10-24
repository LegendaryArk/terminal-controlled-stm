#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RB_SIZE 128u

#endif

size_t rb_push(const uint8_t *data, size_t len);
bool rb_pop(uint8_t *out);
bool rb_is_empty(void);
bool rb_is_full(void);
void rb_reset(void);
