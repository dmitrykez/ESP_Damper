#include "reset.h"
#include "config.h"


static inline bool pressed_now() {
  int v = digitalRead(RESET_GPIO);
  return (v == LOW);
}

static void resetTask(void *param) {
  (void)param;

  uint32_t pressed_start = 0;
  bool was_pressed = false;

  for (;;) {
    bool pressed = pressed_now();

    if (pressed && !was_pressed) {
      was_pressed = true;
      pressed_start = millis();
    } else if (!pressed && was_pressed) {
      was_pressed = false;
      pressed_start = 0;
    }

    if (pressed && was_pressed && pressed_start != 0) {
      if (millis() - pressed_start >= RESET_HOLD_MS) {
        Serial.println("Factory reset: clearing config and rebooting...");
        config_clear();     
        delay(150);
        ESP.restart();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));  // check every 20ms
  }
}

void reset_task_start() {
  pinMode(RESET_GPIO, INPUT_PULLUP);
  xTaskCreate(resetTask, "ResetTask", 3072, nullptr, 3, nullptr);
}
