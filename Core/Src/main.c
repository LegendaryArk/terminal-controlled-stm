#include "main.h"

#include <string.h>
#include <stdio.h>

#include "ring_buffer.h"
#include "stm32f446xx.h"

static void uart_puts(const char *s);
static void uart_putc(uint8_t c);

typedef struct {
  const char *name;
  const char *usage;
  const char *desc;
  void (*handler)(int argc, char **argv);
} cmd_t;

static void cmd_help_handler(int argc, char **argv);
static void cmd_led_handler(int argc, char **argv);
static void cmd_uptime_handler(int argc, char **argv);
static void cmd_echo_handler(int argc, char **argv);

#define LED_PIN (1 << 5)

#define NUM_CMDS (sizeof cmds / sizeof cmds[0])
#define MAX_ARGS 8

volatile uint32_t ms_ticks = 0;

uint8_t rx_byte;

const cmd_t cmds[] = {
  { "help", "help [command]", "Show this list, or details for one command", cmd_help_handler },
  { "led", "led <on|off|toggle>", "Control the green user LED (LD2)", cmd_led_handler },
  { "uptime", "uptime", "Show time since reset", cmd_uptime_handler },
  { "echo", "echo [text...]", "Print the text back", cmd_echo_handler }
};

uint8_t len = 0;
uint8_t line[RB_SIZE] = {0xAA};
uint8_t c;
bool prev_cr = false;

const char nl[] = "\r\n";
const char prompt[] = "> ";
const char bs[] = "\b \b";

int main(void) {
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
    while (rb_pop(&c)) {
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
          for (size_t i = 0; i < NUM_CMDS; i++) {
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
  }
}

static void uart_putc(uint8_t c) {
  while (!(USART2->SR & USART_SR_TXE)) {}
  USART2->DR = c;
}

static void uart_puts(const char *s) {
  while (*s) uart_putc((uint8_t) *s++);
}

static const char led_help[] =
  "usage: led <on|off|toggle>\r\n"
  "  on        turn the LED on\r\n"
  "  off       turn the LED off\r\n"
  "  toggle    switch the LED to the opposite state\r\n";

static void cmd_help_handler(int argc, char **argv) {
  char buf[96];
  if (argc == 1) {
    uart_puts("Commands:\r\n");
    for (size_t i = 0; i < NUM_CMDS; i++) {
      snprintf(buf, sizeof buf, "  %-21s %s \r\n", cmds[i].usage, cmds[i].desc);
      uart_puts(buf);
    }
    return;
  }

  if (strcmp(argv[1], "led") == 0) {
    uart_puts(led_help);
    return;
  }

  for (size_t i = 0; i < NUM_CMDS; i++) {
    if (strcmp(argv[1], cmds[i].name) == 0) {
      snprintf(buf, sizeof buf, "usage: %s\r\n  %s\r\n", cmds[i].usage, cmds[i].desc);
      uart_puts(buf);
      return;
    }
  }

  snprintf(buf, sizeof buf, "help: unknown command '%s'\r\n", argv[1]);
  uart_puts(buf);
  return;
}

static void cmd_led_handler(int argc, char **argv) {
  char buf[64];

  if (argc != 2) {
    if (argc > 2) uart_puts("led: too many arguments\r\n");
    uart_puts(led_help);
    return;
  }

  if (strcmp(argv[1], "on") == 0) {
    GPIOA->BSRR = LED_PIN;
  } else if (strcmp(argv[1], "off") == 0) {
    GPIOA->BSRR = LED_PIN << 16;
  } else if (strcmp(argv[1], "toggle") == 0) {
    uint32_t odr = GPIOA->ODR;
    GPIOA->BSRR = ((odr & LED_PIN) << 16) | (~odr & LED_PIN);
  } else {
    snprintf(buf, sizeof buf, "led: unknown option '%s'\r\n", argv[1]);
    uart_puts(buf);
    uart_puts(led_help);
  }
}

static void cmd_uptime_handler(int argc, char **argv) {
  if (argc > 1) {
    uart_puts("uptime: too many arguments\r\n");
    return;
  }

  char uptime[24];
  snprintf(uptime, sizeof uptime, "%lu ticks", ms_ticks);
  uart_puts(uptime);
  uart_puts(nl);
}

static void cmd_echo_handler(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    uart_puts(argv[i]);
    uart_puts(" ");
  }
  uart_puts(nl);
}
