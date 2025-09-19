#pragma once
#include "globals.h"
#include <vector>

extern volatile bool rx_new_data[NUM_CHANNELS];
extern rx_data_t rx_data[NUM_CHANNELS];

bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);

void rmt_rx_channel_config(uint8_t ch, gpio_num_t gpio);
void reconfig_rmt_rx_channel(uint8_t ch, gpio_num_t gpio);
void clear_rx_buffers(uint8_t ch);
bool parseRMTData();
