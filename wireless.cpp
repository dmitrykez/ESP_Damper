#include "wireless.h"
#include "tx.h"
#include "rx.h"
#include <ArduinoJson.h>

#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS "WIFI_PASS"
#define MQTT_SRVR "192.168.1.123"
#define MQTT_PORT 1883

WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

tx_request_t tx_requests[NUM_CHANNELS] = {};
static void reconnect();
static void mqtt_callback(char* topic, byte* payload, unsigned int length);

void wireless_setup() {

    Serial.print("Connecting to: ");
    Serial.print(WIFI_SSID);
    WiFi.setHostname("ESP Damper");
    WiFi.setSleep(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println(" connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
    Serial.println("HTTP server started");
    ElegantOTA.begin(&server);    // Start ElegantOTA
    
    client.setServer(MQTT_SRVR, MQTT_PORT);
    client.setCallback(mqtt_callback);
}

void wireless_loop() {
    if (!client.connected()) {
        reconnect();
        if (client.connected()) Serial.println("Reconnected");
    }
    client.loop();
    ElegantOTA.loop();
}

static void reconnect() {
  int counter = 0;
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("WIFI is not connected: ");
      Serial.println(WiFi.status());
    }

    String clientId = "ESP32-Damper";
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
      client.publish("/home/damper_cmd", "", true); // Clear retained message
      client.unsubscribe("/home/damper_cmd");
      client.subscribe("/home/damper_cmd");
      
    } else {
      Serial.print(" Failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }

    if(counter==200){
      delay(1000);
      Serial.println("Restarting ESP...");
      ESP.restart();
    } else {
      counter++;
    }
  }
}

static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<100> jsonDoc;
  String jsonString;

  for (unsigned int i = 0; i < length; i++) 
      jsonString += (char)tolower(payload[i]);

  deserializeJson(jsonDoc, jsonString);
  
  uint8_t ch=0;
  uint8_t temp=0;
  String state="";
  uint8_t fan=0;

  if (jsonDoc.containsKey("ch")) {
    ch = jsonDoc["ch"].as<uint8_t>();
  }

  if (jsonDoc.containsKey("temp")) {
    temp = jsonDoc["temp"].as<uint8_t>();
  }

  if (jsonDoc.containsKey("state")) {
    state = jsonDoc["state"].as<String>();
    if (state == "off"){
        fan = 0x60;
    }
    else if (state == "on"){
        fan = jsonDoc["fan"].as<uint8_t>();
    }
  }

  if (!tx_requests[ch].pending) {
      // Just store request
      tx_requests[ch].pending = true;
      tx_requests[ch].ch = ch;
      tx_requests[ch].temp = temp;
      tx_requests[ch].state = state;
      tx_requests[ch].fan = fan;
  }
  else
      public_debug_message("Received duplicate command on ch " + String(ch));
}

void public_message(uint8_t ch, uint8_t temp, String state, uint8_t fan) {
  StaticJsonDocument<180> doc;
  char output[180];
  
  doc["ch"] = ch;
  doc["temp"] = temp;
  doc["state"] = state;
  doc["fan"] = fan;

  serializeJson(doc, output);
  Serial.println(output);
  client.publish("/home/damper", output);
}

void public_debug_message(String msg) {
  StaticJsonDocument<180> doc;
  char output[180];
  
  doc["msg"] = msg;
  
  serializeJson(doc, output);
  Serial.println(output);
  client.publish("/home/damper", output);
}
