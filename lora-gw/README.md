# ESP32-C6 LoRa Gateway — CLI Usage Guide

This document explains how to install `arduino-cli`, manage package cores/libraries, compile, and upload the firmware for the ESP32-C6 LoRa Gateway (`lora-gw`).

---

## 1. Environment Setup (Prerequisites)

### 1.1 Install `arduino-cli` Cleanly
Instead of using a local binary in the repository, install `arduino-cli` globally into your user bin directory (`~/.local/bin`):
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
```

### 1.2 Install the ESP32 Core
Run these commands from any directory to configure the ESP32 core:
```bash
# Update board index
arduino-cli core update-index

# Install ESP32 board support
arduino-cli core install esp32:esp32
```

### 1.3 Install Required Libraries
```bash
arduino-cli lib install "RadioLib"
arduino-cli lib install "Crypto"
arduino-cli lib install "Adafruit SSD1306"
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "ArduinoJson"
arduino-cli lib install "NimBLE-Arduino"
arduino-cli lib install "NimBLE-DataPipe"
```

---

## 2. Identify the USB Port

The ESP32-C6 uses a native USB-CDC port which typically appears as `/dev/ttyACM0` on Linux.

1. Connect the board via USB.
2. Find the port using:
   ```bash
   arduino-cli board list
   ```
3. **Linux Permissions:** If you get a `Permission denied` error when trying to upload, grant read/write access to the port:
   ```bash
   sudo chmod 666 /dev/ttyACM0
   ```

---

## 3. Compile and Upload

We use a `Makefile` to simplify building, asset generation, and uploading.

### 3.1 Using the Makefile (Recommended)
Navigate to the `lora-gw/` directory:
```bash
# Generate HTML assets and compile the firmware
make

# Compile only
make compile

# Upload the compiled binary to the board via USB
make upload PORT=/dev/ttyACM0

# Start the serial monitor
make monitor PORT=/dev/ttyACM0

# Clean build artifacts
make clean
```

### 3.2 Manual CLI Commands (Reference)
If running manually, compile specifying the `min_spiffs` partition scheme to allow enough application program space:

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32c6:PartitionScheme=min_spiffs --output-dir ./build lora-gw.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32c6:PartitionScheme=min_spiffs --input-dir ./build
```

---

## 4. Wireless Firmware Update (OTA WiFi)

The gateway hosts a built-in web portal for OTA updates.

1. Generate the `.bin` file by compiling the code (`make compile`).
2. Open your web browser and go to: **`http://192.168.1.100/update`** (replace with the IP shown on the OLED if it differs).
3. Click **"Choose File"** and select the binary:
   * Path: `lora-gw/build/lora-gw.ino.bin`
4. Click **"Démarrer la mise à jour"** (Start Update).
5. Once uploaded, the page will confirm success and the gateway will automatically reboot.

---

## 5. Initial Provisioning & Configuration (BLE)

On the first boot (when the configuration is not yet initialized in NVM) or when the physical **BOOT button (GPIO 9)** is held down during power-up/reset, the Gateway enters **BLE Configuration Mode**.

### 5.1 Step-by-Step Configuration:
1. **Power up the Gateway**: The OLED display will show `[ BLE CONFIG MODE ]` and the advertising name (e.g., `ESP32-LoRa-Gateway-XXXX`).
2. **Open the Configuration Portal**:
   * Open the local file `lora-gw/html/index.html` in a Web-Bluetooth-compatible browser (e.g., Google Chrome or Microsoft Edge).
   * *Note: Web Bluetooth requires a secure context. Opening a local file (`file:///`) is allowed and trusted by browsers for this API.*
3. **Connect to the Gateway**:
   * Scroll to the BLE configuration tab and click **"Se connecter à la Passerelle"** (Connect to Gateway).
   * Select your device (`ESP32-LoRa-Gateway-XXXX`) in the browser's pairing popup and click **Pair**.
4. **Modify Settings**:
   * The page will automatically retrieve and load the current parameters.
   * Enter your Wi-Fi SSID and Password.
   * Enable/disable **Static IP** (and specify Local IP, Gateway, and Subnet mask if enabled).
   * Input the 16-byte AES Encryption Key (32 hex characters).
5. **Save & Apply**:
   * Click **"Enregistrer la configuration"** (Save Configuration) to store the settings in NVM.
   * The Gateway will write the settings, confirm success on the OLED, and automatically reboot to connect to the Wi-Fi network.

