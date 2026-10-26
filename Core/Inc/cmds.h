#ifndef CMDS_H
#define CMDS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_ARGS 8

#define LED_PIN (1 << 5)

extern volatile uint32_t ms_ticks;

typedef struct {
    const char *name;
    const char *usage;
    const char *desc;
    void (*handler)(int argc, char **argv);
} cmd_t;

extern const cmd_t cmds[];
extern const size_t num_cmds;

void cmd_help(int argc, char **argv);
void cmd_uptime(int argc, char **argv);
void cmd_echo(int argc, char **argv);
void cmd_led(int argc, char **argv);

#endif