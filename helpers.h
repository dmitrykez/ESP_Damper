#pragma once
#include <Arduino.h>
#include <vector>
#include "defaults.h"
#include "globals.h"

String binaryToHexGroups(const String &binStr);
uint32_t binToDec(const String &binStr);
bool validateAndParseFrames(std::vector<String> &frames, rx_data_t &rx_data);
