#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

extern const char* FW_VERSION_STR;

void web_begin(AsyncWebServer& server, PubSubClient& mqttClient);
