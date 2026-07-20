# System Architecture

This document presents the detailed system architecture of the **LoRa Home Gateway and Client Nodes** ecosystem.

---

## Ecosystem Overview

The system operates as a single-channel, encrypted local LoRa telemetry network, bridging low-power sensor nodes to IP networks and monitoring platforms.

```
+--------------------------+               +----------------------------+               +----------------------+
|  ESP32-C3 Node           |  Encrypted    |   ESP32-C6 Gateway         |  HTTP/JSON    | Prometheus / Grafana |
| (SX1262/SX1278 + Sensors)| ------------> | (SX1262/SX1278 + Web Srv) | ------------> |   /metrics Endpoint  |
+--------------------------+  LoRa 433MHz  +----------------------------+               +----------------------+
             ^                                           ^
             | BLE Provisioning                          | BLE / Web Config
             +--------------------+   +------------------+
                                  |   |
                            +---------------+
                            | Web Browser   |
                            | (Dashboard)   |
                            +---------------+
```

---

## 1. Gateway Component (`lora-gw`)

The Gateway acts as the central receiver, decryptor, web host, and telemetry exporter.

* **Microcontroller (MCU):** ESP32-C6
* **Multi-Radio Hardware Support:** Dynamic runtime support for **SX1262** or **SX1278** transceivers (selectable via Web/BLE UI).
* **OLED Display:** SSD1306 (128x64 I2C) displaying real-time reception status, WiFi state, and BLE provisioning indicator.
* **Persistent Storage (NVM):** Non-Volatile Memory (Preferences) stores WiFi credentials, Admin password, static IP settings, selected radio chip, and shared AES key.
* **Web & Metrics Server:**
  * Serves embedded minified Web UI (`index.html`, `admin.html`, `update.html`).
  * Exposes JSON API (`/api/nodes`, `/api/gw_config`).
  * Exposes `/metrics` endpoint formatted for Prometheus scraping.
* **Provisioning & Management:**
  * Forced BLE configuration mode on uninitialized NVM or via BOOT button holding on boot.
  * Web-based HTTP administration & OTA firmware update portal.

---

## 2. Telemetry Client Node Component (`lora-node`)

The client node reads environmental sensors, encrypts the payload, and transmits data periodically over LoRa.

* **Microcontroller (MCU):** ESP32-C3 (e.g. SuperMini or Seeed Studio XIAO ESP32-C3)
* **Multi-Radio Hardware Support:** Dynamic runtime switching between **SX1262** and **SX1278** transceivers stored in NVM.
* **Auto-Detected Sensor Compatibility:**
  * **AHT20:** Temperature (°C) & Relative Humidity (%)
  * **BMP280:** Temperature (°C) & Barometric Pressure (hPa)
  * **TSL2561 / BH1750:** Ambient Light Intensity (Lux)
  * **SCD41 / SCD40:** Photoacoustic CO₂ Concentration (ppm), Temperature (°C) & Humidity (%)
* **Persistent Storage (NVM):** Stores Node ID, node name, transmission interval, LoRa RF configuration (frequency, bandwidth, spreading factor, coding rate, preamble, sync word, chip selection), and AES-128 key.
* **Operating Modes:**
  * **Provisioning Mode (BLE):** Starts BLE server for 60 seconds on boot (or triggered via BOOT button / radio error fallback) to allow wireless provisioning of parameters and wireless OTA firmware updates.
  * **Normal Telemetry Mode:** Enters low-power execution loop, reads sensors, constructs packed `SensorPayload`, encrypts frame via AES-128-GCM, transmits packet, and waits for next cycle.

---

## 3. Communication & Data Pipelines

1. **Over-The-Air RF (LoRa):**
   * Binary packed frame structure (Header + Encrypted Payload + AES-GCM Auth Tag).
   * Replay-protected sequence counting and dynamic hardware TRNG session randomization.
2. **Local Management (BLE DataPipe):**
   * Binary & JSON wrapper protocol over Web Bluetooth (NimBLE) for secure configuration without internet connectivity.
