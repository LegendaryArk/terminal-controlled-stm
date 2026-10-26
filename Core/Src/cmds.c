#include "cmds.h"

#include <string.h>
#include <stdio.h>

#include "stm32f446xx.h"
#include "uart.h"

const cmd_t cmds[] = {
  { "help", "help [command]", "Show this list, or details for one command", cmd_help },
  { "uptime", "uptime", "Show time since reset", cmd_uptime },
  { "echo", "echo [text...]", "Print the text back", cmd_echo },
  { "led", "led <on|off|toggle>", "Control the green user LED (LD2)", cmd_led }
};

const size_t num_cmds = sizeof cmds / sizeof cmds[0];

static const char nl[] = "\r\n";

static const char led_help[] =
  "usage: led <on|off|toggle>\r\n"
  "  on        turn the LED on\r\n"
  "  off       turn the LED off\r\n"
  "  toggle    switch the LED to the opposite state\r\n";

void cmd_help(int argc, char **argv) {
  char buf[96];
  if (argc == 1) {
    uart_puts("Commands:\r\n");
    for (size_t i = 0; i < num_cmds; i++) {
      snprintf(buf, sizeof buf, "  %-21s %s \r\n", cmds[i].usage, cmds[i].desc);
      uart_puts(buf);
    }
    return;
  }

  if (strcmp(argv[1], "led") == 0) {
    uart_puts(led_help);
    return;
  }

  for (size_t i = 0; i < num_cmds; i++) {
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

void cmd_led(int argc, char **argv) {
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

void cmd_uptime(int argc, char **argv) {
  if (argc > 1) {
    uart_puts("uptime: too many arguments\r\n");
    return;
  }

  char uptime[24];
  snprintf(uptime, sizeof uptime, "%lu ticks", ms_ticks);
  uart_puts(uptime);
  uart_puts(nl);
}

void cmd_echo(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    uart_puts(argv[i]);
    uart_puts(" ");
  }
  uart_puts(nl);
}
