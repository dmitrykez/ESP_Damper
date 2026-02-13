#include "wireless.h"
#include "tx.h"
#include "rx.h"
#include "web.h"
#include <ArduinoJson.h>

#define AP_IP       IPAddress(192, 168, 50, 1)
#define AP_GATEWAY  IPAddress(192, 168, 50, 1)
#define AP_SUBNET   IPAddress(255, 255, 255, 0)

WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

tx_request_t tx_requests[NUM_CHANNELS] = {};
static void reconnect();
static void mqtt_callback(char* topic, byte* payload, unsigned int length);

void wireless_setup() {
    bool wifi_sta_mode = true;
    if (device_config.wifi_ssid[0] == '\0')
        wifi_sta_mode = false;
  
    if(wifi_sta_mode)
        start_sta_mode();
    else
        start_ap_mode();

    web_begin(server, client);
    server.begin();
    Serial.println("HTTP server started");
    ElegantOTA.begin(&server);    // Start ElegantOTA
    
    if(wifi_sta_mode) {
        led_set_blink(100);
        client.setServer(device_config.mqtt_server, device_config.mqtt_port);
        client.setCallback(mqtt_callback);
    }
}

void wireless_loop() {
    if(WiFi.getMode() == WIFI_AP) 
        led_set_blink(1000);
    else {
        if (!client.connected()) {
            reconnect();
            if (client.connected()) Serial.println("Reconnected");
        }
        client.loop();
    }
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

    String clientId = device_config.device_name;
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
      client.unsubscribe(device_config.mqtt_topic_rx);
      client.subscribe(device_config.mqtt_topic_rx);
      
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
    if (device_config.extended_channels)
      ch = jsonDoc["ch"].as<uint8_t>() - device_config.extended_channels * 4;
    else
      ch = jsonDoc["ch"].as<uint8_t>();
    if (ch >= 4) return;
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
      public_debug_message("Received duplicate command on ch " + String(ch + device_config.extended_channels * 4));
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
  client.publish(device_config.mqtt_topic_tx, output);
}

void public_debug_message(String msg) {
  StaticJsonDocument<180> doc;
  char output[180];
  
  doc["device"] = device_config.device_name;
  doc["msg"] = msg;
  
  serializeJson(doc, output);
  Serial.println(output);
  client.publish(device_config.mqtt_topic_tx, output);
}

void start_ap_mode() {
    Serial.println("No WiFi SSID configured, entering AP mode");
    WiFi.mode(WIFI_AP);
    // Set static IP for AP mode
    if (!WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET)) {
        Serial.println("Failed to configure AP IP");
    }

    WiFi.softAP("ESP_Damper_Setup");

    Serial.println("AP mode started");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    led_set_blink(1000);   // Slow blink in AP mode
}

void start_sta_mode() {
    Serial.print("Connecting to: ");
    Serial.print(device_config.wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(device_config.device_name);
    WiFi.setSleep(true);
    WiFi.begin(device_config.wifi_ssid, device_config.wifi_pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println(" connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}