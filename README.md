# ESP32 LoRa Gateway & Configurable Sensor Nodes System

An end-to-end telemetry system featuring a modular **ESP32-C6 Web Gateway** and configurable **ESP32-C3 Sensor Client Nodes** transmitting encrypted data using **LoRa (433 MHz)** and **AES-128 GCM** security. 

The gateway exports **Prometheus metrics** (`/metrics`) for Grafana visualization and hosts a local configuration and diagnostic dashboard. Both devices store credentials and configuration parameters dynamically inside their **Non-Volatile Memory (NVM)** and support **Web Bluetooth (BLE) provisioning** directly from the local dashboard.

![LoRa Gateway Dashboard](imgs/webinterface.png)

---

## Workspace Structure

*   [**`lora-gw/`**](./lora-gw/): ESP32-C6 central gateway receiver, web server, and dashboard.
*   [**`lora-node/`**](./lora-node/): ESP32-C3 telemetry client node reading environmental sensors (AHT20, BMP280, TSL2561).

---

## System Architecture

```mermaid
graph TD
    subgraph "Sensor Node (ESP32-C3)"
        A1["Sensors: AHT20 / BMP280 / TSL2561"] --> A2["Encrypt AES-128 GCM"]
        A2 --> A3["Transmit LoRa Packet"]
    end
    
    subgraph "Gateway (ESP32-C6)"
        B1["Receive LoRa Packet"] --> B2["Decrypt AES-128 GCM"]
        B2 --> B3["Expose Prometheus Metrics (/metrics)"]
        B2 --> B4["Serve Web Dashboard & BLE Config Portal"]
    end

    A3 -->|433 MHz LoRa| B1
    C["User Browser / Local Dashboard"] -->|Web Bluetooth BLE| D["Node / Gateway Config Modes"]
```

### 1. Gateway (`lora-gw`)
Acts as the central receiver, decryptor, and metrics exporter.
*   **MCU:** ESP32-C6
*   **LoRa Transceiver:** SX1262
*   **OLED Display:** SSD1306 (128x64) displaying system status and BLE config options.
*   **NVM Configs:** Persistent storage for Wi-Fi SSID/password, IP address (static IP support), and AES keys.
*   **Provisioning:** Forced BLE config mode if NVM is uninitialized or when the `BOOT` button (GPIO 9) is held on startup.

### 2. Client Node (`lora-node`)
Telemetry node reading environmental sensors and sending GCM-encrypted payloads.
*   **MCU:** ESP32-C3
*   **LoRa Transceiver:** SX1278
*   **Sensors Support:** AHT20 (temp/humidity), BMP280 (temp/pressure), TSL2561 (light).
*   **NVM Configs:** Persistent storage for Node ID, name, tx interval, LoRa frequency, bandwidth, spreading factor, coding rate, preamble, sync word, and AES key.
*   **Provisioning:** Enters BLE config mode for 60 seconds on boot (or via `BOOT` button), then transitions to normal telemetry transmission.

---

## Security & Packet Format (AES-128 GCM)

*   **Initialization Vector (IV):** Derived dynamically using the frame's unencrypted header (Node ID, Sequence Number, and Random Session ID) padded with 3 bytes of `0x00` to form a 12-byte IV.
*   **Authentication Tag:** `8 bytes` appended to the end of the payload.

### Over-The-Air Frame Structure

| Byte Index | Field | Description |
| :--- | :--- | :--- |
| `0` | **Node ID** | `uint8_t` Identifier of the transmitting node |
| `1 - 4` | **Sequence Number** | `uint32_t` Big-endian packet counter |
| `5 - 8` | **Random Session ID** | `uint32_t` Big-endian node session randomizer (resets on boot) |
| `9 - ...` | **Encrypted Payload** | Ciphertext of the payload |
| `End - 7` to `End` | **GCM Tag** | `8 bytes` auth tag |

---

## Prerequisites & Installation

### 1. Install `arduino-cli` Globally
To compile and flash the boards, install `arduino-cli` in your user-level binary directory (`~/.local/bin`):
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
```
Ensure that `~/.local/bin` is in your system `PATH`.

### 2. Install Cores & Required Libraries
Run the following commands to install the ESP32 platform core and all library dependencies for both the gateway and the nodes:
```bash
# Update index and install ESP32 platform
arduino-cli core update-index
arduino-cli core install esp32:esp32

# Install all required libraries
arduino-cli lib install "RadioLib" "Crypto" "Adafruit SSD1306" "Adafruit GFX Library" "Adafruit AHTX0" "Adafruit BusIO" "Adafruit Unified Sensor" "NimBLE-Arduino" "ArduinoJson" "NimBLE-DataPipe"
```

---

## Quick Start (Build & Flash)

### Flash the Gateway
```bash
cd lora-gw
make         # Generates web headers and compiles
make upload  # Flash the firmware via USB (Default port /dev/ttyACM0)
```

### Flash the Node
```bash
cd lora-node
make         # Compile node sketch
make upload  # Flash the node via USB (Default port /dev/ttyACM0)
```
