#pragma once
#include <Arduino.h>

typedef struct {
    char device_name[32];
    char wifi_ssid[32];
    char wifi_pass[64];
    char mqtt_server[64];
    uint16_t mqtt_port;
    char mqtt_topic_rx[64];
    char mqtt_topic_tx[64];
    bool dark_mode;
    bool extended_channels;
    char mqtt_user[32];
    char mqtt_pass[64];
    bool debug_verbose;
} device_config_t;

extern device_config_t device_config;

void config_begin();
void config_load();
void config_save();
void config_set_defaults();
void config_clear();
