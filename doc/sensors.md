# Compatible Sensors & Hardware Guide

The **LoRa Home Gateway & Node** ecosystem features plug-and-play **automatic I2C sensor detection**. At boot, the node scans the I2C bus and dynamically initializes only the sensors connected to the hardware.

---

## 1. Supported Environmental Sensors

| Sensor Module | Physical Measurements | Range / Accuracy | Communication | I2C Address(es) | Reading Type ID(s) |
|---|---|---|---|---|---|
| **AHT20** | Temperature, Relative Humidity | -40°C to +85°C (±0.3°C)<br>0% to 100% RH (±2%) | I2C | `0x38` | `3` (`TYPE_AHT20_TEMP`)<br>`4` (`TYPE_AHT20_HUM`) |
| **BMP280** | Atmospheric Pressure, Temperature | 300 to 1100 hPa (±1 hPa)<br>-40°C to +85°C (±1°C) | I2C | `0x76` or `0x77` | `5` (`TYPE_BMP280_TEMP`)<br>`6` (`TYPE_BMP280_PRES`) |
| **TSL2561** | Ambient Light Intensity | 0.1 to 40,000+ Lux | I2C | `0x39`, `0x29`, `0x49` | `7` (`TYPE_TSL2561_LUX`) |
| **SCD41 / SCD40** | Photoacoustic CO₂ Concentration, Temperature, Humidity | 400 to 5,000+ ppm (±40 ppm + 5%) | I2C | `0x62` | `9` (`TYPE_SCD40_CO2`) |
| **INA226** | Bus / Battery Voltage | 0–36V DC | I2C | `0x40`, `0x41`, `0x44`, `0x45` | `10` (`TYPE_INA226_VOLT`) |

---

## 2. Sensor Auto-Detection & Telemetry Packing

On every power-on or soft reset:
1. The **ESP32-C3 Node** runs an **I2C Scanner** routine over `I2C_SDA` (GPIO 3) and `I2C_SCL` (GPIO 4).
2. For each detected sensor, the corresponding driver instance is allocated and initialized.
3. During telemetry measurement loops, valid readings are packed into the 43-byte binary `SensorPayload` array (up to 6 distinct readings per packet).
4. Physical values are scaled into fixed-point signed integers (`int32_t`) before encryption to save radio bandwidth:
   - **Temperatures:** Multiplied by `100` ($21.45 \text{ °C} \rightarrow 2145$)
   - **Humidity:** Multiplied by `100` ($48.20 \text{ \%} \rightarrow 4820$)
   - **Pressure:** Multiplied by `10` ($1013.25 \text{ hPa} \rightarrow 10132$)
   - **Lux & CO₂:** Raw integer values ($494 \text{ ppm} \rightarrow 494$)

---

## 3. Supported LoRa Transceivers

Both Gateway and Client Nodes support dynamic, runtime radio chip switching without recompiling the firmware.

| Transceiver Chip | Supported Frequencies | Features | Wiring / Bus | Selection ID in UI/NVM |
|---|---|---|---|---|
| **Semtech SX1262** | 150 MHz – 960 MHz (433/868/915 MHz) | High efficiency, +22 dBm TX, BUSY pin line | SPI + CS, RST, BUSY, DIO1 | `2` (`SX1262`) |
| **Semtech SX1278 / SX1276** | 137 MHz – 525 MHz (433 MHz) | Legacy classic, +20 dBm TX | SPI + CS, RST, DIO0 | `1` (`SX1278`) |

---

## 4. How to Add a New Sensor Type

To extend the system with a new sensor (e.g. Soil Moisture or PM2.5 Dust sensor):
1. **Define the Reading Type ID** in [`shared_protocol.h`](file:///home/salon/Lab/esp32-lora-gateway-nodes/shared_protocol.h):
   ```cpp
   enum ReadingType {
     ...
     TYPE_SOIL_MOISTURE = 10
   };
   ```
2. **Add Metadata Mapping** in `getReadingDefinition()` in `shared_protocol.h`:
   ```cpp
   case TYPE_SOIL_MOISTURE: return {10, "soil_moisture_percent", "Moisture", "%", 1.0f};
   ```
3. **Read Sensor in Node** (`lora-node/lora_manager.ino`) and pack into `payload.readings`:
   ```cpp
   payload.readings[payload.count].type = TYPE_SOIL_MOISTURE;
   payload.readings[payload.count].value = (int32_t)moisture_value;
   payload.count++;
   ```
4. The **Gateway** and Prometheus `/metrics` exporter will automatically format and display the new metric without needing gateway-side logic changes!
