#pragma once
#include <Arduino.h>
#include <vector>
#include "defaults.h"
#include "globals.h"

void clear_mqtt_data(mqtt_data_t mqtt_data);
String binaryToHexGroups(const String &binStr);
uint32_t binToDec(const String &binStr);
bool validateAndParseFrames(std::vector<String> &frames, rx_data_t &rx_data);
