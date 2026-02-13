// Latest ElegantOTA requires manual configuration to work in AsyncWebServer mode:
// Add in ElegantOTA.h:
//    #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
//
// To get to update page by direct IP address:
// Remove "update" in ElegantOTA.cpp:
//    _server->on("/update", HTTP_GET, [&](AsyncWebServerRequest *request)


#include "wireless.h"
#include "rx.h"
#include "tx.h"
#include "helpers.h"
#include "config.h"
#include "reset.h"

#define VERSION "1.1.10"

const char* FW_VERSION_STR = VERSION;
bool single_shot = true;
mqtt_data_t mqtt_data[NUM_CHANNELS];

void setup() {
    Serial.begin(115200);
    delay(500);
    
    led_task_start();
    led_set_blink(250);

    config_begin();
    config_load();
    
    reset_task_start();

    Serial.println();
    Serial.print("Current FW version: ");
    Serial.println(VERSION);
    Serial.println("Starting WiFi...");
        
    wireless_setup();

    Serial.println("Starting RMT RX/TX multi channel...");
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        rmt_rx_channel_config(ch, CHANNEL_GPIOS[ch]);
        ack_irq_init(ch);
        rx_last_call[ch] = micros();
    }
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    
    // uint8_t g_channel_offset = device_config.extended_channels ? 4 : 0;
    Serial.print("Device serving channels: ");
    Serial.println(device_config.extended_channels ? "4-7" : "0-3");

    led_set_on();
}

void loop() {
    wireless_loop();

    // Handle RX -> MQTT transaction 
    if (parseRMTData()) {
        for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
            if (rx_new_data[ch]) {
                rx_new_data[ch] = false;
                ack_irq_start(ch);
                ack_gpio_init(ch);
                
                mqtt_data[ch].ch = ch + device_config.extended_channels * 4;
                mqtt_data[ch].temp = rx_data[ch].temp;
                mqtt_data[ch].state = rx_data[ch].state;
                mqtt_data[ch].fan = rx_data[ch].fan;
                mqtt_data[ch].pending = true;
            }
        }
    }
         
    // Handle MQTT -> TX transaction
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        if (tx_requests[ch].pending && !mqtt_data[ch].pending) {         
            send_tx_payload(ch, CHANNEL_GPIOS[ch], tx_requests[ch].temp, tx_requests[ch].state == "on", tx_requests[ch].fan);

            reconfig_rmt_rx_channel(ch, CHANNEL_GPIOS[ch]);
            ack_irq_start(ch);
            ack_gpio_init(ch);
            
            mqtt_data[ch].ch = ch + device_config.extended_channels * 4;
            mqtt_data[ch].temp = tx_requests[ch].temp;
            mqtt_data[ch].state = tx_requests[ch].state;
            mqtt_data[ch].fan = tx_requests[ch].fan;
            mqtt_data[ch].pending = true;
        }
    }

    // Handle ACK signal and send MQTT message
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        if (mqtt_data[ch].pending && ack_active[ch]){
            public_message(mqtt_data[ch].ch, mqtt_data[ch].temp, mqtt_data[ch].state, mqtt_data[ch].fan);
            tx_requests[ch].pending = false;
            ack_active[ch] = false;
            mqtt_data[ch].pending = false;
        }
    }

    // Handle NACK signal
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ++ch) {
        if (ack_timeout[ch]) {
            public_debug_message("Ch " + String(ch + device_config.extended_channels * 4) + " command failed");
            mqtt_data[ch].pending = false;
            tx_requests[ch].pending = false;
            ack_timeout[ch] = false;
        }
    }
 
    // Send MQTT message when ESP has booted
    if (single_shot){
        public_debug_message("ESP boot done, version " + String(VERSION));
        single_shot = false;
    }
}