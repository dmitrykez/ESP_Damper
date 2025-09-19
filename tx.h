#pragma once
#include "globals.h"

void rmt_tx_channel_config(uint8_t ch, gpio_num_t gpio);
rmt_symbol_word_t make_sym(uint8_t level0, uint16_t dur0, uint8_t level1, uint16_t dur1);
void send_dummy_frame(uint8_t ch);
uint8_t compute_trailer(const uint8_t *data, size_t len);
void build_frame_symbols(const uint8_t frame[7], rmt_symbol_word_t* out, size_t& out_count);
void send_tx_payload(uint8_t ch, gpio_num_t gpio, uint8_t temperature, bool state, uint8_t fan);
