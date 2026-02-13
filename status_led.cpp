#include "status_led.h"

static volatile LedMode g_mode = LED_OFF;
static volatile uint16_t g_interval_ms = 500;

static inline void write_led(bool on) {
  digitalWrite(PWR_LED, (on ? LOW : HIGH));
}

static void ledTask(void *param) {
  (void)param;

  bool state = false;

  for (;;) {
    LedMode mode = g_mode;

    if (mode == LED_OFF) {
      write_led(false);
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    if (mode == LED_ON) {
      write_led(true);
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    // LED_BLINK
    uint16_t interval = g_interval_ms;
    state = !state;
    write_led(state);
    vTaskDelay(pdMS_TO_TICKS(interval));
  }
}

void led_task_start() {
  pinMode(PWR_LED, OUTPUT);
  write_led(false);

  // Priority 2 is enough; stack 2048 is fine for this tiny task
  xTaskCreate(ledTask, "LedTask", 2048, nullptr, 2, nullptr);
}

void led_set_off() {
  g_mode = LED_OFF;
}

void led_set_on() {
  g_mode = LED_ON;
}

void led_set_blink(uint16_t interval_ms) {
  if (interval_ms == 0) interval_ms = 100;
  g_interval_ms = interval_ms;
  g_mode = LED_BLINK;
}
