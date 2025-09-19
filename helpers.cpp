#include "helpers.h"

// Utility: converts binary string to hex string
String binaryToHexGroups(const String &binStr) {
    String padded = binStr;
    while (padded.length() % 8 != 0) padded = "0" + padded;
    String result;
    for (int i = 0; i < padded.length(); i += 8) {
        String byteStr = padded.substring(i, i + 8);
        char byteVal = strtol(byteStr.c_str(), nullptr, 2);
        char buf[4];
        sprintf(buf, "%02X ", (unsigned char)byteVal);
        result += buf;
    }
    return result;
}

// Convert binary string to decimal
uint32_t binToDec(const String &binStr) {
    return strtoul(binStr.c_str(), nullptr, 2);
}

// Validation and parsing logic
bool validateAndParseFrames(std::vector<String> &frames, rx_data_t &rx_data) {
    if (frames.size() != RX_FRAMES) return false;

    int match12 = (frames[0] == frames[1] && frames[0] != DUMMY_FRAME);
    int match13 = (frames[0] == frames[2] && frames[0] != DUMMY_FRAME);
    int match23 = (frames[1] == frames[2] && frames[1] != DUMMY_FRAME);

    String selected;
    if (match12 || match13) selected = frames[0];
    else if (match23) selected = frames[1];
    else {
        Serial.println("No valid matching frames. Ignored.");
        return false;
    }

    Serial.println("Valid frame (binary): " + selected);
    Serial.println("Valid frame (hex): " + binaryToHexGroups(selected));

    if (selected.length() < 56) {
        Serial.println("Frame too short!");
        return false;
    }

    String tempBits = selected.substring(8, 16);
    uint8_t temperature = binToDec(tempBits) + 5;
    bool state = (selected.charAt(17) == '1');
    uint8_t damperAngle = binToDec(selected.substring(18, 20));

    rx_data.temp = temperature;
    rx_data.state = state ? "on" : "off";
    rx_data.fan = damperAngle;

    Serial.printf("Temperature: %d C\n", temperature);
    Serial.printf("State: %s\n", state ? "ON" : "OFF");
    Serial.printf("Damper Angle: %d\n", damperAngle);

    return true;

}
