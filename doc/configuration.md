# Configuration & Provisioning Guide

Both the **Gateway** and **Sensor Client Nodes** store configuration parameters in **Non-Volatile Memory (NVM)** and support wireless **Web Bluetooth (BLE) provisioning** directly from modern web browsers (Chrome, Edge, Opera).

---

## 1. Gateway Configuration Portal

The Gateway hosts a responsive web dashboard accessible over HTTP and Web Bluetooth.

### Accessing the Web Interface
1. Connect to the Gateway's local IP address in your browser: `http://<gateway-ip>` (e.g. `http://192.168.1.150`).
2. Click **Administration →** in the top navigation bar.
3. Authenticate with the Admin password (default: `admin`).

### Configurable Gateway Settings
- **WiFi Settings:** SSID & Passphrase.
- **Network Mode:** DHCP or Static IP (Local IP, Gateway Router, Subnet Mask).
- **LoRa Hardware Chip:** Select **SX1262** or **SX1278** transceiver.
- **Encryption Key:** 128-bit AES-GCM network key (hexadecimal format).
- **Admin Password:** Security password for saved HTTP POST configurations.

---

## 2. Telemetry Node BLE Provisioning

Nodes advertise a Web Bluetooth service when unconfigured or when triggered manually.

### How to Force BLE Config Mode on a Node
1. **At Power-On / Boot:** Hold or press the **BOOT button (GPIO 9)** during the 1.5-second boot window.
2. **From Running Mode:** Press the **BOOT button (GPIO 9)**. The node automatically saves a `force_config` flag and reboots into BLE mode.
3. **On Radio Error:** If the radio chip fails to initialize (e.g. wrong hardware selection), the node automatically reboots into BLE mode.

### Provisioning via Web Bluetooth
1. Open the Gateway Administration Page (`http://<gateway-ip>/admin`).
2. Scroll to the **Node BLE Configuration** section.
3. Click **Connect to Node**. Select your node from the browser Bluetooth prompt (e.g., `ESP32-LoRa-NODE-03`).
4. Modify any parameters:
   - **Node ID (1–15)** & **Node Name** (e.g., `BUREAU`).
   - **LoRa Chip Hardware:** Select **SX1262** or **SX1278**.
   - **RF Parameters:** Frequency (e.g. `433.0 MHz`), Bandwidth (`125.0 kHz`), Spreading Factor (`SF7`–`SF12`), Coding Rate (`4/5`), Sync Word (`0x12` or `0x34`).
     > ⚠️ **Important:** In single-channel transceivers (ESP32 + SX1262), the **Spreading Factor (SF)**, **Frequency**, **Bandwidth**, and **Sync Word** MUST be identical across all nodes and the Gateway.
   - **Network AES Key:** Must match the Gateway's AES key.
   - **Transmit Interval:** Sleeping / telemetry interval in seconds (e.g., `60s`).
5. Click **Save Configuration**. The node saves settings to NVM and reboots into normal telemetry mode.

---

## 3. Wireless OTA Firmware Updates

Nodes support wireless Over-The-Air (OTA) firmware updates via BLE without disconnecting hardware or plugging in USB cables:

1. Compile the firmware in `lora-node/`: `make compile` (generates `./build/lora-node.ino.bin`).
2. Connect to the node via BLE on `http://<gateway-ip>/admin`.
3. Under **Node OTA Update**, click **Choose File** and select `lora-node.ino.bin`.
4. Click **Start OTA Update**. The progress bar will stream binary chunks directly into the ESP32 OTA partition.
5. Upon completion, the node finalizes the flash, verifies integrity, and reboots automatically with the new firmware.
