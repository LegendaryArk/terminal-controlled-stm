#include "led.h"

led_t led = { GPIOA, LED_PIN, 0, 0 };

void led_on(led_t *led) {
    led->delay_ms = 0;
    led->port->BSRR = led->pin;
}

void led_off(led_t *led) {
    led->delay_ms = 0;
    led->port->BSRR = led->pin << 16;
}

void led_toggle(led_t *led) {
    led->delay_ms = 0;
    uint32_t odr = led->port->ODR;
    led->port->BSRR = ((odr & led->pin) << 16) | (~odr & led->pin);
}

void led_blink(led_t *led, uint32_t delay_ms) {
    led->delay_ms = delay_ms;
    led->prev_time = ms_ticks;
}

void led_update(led_t *led) {
    uint32_t now = ms_ticks;
    if (led->delay_ms > 0 && now - led->prev_time >= led->delay_ms) {
        led->prev_time += led->delay_ms;
        uint32_t odr = led->port->ODR;
        led->port->BSRR = ((odr & led->pin) << 16) | (~odr & led->pin);
    }
}
