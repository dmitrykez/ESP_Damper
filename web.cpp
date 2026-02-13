#include "web.h"
#include "config.h"
#include "logo.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// --------- Helpers ---------
static String htmlEscape(const String& in) {
  String out;
  out.reserve(in.length() + 16);
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    switch (c) {
      case '&': out += F("&amp;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      case '"': out += F("&quot;"); break;
      case '\'': out += F("&#39;"); break;
      default: out += c; break;
    }
  }
  return out;
}

static String valOrEmpty(const char* stored) {
  if (stored && stored[0] != '\0') return htmlEscape(String(stored));
  return String("");
}

static String valPortOrEmpty(uint16_t port) {
  if (port != 0) return String(port);
  return String("");
}

static String wifiStatusStr() {
  switch (WiFi.status()) {
    case WL_CONNECTED: return "connected";
    case WL_NO_SSID_AVAIL: return "no_ssid";
    case WL_CONNECT_FAILED: return "failed";
    case WL_DISCONNECTED: return "disconnected";
    default: return "unknown";
  }
}

// --------- Page builder ---------
static String buildConfigPageHtml() {
  // Editable values
  String val_dev  = valOrEmpty(device_config.device_name);
  String val_ssid = valOrEmpty(device_config.wifi_ssid);
  String val_mqtt = valOrEmpty(device_config.mqtt_server);
  String val_port = valPortOrEmpty(device_config.mqtt_port);
  String val_rx   = valOrEmpty(device_config.mqtt_topic_rx);
  String val_tx   = valOrEmpty(device_config.mqtt_topic_tx);

  // Password: never show stored value; leaving empty means unchanged
  String pass_placeholder = (device_config.wifi_pass[0] != '\0')
                              ? String("********")
                              : String("WiFi password");

  String html;
  html.reserve(9500);

  html += F(
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>ESP Damper</title>"
    "<link rel='icon' type='image/png' href='/logo.png?v=2'>"
    "<link rel='apple-touch-icon' href='/logo.png?v=2'>"
    "<style>"
    ":root{--bg:#ffffff;--card:#f7f7f8;--border:#e5e5e7;--text:#1c1c1e;--muted:#8e8e93;--accent:#0a84ff;}"
    "[data-theme='dark']{--bg:#0b0b0c;--card:#161618;--border:#2a2a2d;--text:#f2f2f7;--muted:#a1a1a6;--accent:#0a84ff;}"
    "*{box-sizing:border-box;}body{margin:0;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:var(--bg);color:var(--text);}"
    ".container{max-width:420px;margin:0 auto;padding:24px 16px 40px;}"
    ".logo{display:block;max-width:350px;margin:8px auto 24px;}"
    ".section{margin-bottom:16px;}"
    ".field{background:var(--card);border:1px solid var(--border);border-radius:16px;padding:12px 14px;margin-bottom:12px;}"
    ".field label{display:block;font-size:12px;color:var(--muted);margin-bottom:4px;}"
    ".field input{width:100%;border:none;outline:none;background:transparent;font-size:16px;color:var(--text);}"
    ".button{width:100%;height:48px;border-radius:24px;border:none;font-size:16px;font-weight:600;cursor:pointer;}"
    ".button.primary{background:var(--accent);color:white;margin-top:8px;}"
    ".button.secondary{background:var(--card);color:var(--text);border:1px solid var(--border);margin-bottom:6px;}"
    ".status{background:var(--card);border:1px solid var(--border);border-radius:16px;padding:14px;margin-bottom:16px;}"
    ".chip{display:inline-block;padding:8px 12px;border-radius:999px;border:1px solid var(--border);background:var(--bg);font-weight:600;font-size:13px;margin-right:8px;}"
    ".chip.ok{background:#58a86c;color:#0b0b0c;border-color:#43734f;}"
    ".chip.err{background:#d64a42;color:#ffffff;border-color:#963732;}"
    "[data-theme='dark'] .chip.ok{color:#0b0b0c;}"
    ".sub{margin-top:10px;color:var(--muted);font-size:12px;word-break:break-word;}"
    ".switch{position:relative;display:inline-block;width:54px;height:32px;}"
    ".switch input{display:none;}"
    ".slider{position:absolute;cursor:pointer;inset:0;background:#797979;border-radius:999px;transition:.2s;}"
    ".slider:before{content:'';position:absolute;height:26px;width:26px;left:3px;top:3px;background:white;border-radius:999px;transition:.2s;box-shadow:0 2px 10px rgba(0,0,0,.12);}"
    ".switch input:checked + .slider{background:var(--accent);}"
    ".switch input:checked + .slider:before{transform:translateX(22px);}"
    "input[type='radio']{accent-color: var(--accent);width: 26px;height: 26px;}"
    ".footer{margin-top: 24px;text-align: center;font-size: 12px;color: var(--muted);opacity: 0.8;}"
    "</style></head><body><div class='container'>"
  );

  // Logo (switched by JS)
  html += F("<img class='logo' id='logoImg' src='/logo.png' alt='ESP Damper'>");

  html += F("<form action='/save' method='POST' enctype='application/x-www-form-urlencoded'>");

  // Config fields
  html += F("<div class='section'>");

  html += F("<div class='field'><label>Device Name</label><input name='devname' value='");
  html += val_dev;
  html += F("' placeholder='ESP-Damper'></div>");

  html += F("<div class='field'><label>WiFi SSID</label><input name='ssid' value='");
  html += val_ssid;
  html += F("' placeholder='Network name'></div>");

  html += F("<div class='field'><label>WiFi Password</label><input name='pass' type='password' value='' placeholder='");
  html += htmlEscape(pass_placeholder);
  html += F("'></div>");

  html += F("<div class='field'><label>MQTT Server</label><input name='mqtt' value='");
  html += val_mqtt;
  html += F("' placeholder='192.168.1.123'></div>");

  html += F("<div class='field'><label>MQTT Port</label><input name='port' value='");
  html += val_port;
  html += F("' placeholder='1883'></div>");

  html += F("<div class='field'><label>MQTT RX Topic</label><input name='rx' value='");
  html += val_rx;
  html += F("' placeholder='/home/damper_cmd'></div>");

  html += F("<div class='field'><label>MQTT TX Topic</label><input name='tx' value='");
  html += val_tx;
  html += F("' placeholder='/home/damper'></div>");

  // Status panel
  html += F(
    "<div class='status'>"
    "<div class='chip' id='wifiChip'>WiFi: ...</div>"
    "<div class='chip' id='mqttChip'>MQTT: ...</div>"
    "<div class='sub' id='infoLine1'>...</div>"
    "<div class='sub' id='infoLine2'>...</div>"
    "</div>"
  );

  html += F(
    "<div class='section'>"
    "<div class='field'>"
    "<label>Channel Range</label>"
    "<div style='display:flex;gap:12px;'>"

    "<label style='display:flex;align-items:center;gap:6px;color:var(--text);'>"
    "<input type='radio' name='ch_range' value='low' ");
  html += device_config.extended_channels ? "" : "checked";
  html += F(
    "> Channels 0-3</label>"
    "<label style='display:flex;align-items:center;gap:6px;color:var(--text);'>"
    "<input type='radio' name='ch_range' value='high' ");
  html += device_config.extended_channels ? "checked" : "";
  html += F("> Channels 4-7</label>"
    "</div></div></div>"
  );

  // Appearance toggle MUST be before script, so element exists
  html += F(
    "<div class='section'>"
    "<div class='field' style='display:flex;align-items:center;justify-content:space-between;'>"
    "<div><label>Dark UI</label><div style='font-size:16px;font-weight:400;'>Appearance</div></div>"
    "<label class='switch'><input id='darkToggle' type='checkbox'><span class='slider'></span></label>"
    "</div></div>"
  );

  html += F("<button class='button primary' type='submit'>Save & Reboot</button>");
  
  html += F("</div>"); // section
  html += F("</form>");

  html += F("<button class='button secondary' onclick=\"location.href='/update'\">Update Firmware</button>");

  html += F(
    "<button class='button secondary' style='margin-top:12px;border-color:#963732;color:#fff;background:#d64a42;' "
    "onclick=\"if(confirm('Factory reset, really? This will erase WiFi/MQTT settings and reboot.')) location.href='/reset';\">"
    "Factory Reset</button>"
  );
 
  // JS: theme + logo + status polling
  html += F(
    "<script>"
    "function applyTheme(d){document.documentElement.setAttribute('data-theme', d?'dark':'light');}"
    "let dark=(localStorage.getItem('ui_dark')==='1');"
    "applyTheme(dark);"
    "const t=document.getElementById('darkToggle');"
    "t.checked=dark;"
    "t.addEventListener('change',()=>{"
    "const v=t.checked?'1':'0';"
    "localStorage.setItem('ui_dark',v);"
    "applyTheme(t.checked);"
    "fetch('/theme?dark='+v);"
    "});"
    "async function refreshStatus(){"
    "try{const r=await fetch('/status',{cache:'no-store'});const s=await r.json();"
    "const wifiOk=(s.wifi==='connected')||(s.mode==='ap');"
    "const mqttOk=!!s.mqtt_connected;"
    "const wifiChip=document.getElementById('wifiChip');"
    "const mqttChip=document.getElementById('mqttChip');"
    "wifiChip.textContent=`WiFi: ${wifiOk?'OK':'ERR'} (${s.mode.toUpperCase()})`;"
    "wifiChip.classList.toggle('ok',wifiOk);"
    "wifiChip.classList.toggle('err',!wifiOk);"
    "mqttChip.textContent=`MQTT: ${mqttOk?'OK':'ERR'}`;"
    "mqttChip.classList.toggle('ok',mqttOk);"
    "mqttChip.classList.toggle('err',!mqttOk);"
    "document.getElementById('infoLine1').textContent=`WiFi SSID: ${s.ssid} | IP: ${s.ip} | RSSI: ${s.rssi} dBm`;"
    "document.getElementById('infoLine2').textContent=`MQTT: ${s.mqtt_server}:${s.mqtt_port} | State: ${s.mqtt_state}`;"
    "}catch(e){"
    "const wifiChip=document.getElementById('wifiChip');"
    "const mqttChip=document.getElementById('mqttChip');"
    "wifiChip.textContent='WiFi: ERR';"
    "mqttChip.textContent='MQTT: ERR';"
    "wifiChip.classList.add('err');wifiChip.classList.remove('ok');"
    "mqttChip.classList.add('err');mqttChip.classList.remove('ok');"
    "document.getElementById('infoLine1').textContent='Status unavailable';"
    "}}"
    "refreshStatus();setInterval(refreshStatus,1000);"
    "</script>"
  );

  html += F("<div class='footer'>FW version: ");
  html += FW_VERSION_STR;
  html += F("</div>");

  html += F("</div></body></html>");

  return html;
}

// --------- Route registrations ---------
void web_begin(AsyncWebServer& server, PubSubClient& mqttClient) {

    server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response =
        request->beginResponse_P(200, "image/png", (const uint8_t*)LOGO_PNG, LOGO_PNG_LEN);
      response->setContentLength(LOGO_PNG_LEN);
      response->addHeader("Content-Encoding", "identity");
      response->addHeader("Cache-Control", "no-store");
      request->send(response);
    });

    server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request) {
      request->send(200, "text/html", buildConfigPageHtml());
    });

    server.on("/reset", HTTP_GET, [&](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Factory reset... rebooting");
        Serial.println("Factory reset: clearing config and rebooting...");
        config_clear();     
        delay(150);
        ESP.restart();
    });

    // Save
    server.on("/save", HTTP_POST, [&](AsyncWebServerRequest *request) {
      auto update_str_if_changed = [&](const char* name, char* target, size_t len) {
        if (!request->hasParam(name, true)) return;
        String v = request->getParam(name, true)->value();
        if (v == String(target)) return;
        strlcpy(target, v.c_str(), len);
        Serial.print("Updated param: ");
        Serial.print(name);
        Serial.print(" to ");
        Serial.println(v);
      };

      update_str_if_changed("devname", device_config.device_name, sizeof(device_config.device_name));
      update_str_if_changed("ssid",    device_config.wifi_ssid,   sizeof(device_config.wifi_ssid));
      update_str_if_changed("mqtt",    device_config.mqtt_server, sizeof(device_config.mqtt_server));
      update_str_if_changed("rx",      device_config.mqtt_topic_rx, sizeof(device_config.mqtt_topic_rx));
      update_str_if_changed("tx",      device_config.mqtt_topic_tx, sizeof(device_config.mqtt_topic_tx));

      if (request->hasParam("ch_range", true)) {
          String v = request->getParam("ch_range", true)->value();
          bool newFlag = (v == "high");

          if (newFlag != device_config.extended_channels) {
              device_config.extended_channels = newFlag;
          }
      }

      // Password: only overwrite if user entered something
      if (request->hasParam("pass", true)) {
        String v = request->getParam("pass", true)->value();
        if (v.length() > 0) {
          strlcpy(device_config.wifi_pass, v.c_str(), sizeof(device_config.wifi_pass));
        }
      }

      // Port: update if changed and non-zero
      if (request->hasParam("port", true)) {
        String p = request->getParam("port", true)->value();
        uint16_t newPort = (uint16_t)p.toInt();
        if (newPort != 0 && newPort != device_config.mqtt_port) {
          device_config.mqtt_port = newPort;
        }
      }

      config_save();

      request->send(200, "text/html",
        "<!doctype html><html><head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Rebooting...</title>"
        "<style>"
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;"
        "text-align:center;padding:40px;background:#fff;color:#111;}"
        "</style>"
        "</head><body>"
        "<h2 style='color:#0a84ff;font-size:32px;'>Saved!</h2>"
        "<p>Rebooting device...</p>"
        "<p id='st'>Please wait...</p>"
        "<script>"
        "function ping(){"
        " fetch('/',{cache:'no-store'})"
        "  .then(()=>location.href='/')"
        "  .catch(()=>setTimeout(ping,1000));"
        "}"
        "setTimeout(ping,1500);"
        "</script>"
        "</body></html>"
      );

      // Schedule reboot after response had time to flush
      static esp_timer_handle_t reboot_timer = nullptr;

      if (!reboot_timer) {
        esp_timer_create_args_t args = {};
        args.callback = [](void*) { ESP.restart(); };
        args.arg = nullptr;
        args.dispatch_method = ESP_TIMER_TASK;
        args.name = "reboot_timer";
        esp_timer_create(&args, &reboot_timer);
      }

        // 1500ms gives the browser time to receive/render the HTML reliably
        esp_timer_stop(reboot_timer);
        esp_timer_start_once(reboot_timer, 1500 * 1000);  // microseconds
      });

    // Status JSON
    server.on("/status", HTTP_GET, [&](AsyncWebServerRequest *request) {
      StaticJsonDocument<384> doc;

      doc["mode"] = (WiFi.getMode() == WIFI_AP) ? "ap" : "sta";
      doc["wifi"] = wifiStatusStr();
      doc["ssid"] = (WiFi.getMode() == WIFI_AP) ? String("ESP-Damper-Setup") : String(WiFi.SSID());
      doc["ip"]   = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
      doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;

      doc["mqtt_connected"] = mqttClient.connected();
      doc["mqtt_state"]     = mqttClient.state();
      doc["mqtt_server"]    = String(device_config.mqtt_server);
      doc["mqtt_port"]      = device_config.mqtt_port;

      String out;
      serializeJson(doc, out);
      request->send(200, "application/json", out);
  });

  // Theme: persist on device
  server.on("/theme", HTTP_GET, [&](AsyncWebServerRequest *request) {
    if (request->hasParam("dark")) {
      String v = request->getParam("dark")->value();
      device_config.dark_mode = (v == "1");
      config_save();
    }
    request->redirect("/");
  });
}