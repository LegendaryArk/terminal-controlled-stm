#include "uart.h"

#include "stm32f446xx.h"

ring_buffer_t rx_rb;
ring_buffer_t tx_rb;

void uart_putc(uint8_t c) {
  while (!rb_push(&tx_rb, &c, 1)) {}
  USART2->CR1 |= USART_CR1_TXEIE;
}

void uart_puts(const char *s) {
  while (*s) uart_putc((uint8_t) *s++);
}
