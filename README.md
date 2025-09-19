# ESP Damper  
## HVAC Damper Controller

This project is designed to monitor and control HVAC dampers using the **TAC-910 protocol**.  
It utilizes the **ESP32-S2-WROOM** microcontroller, which supports up to 4 RMT channels, allowing control of up to **four dampers**.

---

## 🚀 Key Features

- **Parallel Integration**  
  Hardware connects in parallel with an existing TAC-910 controller.  
  This enables the ESP32 to both **listen to IR commands** received by the TAC-910 and **send its own IR commands**.

- **Wi-Fi & MQTT Communication**  
  The ESP32 connects to the home network via Wi-Fi and communicates with **Home Assistant** using the **MQTT protocol**.

---

## ⚙️ Operation Overview

- **Startup**  
  On boot, the controller connects to Wi-Fi and the MQTT broker, then enters RX mode to listen for IR commands on all four RMT channels.

- **IR → MQTT**  
  When an IR command is received, it is parsed and published to Home Assistant via MQTT.

- **MQTT → IR**  
  When an MQTT command is received, it’s converted into the appropriate IR format.  
  The relevant RMT channel switches to TX mode, sends the command to the TAC-910, and then returns to RX mode.

- **FW update**  
  The FW can be updated using OTA by accessing the controller in your browser using controller IP address.

---

## 📦 Modules

- **RX**  
  Listens for IR commands on up to 4 channels using the RMT module.

- **TX**  
  Sends IR commands to the TAC-910 via the RMT module.

- **Wireless**  
  Handles Wi-Fi and MQTT connectivity, as well as OTA firmware updates.

- **Helpers**  
  Includes functions and defaults for data parsing and command management.

- **EEPROM**  
  Defined but not currently used.

---

## 📝 TODO

- [ ] Verify successful RMT command transmission by detecting a low-level long pulse after sending.  
- [ ] Add a user interface for updating Wi-Fi credentials.  
- [ ] Utilize EEPROM to:  
  - [ ] Store the last known configuration for each damper channel.  
  - [ ] Save Wi-Fi credentials.


## 🛠️ Controller view
<img width="457" height="348" alt="image" src="https://github.com/user-attachments/assets/ba949e85-a7b5-4301-97c7-c4e28e1ba06b" />


