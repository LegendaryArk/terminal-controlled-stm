#include "main.h"

#include <string.h>
#include <stdio.h>

#include "stm32f446xx.h"
#include "cmds.h"
#include "ring_buffer.h"
#include "uart.h"
#include "led.h"

volatile uint32_t ms_ticks = 0;

uint8_t len = 0;
uint8_t line[RB_SIZE] = {0xAA};
uint8_t c;
bool prev_cr = false;

const char nl[] = "\r\n";
const char prompt[] = "> ";
const char bs[] = "\b \b";

int main(void) {
  rb_reset(&rx_rb);
  rb_reset(&tx_rb);

  SysTick->LOAD = (SystemCoreClock / 1000) - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
  (void) RCC->AHB1ENR;
  (void) RCC->APB1ENR;

  GPIOA->MODER |= (1u << (5 * 2));
  GPIOA->MODER = (GPIOA->MODER & ~(0xFu << 4)) | (0xAu << 4);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFFu << 8)) | (0x77u << 8);

  USART2->BRR = (8u << USART_BRR_DIV_Mantissa_Pos) | 11u; // Baud Rate of 115200, USARTDIV of 8.68
  USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_UE;

  NVIC_EnableIRQ(USART2_IRQn);

  uart_puts(nl);
  uart_puts(prompt);

  while (1) {
    while (rb_pop(&rx_rb, &c)) {
      if (c == '\r' || c == '\n') {
        if (c == '\n' && prev_cr) { prev_cr = false; continue; }
        prev_cr = (c == '\r');

        uart_puts(nl);

        line[len] = '\0';
        int argc = 0;
        char *argv[MAX_ARGS + 1];
        char *tok = strtok((char *) line, " ");
        while(tok != NULL && argc < MAX_ARGS) {
          argv[argc++] = tok;
          tok = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        if (argc > 0) {
          bool found = false;
          for (size_t i = 0; i < num_cmds; i++) {
            if (strcmp(argv[0], cmds[i].name) == 0) {
              found = true;
              cmds[i].handler(argc, argv);
              break;
            }
          }

          if (!found) {
            char buf[64];
            snprintf(buf, sizeof buf, "unknown command '%s'\r\n", argv[0]);
            uart_puts(buf);
          }
        }

        len = 0;

        uart_puts(prompt);
      } else if (c == 0x7F || c == 0x08) {
        prev_cr = false;
        if (len > 0) len--;
        uart_puts(bs);
      } else if (c >= 0x20 && c <= 0x7E && len < RB_SIZE) {
        prev_cr = false;
        line[len++] = c;
        uart_putc(c);
      }
    }

    led_update(&led);
  }
}
