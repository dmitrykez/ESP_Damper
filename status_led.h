#pragma once
#include <Arduino.h>

#define PWR_LED 16

enum LedMode : uint8_t {
  LED_OFF = 0,
  LED_ON  = 1,
  LED_BLINK = 2
};

void led_task_start();
void led_set_off();
void led_set_on();
void led_set_blink(uint16_t interval_ms);