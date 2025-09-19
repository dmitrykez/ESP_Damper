#include "rx.h"
#include "helpers.h"

rmt_symbol_word_t rx_buffer[NUM_CHANNELS][RX_BUFFER_SIZE];
rmt_symbol_word_t rx_data_copy[NUM_CHANNELS][RX_FRAMES][RX_BUFFER_SIZE];
size_t rx_data_len[NUM_CHANNELS] = {0};
volatile bool data_ready[NUM_CHANNELS] = {false};
size_t rx_data_chunk[NUM_CHANNELS] = {0};
size_t rx_last_call[NUM_CHANNELS] = {0};
rmt_channel_handle_t rx_channel[NUM_CHANNELS] = {NULL};
rx_data_t rx_data[NUM_CHANNELS] = {};
volatile bool rx_new_data[NUM_CHANNELS] = {};

rmt_receive_config_t rx_config = {
    .signal_range_min_ns = 2000,
    .signal_range_max_ns = 2500000,
};

bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data) {
    int ch = (int)(intptr_t)user_data;
    if (ch < 0 || ch >= NUM_CHANNELS) return false;

    size_t count = edata->num_symbols;
    if (micros() - rx_last_call[ch] > FRAME_TIMEOUT_US) clear_rx_buffers(ch);
    if (count > RX_BUFFER_SIZE) count = RX_BUFFER_SIZE;
    if (count == 57) {
        memcpy(rx_data_copy[ch][rx_data_chunk[ch]], edata->received_symbols, count * sizeof(rmt_symbol_word_t));
        rx_data_len[ch] = count;
        rx_data_chunk[ch]++;
        if (rx_data_chunk[ch] == RX_FRAMES) { rx_data_chunk[ch] = 0; data_ready[ch] = true; }
    }

    rmt_receive(rx_channel[ch], rx_buffer[ch], sizeof(rx_buffer[ch]), &rx_config);
    rx_last_call[ch] = micros();
    return true;
}

void rmt_rx_channel_config(uint8_t ch, gpio_num_t gpio) {
    if (tx_channel[ch]) {
        rmt_disable(tx_channel[ch]);
        rmt_del_channel(tx_channel[ch]);
        tx_channel[ch] = NULL;
    }

    pinMode(gpio, INPUT);

    rmt_rx_channel_config_t rx_channel_cfg = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .mem_block_symbols = RX_BUFFER_SIZE,
    };

    rmt_rx_event_callbacks_t cbs = { .on_recv_done = rmt_rx_done_callback };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel[ch]));
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel[ch], &cbs, (void*)(intptr_t)ch));
    ESP_ERROR_CHECK(rmt_enable(rx_channel[ch]));
    ESP_ERROR_CHECK(rmt_receive(rx_channel[ch], rx_buffer[ch], sizeof(rx_buffer[ch]), &rx_config));

    Serial.printf("Configured RX channel %u: handle=%d gpio=%d\n", (unsigned)ch, int(rx_channel[ch]), int(gpio));
}

void clear_rx_buffers(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return;
    rx_data_chunk[ch] = 0;
    memset(rx_data_copy[ch], 0, sizeof(rx_data_copy[ch]));
}

void reconfig_rmt_rx_channel(uint8_t ch, gpio_num_t gpio) {
    // Tear down existing and re-create
    if (ch >= NUM_CHANNELS) return;
    if (rx_channel[ch]) {
        ESP_ERROR_CHECK(rmt_disable(rx_channel[ch]));
        ESP_ERROR_CHECK(rmt_del_channel(rx_channel[ch]));
        rx_channel[ch] = NULL;
    }
    rmt_rx_channel_config(ch, gpio);
}

bool parseRMTData() {
    bool data_received = false;

    // Check each channel for ready data
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        if (!data_ready[ch]) continue;

        size_t len = rx_data_len[ch];
        rmt_symbol_word_t symbols[RX_BUFFER_SIZE];
        std::vector<String> frames;
        for(int chunk=0; chunk<RX_FRAMES; chunk++) {
            noInterrupts();
            memcpy(symbols, rx_data_copy[ch][chunk], len * sizeof(rmt_symbol_word_t));
            data_ready[ch] = false;
            interrupts();

            int i = 0;
            String bits = "";
            int level0 = symbols[i].level1;
            int level1 = symbols[i].level0;
            int duration0 = symbols[i].duration1;
            int duration1 = symbols[i].duration0;

            if (level0 == 0 && duration0 >= HEADER_LOW_MIN && duration0 <= HEADER_LOW_MAX &&
                level1 == 1 && duration1 >= HEADER_HIGH_MIN && duration1 <= HEADER_HIGH_MAX) {
                i++;
                bits = "";
                Serial.printf("Ch %d chunk %d\n", ch, chunk);
                while (i < (int)len) {
                    level0 = symbols[i].level1;
                    level1 = symbols[i].level0;
                    duration0 = symbols[i].duration1;
                    duration1 = symbols[i].duration0;
                    if (duration1 >= BIT_HIGH_0_MIN && duration1 <= BIT_HIGH_0_MAX) {
                        bits += "0";
                    } else if (duration1 >= BIT_HIGH_1_MIN && duration1 <= BIT_HIGH_1_MAX) {
                        bits += "1";
                    } else {
                        Serial.printf("Framing parsing error at location: %d\n", i);
                        Serial.printf("Low: %d\n", duration0);
                        Serial.printf("High: %d\n", duration1);
                        i++;
                        continue;
                    }
                    i++;
                }
            }

            if (bits.length() > 0) frames.push_back(bits);
            else {
                frames.push_back(DUMMY_FRAME);
                Serial.printf("Ch %d broken chunk diagnostics: level0=%d level1=%d dur0=%d dur1=%d\n", ch, level0, level1, duration0, duration1);
            }
        }

        if (frames.size() == RX_FRAMES) {
            Serial.printf("Complete frame decoded (ch %u):\n", (unsigned)ch);
            Serial.println("Frame 1: " + frames[0] + " (" + binaryToHexGroups(frames[0]) + ")");
            Serial.println("Frame 2: " + frames[1] + " (" + binaryToHexGroups(frames[1]) + ")");
            Serial.println("Frame 3: " + frames[2] + " (" + binaryToHexGroups(frames[2]) + ")");
            bool res = validateAndParseFrames(frames, rx_data[ch]);
            if (res) {
              rx_new_data[ch] = true;
              data_received = true;
            }
            frames.clear();
        } else {
            Serial.printf("Invalid frames count found on ch %u!\n", (unsigned)ch);
            frames.clear();
        }
        clear_rx_buffers(ch);
    } // end channels loop
    return data_received;
}
