#include "config.h"
#include <Preferences.h>

Preferences prefs;
device_config_t device_config;

void config_set_defaults() {
    strcpy(device_config.device_name, "ESP Damper");
    strcpy(device_config.wifi_ssid, "");
    strcpy(device_config.wifi_pass, "");
    strcpy(device_config.mqtt_server, "192.168.1.123");
    device_config.mqtt_port = 1883;
    strcpy(device_config.mqtt_topic_rx, "/home/damper_cmd");
    strcpy(device_config.mqtt_topic_tx, "/home/damper");
    device_config.dark_mode = false;
    device_config.extended_channels = false;
}

void config_begin() {
    prefs.begin("config", false);
}

void config_load() {
    if (!prefs.getBool("valid", false)) {
        config_set_defaults();
        config_save();
        return;
    }

    prefs.getBytes("cfg", &device_config, sizeof(device_config));
}

void config_save() {
    prefs.putBytes("cfg", &device_config, sizeof(device_config));
    prefs.putBool("valid", true);
}

void config_clear() {
    // Wipe the entire "config" namespace
    prefs.clear();
    prefs.putBool("valid", false);
}
