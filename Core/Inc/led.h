#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f446xx.h"

#define LED_PIN (1 << 5)

extern volatile uint32_t ms_ticks;

typedef struct {
    GPIO_TypeDef *port;
    const uint32_t pin;
    uint32_t delay_ms;
    uint32_t prev_time;
} led_t;

extern led_t led;

void led_on(led_t *led);
void led_off(led_t *led);
void led_toggle(led_t *led);
void led_blink(led_t *led, uint32_t delay_time);
void led_update(led_t *led);

#endif