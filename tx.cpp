#include "tx.h"

rmt_channel_handle_t tx_channel[NUM_CHANNELS] = { NULL };
rmt_encoder_handle_t copy_encoder[NUM_CHANNELS] = { NULL };

void rmt_tx_channel_config(uint8_t ch, gpio_num_t gpio) {
    if (ch >= NUM_CHANNELS) return;

    if (rx_channel[ch]) {
        Serial.printf("Removing RX ch: %d \n", (int)rx_channel);
        ESP_ERROR_CHECK(rmt_disable(rx_channel[ch]));
        ESP_ERROR_CHECK(rmt_del_channel(rx_channel[ch]));
        rx_channel[ch] = NULL;
    }

    pinMode(gpio, OUTPUT_OPEN_DRAIN);

    rmt_tx_channel_config_t tx_config = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .mem_block_symbols = RX_BUFFER_SIZE,
        .trans_queue_depth = 4,
        .flags = { .invert_out = false, .with_dma = false, .io_loop_back = false, .io_od_mode = true },
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &tx_channel[ch]));

    rmt_copy_encoder_config_t cp_cfg = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&cp_cfg, &copy_encoder[ch]));
    ESP_ERROR_CHECK(rmt_enable(tx_channel[ch]));
    //send_dummy_frame();

    Serial.printf("Configured TX channel %u: handle=%d gpio=%d\n", (unsigned)ch, int(tx_channel[ch]), int(gpio));

}

rmt_symbol_word_t make_sym(uint8_t level0, uint16_t dur0, uint8_t level1, uint16_t dur1) {
  rmt_symbol_word_t s;
  s.level0 = level0;
  s.duration0 = dur0;
  s.level1 = level1;
  s.duration1 = dur1;
  return s;
}

void send_dummy_frame(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return;
    if (!tx_channel[ch] || !copy_encoder[ch]) return;

    rmt_symbol_word_t dummy = make_sym(1, 0, 1, 0);
    rmt_transmit_config_t tx_cfg = { .loop_count = 0, .flags = { .eot_level = 0 } };
    ESP_ERROR_CHECK(rmt_transmit(tx_channel[ch], copy_encoder[ch], &dummy, sizeof(rmt_symbol_word_t), &tx_cfg));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_channel[ch], 3000));
    Serial.printf("Dummy frame sent on ch %u\n", (unsigned)ch);
}

// --- Parity bit (trailer byte) calculation ---
uint8_t compute_trailer(const uint8_t *data, size_t len) {
  uint32_t total = 0;
  for (size_t i = 0; i < len; i++) {
      uint8_t byte = data[i];
      while (byte) {
          total += byte & 1;   // add lowest bit
          byte >>= 1;          // shift right
      }
  }
  // If ODD -> use 0x42, else 0x4A
  return (total % 2) ? 0x4A : 0x42;
}

// === Build full frame symbols ===
void build_frame_symbols(const uint8_t frame[7], rmt_symbol_word_t* out, size_t& out_count) {
  uint8_t temp[7];
  memcpy(temp, frame, 6);
  temp[6] = compute_trailer(frame, 6);
  
  Serial.print("Payload 0x");
  for (int i = 0; i <= 6; i++) {
    Serial.printf("%02X", temp[i]);
  }
  Serial.println();


  // merge into 56-bit payload (MSB first)
  uint64_t payload = 0;
  for (int i=0; i<7; i++) {
    payload = (payload << 8) | temp[i];
  }

  size_t idx = 0;
  for (int rep = 0; rep < 3; ++rep) {
    if (idx == 0)
      out[idx++] = make_sym(0, HEADER_LOW_US-1300, 1, HEADER_HIGH_US); // First pulse always longer by 1.3ms
    else
      out[idx++] = make_sym(0, HEADER_LOW_US, 1, HEADER_HIGH_US);
    for (int b = 55; b >= 0; --b) {
      bool bit = (payload >> b) & 0x1;
      if (bit) {
        out[idx++] = make_sym(0, BIT1_LOW_US, 1, BIT1_HIGH_US);
      } else {
        out[idx++] = make_sym(0, BIT0_LOW_US, 1, BIT0_HIGH_US);
      }
    }
  }
  // Footer
  out[idx++] = make_sym(0, FOOTER_LOW_US, 1, 500);
  out_count = idx;
}

void send_tx_payload(uint8_t ch, gpio_num_t gpio, uint8_t temperature, bool state, uint8_t fan) {
    if (ch >= NUM_CHANNELS) return;

    rmt_symbol_word_t syms[174];
    size_t n_syms = 0;

    if (!tx_channel[ch]) rmt_tx_channel_config(ch, gpio);

    temperature = temperature - 5;
    uint8_t status = state ? (state << 6) | (fan << 4) : 0x30;
    

    uint8_t DEMO_PAYLOAD[7] = {0x14, temperature, status, 0xF4, 0xFF, 0xFF, 0x00};
    build_frame_symbols(DEMO_PAYLOAD, syms, n_syms);

    rmt_transmit_config_t tx_cfg = { .loop_count = 0, .flags = { .eot_level = 1 } };

    ESP_ERROR_CHECK(rmt_transmit(tx_channel[ch], copy_encoder[ch], syms, n_syms * sizeof(rmt_symbol_word_t), &tx_cfg));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_channel[ch], 3000));

    Serial.printf("Sent one frame on ch %u\n", (unsigned)ch);
    delay(100);
}