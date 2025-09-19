#include "wireless.h"
#include "rx.h"
#include "tx.h"
#include "helpers.h"

#include <EEPROM.h>

#define VERSION "1.0.1"

#define PWR_LED 16
#define EEPROM_SIZE 1

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.print("Current FW version: ");
    Serial.println(VERSION);
    Serial.println("Starting RMT RX/TX multi channel...");
    pinMode(PWR_LED, OUTPUT);
    
    EEPROM.begin(EEPROM_SIZE);  

    wireless_setup();

    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        rmt_rx_channel_config(ch, CHANNEL_GPIOS[ch]);
        rx_last_call[ch] = micros();
    }
    digitalWrite(PWR_LED, LOW);
}

void loop() {
    if (parseRMTData()) {
        for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
            if (rx_new_data[ch]) {
                rx_new_data[ch] = false;
                public_message(ch, rx_data[ch].temp, rx_data[ch].state, rx_data[ch].fan);
            }
        }
    }

    // MQTT -> TX send
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        if (tx_requests[ch].pending) {
            tx_requests[ch].pending = false;

            send_tx_payload(ch, CHANNEL_GPIOS[ch], tx_requests[ch].temp, tx_requests[ch].state == "on", tx_requests[ch].fan);

            // After TX, re-enable RX
            reconfig_rmt_rx_channel(ch, CHANNEL_GPIOS[ch]);
            tx_last_call[ch] = micros();

            public_message(ch, tx_requests[ch].temp, tx_requests[ch].state, tx_requests[ch].fan);

            Serial.printf("TX sent on ch %d (temp=%d, state=%s, fan=%d)\n",
                tx_requests[ch].ch,
                tx_requests[ch].temp,
                tx_requests[ch].state.c_str(),
                tx_requests[ch].fan);
        }
    }

    wireless_loop();
}