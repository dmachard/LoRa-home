# ESP32 LoRa Gateway & Node — Development & Compilation Guide

This document describes how to set up the development environment, install all required cores and libraries, and compile or flash the Gateway and Node firmwares in this repository.

---

## 1. Prerequisites: Install `arduino-cli`

To avoid embedding local binaries, install `arduino-cli` globally in your user-level binary directory:
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
```
Make sure `~/.local/bin` is added to your environment `PATH` (typically inside your `~/.bashrc` or `~/.profile`).

---

## 2. Core and Library Installation

Once `arduino-cli` is installed, install the ESP32 platform core and all required libraries by executing:

```bash
# 2.1 Update the index and install the ESP32 core
arduino-cli core update-index
arduino-cli core install esp32:esp32

# 2.2 Install all library dependencies
arduino-cli lib install "RadioLib" "Crypto" "Adafruit SSD1306" "Adafruit GFX Library" "Adafruit AHTX0" "Adafruit BusIO" "Adafruit Unified Sensor" "NimBLE-Arduino" "ArduinoJson" "NimBLE-DataPipe"
```

---

## 3. Compiling the Gateway (`lora-gw`)

The Gateway is based on the **ESP32-C6** and an **SX1262** transceiver. It serves a local dashboard and exports Prometheus metrics.

Navigate to the `lora-gw/` directory:
```bash
cd lora-gw
```

*   **Generate Web Assets & Compile**:
    ```bash
    make
    ```
    *(This runs `generate_assets.py` to convert `html/` files to `PROGMEM` headers, then compiles the sketch).*
*   **Compile Only**:
    ```bash
    make compile
    ```
*   **Upload to Board** (via USB, default port `/dev/ttyACM0`):
    ```bash
    make upload PORT=/dev/ttyACM0
    ```
*   **Start Serial Monitor**:
    ```bash
    make monitor PORT=/dev/ttyACM0
    ```

> [!NOTE]
> The Gateway Makefile automatically appends `:PartitionScheme=min_spiffs` to the FQBN. This minimizes the SPIFFS storage to 128KB, freeing up **1.9MB of APP space** to allow OTA updates to fit comfortably.

---

## 4. Compiling the Node (`lora-node`)

The Node is based on the **ESP32-C3** and an **SX1278** transceiver. It reads sensors and transmits encrypted payloads.

Navigate to the `lora-node/` directory:
```bash
cd lora-node
```

*   **Compile Only**:
    ```bash
    make compile
    ```
*   **Upload to Board** (via USB, default port `/dev/ttyACM0`):
    ```bash
    make upload PORT=/dev/ttyACM0
    ```
*   **Start Serial Monitor**:
    ```bash
    make monitor PORT=/dev/ttyACM0
    ```

> [!NOTE]
> The Node Makefile compiles using `esp32:esp32:esp32c3:CDCOnBoot=cdc` by default to ensure serial output print statements are visible on the native USB-CDC port.
