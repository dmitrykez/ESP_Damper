#pragma once
#include "globals.h"
#include <vector>

typedef void (*rx_frame_cb_t)(
  uint8_t ch,
  const rmt_symbol_word_t* symbols,
  size_t num_symbols,
  const String& bits
);

void rx_set_callback(rx_frame_cb_t cb);

extern volatile bool rx_new_data[NUM_CHANNELS];
extern rx_data_t rx_data[NUM_CHANNELS];

void ack_irq_init(uint8_t ch);
void ack_irq_start(uint8_t ch);
void ack_irq_stop(uint8_t ch);
void ack_gpio_init(uint8_t ch);
void ack_gpio_remove(uint8_t ch);

void IRAM_ATTR oneshot_timer_callback(void *arg);
void IRAM_ATTR ack_gpio_isr(void *arg);
bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);

void rmt_rx_channel_config(uint8_t ch, gpio_num_t gpio);
void reconfig_rmt_rx_channel(uint8_t ch, gpio_num_t gpio);
void clear_rx_buffers(uint8_t ch);
bool parseRMTData();
