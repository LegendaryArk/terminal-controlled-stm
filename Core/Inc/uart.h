#ifndef UART_H
#define UART_H

#include <stdint.h>

#include "ring_buffer.h"

extern ring_buffer_t tx_rb;
extern ring_buffer_t rx_rb;

void uart_putc(uint8_t c);
void uart_puts(const char *s);

#endif
