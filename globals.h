#pragma once
#include "defaults.h"

typedef struct {
    uint8_t temp;
    String state;
    uint8_t fan;
} rx_data_t;

// RX Global buffers and flags
extern rmt_symbol_word_t rx_buffer[NUM_CHANNELS][RX_BUFFER_SIZE];
extern rmt_symbol_word_t rx_data_copy[NUM_CHANNELS][RX_FRAMES][RX_BUFFER_SIZE];
extern size_t rx_data_len[NUM_CHANNELS];
extern volatile bool data_ready[NUM_CHANNELS];
extern size_t rx_data_chunk[NUM_CHANNELS];
extern size_t rx_last_call[NUM_CHANNELS];
extern rmt_channel_handle_t rx_channel[NUM_CHANNELS];

// RX config
extern rmt_receive_config_t rx_config;

// TX Global buffers and flags
extern rmt_channel_handle_t tx_channel[NUM_CHANNELS];
extern rmt_encoder_handle_t copy_encoder[NUM_CHANNELS];
extern size_t tx_last_call[NUM_CHANNELS];
