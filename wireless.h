#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "defaults.h"
#include "config.h"
#include "status_led.h"

typedef struct {
    bool pending;
    uint8_t ch;
    uint8_t temp;
    String state;
    uint8_t fan;
} tx_request_t;

extern tx_request_t tx_requests[NUM_CHANNELS];

extern WiFiClient espClient;
extern PubSubClient client;
extern AsyncWebServer server;

void wireless_setup();
void wireless_loop();
void public_message(uint8_t ch, uint8_t temp, String state, uint8_t fan);
void public_debug_message(String msg);
void start_ap_mode();
void start_sta_mode();