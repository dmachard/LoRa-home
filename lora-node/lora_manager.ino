// LoRa transmission structures and constants
#define HDR_SIZE 9
#define TAG_SIZE 8

#include "shared_protocol.h"

void startLoRaMode() {
  Serial.println("--- STARTING NORMAL LORA MODE ---");

  if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Turn LED off
  }

  esp_task_wdt_config_t wdt_config = {.timeout_ms = WDT_TIMEOUT_S * 1000,
                                      .idle_core_mask = 0,
                                      .trigger_panic = true};
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);

  // Initialize only the I2C sensors detected during setup scan
  if (aht_detected) {
    aht = new Adafruit_AHTX0();
    if (aht->begin()) {
      Serial.println("AHT20 sensor initialized successfully!");
    } else {
      Serial.println("AHT20 initialization failed!");
      aht_detected = false;
      delete aht;
      aht = nullptr;
    }
  }

  if (bmp_detected) {
    bmp = new Adafruit_BMP280();
    if (bmp->begin(bmp_addr)) {
      Serial.println("BMP280 sensor initialized successfully!");
    } else {
      Serial.println("BMP280 initialization failed!");
      bmp_detected = false;
      delete bmp;
      bmp = nullptr;
    }
  }

  if (tsl_detected) {
    tsl = new Adafruit_TSL2561_Unified(tsl_addr, 12345);
    if (tsl->begin()) {
      Serial.println("TSL2561 sensor initialized successfully!");
      tsl->enableAutoRange(true);
      tsl->setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
    } else {
      Serial.println("TSL2561 initialization failed!");
      tsl_detected = false;
      delete tsl;
      tsl = nullptr;
    }
  }

  if (scd_detected) {
    scd4x = new SensirionI2cScd4x();
    scd4x->begin(Wire, 0x62);
    scd4x->stopPeriodicMeasurement();
    uint16_t error = scd4x->startPeriodicMeasurement();
    if (!error) {
      Serial.println("SCD41 sensor initialized successfully!");
    } else {
      Serial.println("SCD41 initialization failed!");
      scd_detected = false;
      delete scd4x;
      scd4x = nullptr;
    }
  }

  if (ina_detected) {
    ina = new INA226(ina_addr);
    if (ina->begin()) {
      ina->setMaxCurrentShunt(3.6, 0.1); // Calibration for 0.1 Ohm shunt resistor (R100) up to 3.6A
      Serial.println("INA226 sensor initialized successfully!");
    } else {
      Serial.println("INA226 initialization failed!");
      ina_detected = false;
      delete ina;
      ina = nullptr;
    }
  }

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, 10);

  if (config.lora_chip == 2) {
    Serial.println("Initializing SX1262 (CS=10, DIO1=1, RST=0, BUSY=5)...");
    Module* mod = new Module(10, 1, 0, 5);
    SX1262* radio62 = new SX1262(mod);
    esp_task_wdt_reset();
    int state = radio62->begin(config.lora_freq, config.lora_bw, config.lora_sf,
                               config.lora_cr, config.lora_sync, config.lora_power,
                               config.lora_preamble);
    esp_task_wdt_reset();
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("SX1262 initialized successfully!");
      radio = radio62;
    } else {
      Serial.printf("SX1262 initialization failed! Error code: %d\n", state);
      Serial.println("Entering BLE Config mode due to radio initialization error...");
      prefs.begin("lora_cfg", false);
      prefs.putBool("force_config", true);
      prefs.end();
      delay(1000);
      ESP.restart();
    }
  } else {
    Serial.println("Initializing SX1278 (CS=10, DIO0=1, RST=0)...");
    Module* mod = new Module(10, 1, 0, -1);
    SX1278* radio78 = new SX1278(mod);
    esp_task_wdt_reset();
    int state = radio78->begin(config.lora_freq, config.lora_bw, config.lora_sf,
                               config.lora_cr, config.lora_sync, config.lora_power,
                               config.lora_preamble);
    esp_task_wdt_reset();
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("SX1278 initialized successfully!");
      radio = radio78;
    } else {
      Serial.printf("SX1278 initialization failed! Error code: %d\n", state);
      Serial.println("Entering BLE Config mode due to radio initialization error...");
      prefs.begin("lora_cfg", false);
      prefs.putBool("force_config", true);
      prefs.end();
      delay(1000);
      ESP.restart();
    }
  }
}

void loopLoRa() {
  esp_task_wdt_reset();

  SensorPayload payload;
  memset(&payload, 0, sizeof(payload));
  payload.count = 0;

  // 1. Read AHT20
  if (aht_detected && aht != nullptr) {
    sensors_event_t humidity_event, temp_event;
    if (aht->getEvent(&humidity_event, &temp_event)) {
      float t = temp_event.temperature;
      float h = humidity_event.relative_humidity;
      Serial.printf("AHT20: T=%.2f°C | H=%.2f%%\n", t, h);

      payload.readings[payload.count].type = TYPE_AHT20_TEMP;
      payload.readings[payload.count].value = (int32_t)(t * 100);
      payload.count++;

      payload.readings[payload.count].type = TYPE_AHT20_HUM;
      payload.readings[payload.count].value = (int32_t)(h * 100);
      payload.count++;
    } else {
      Serial.println("AHT20 read failed");
    }
  }

  // 2. Read BMP280
  if (bmp_detected && bmp != nullptr) {
    float t = bmp->readTemperature();
    float p = bmp->readPressure();
    Serial.printf("BMP280: T=%.2f°C | P=%.1fhPa\n", t, p / 100.0f);

    payload.readings[payload.count].type = TYPE_BMP280_PRES;
    payload.readings[payload.count].value = (int32_t)(p / 10.0f); // Pressure in tenths of hPa (Pa / 10)
    payload.count++;
  }

  // 3. Read TSL2561
  if (tsl_detected && tsl != nullptr) {
    sensors_event_t event;
    if (tsl->getEvent(&event)) {
      float lux = event.light;
      Serial.printf("TSL2561: L=%.1flux\n", lux);

      payload.readings[payload.count].type = TYPE_TSL2561_LUX;
      payload.readings[payload.count].value = (int32_t)lux;
      payload.count++;
    } else {
      Serial.println("TSL2561 read failed");
    }
  }

  // 4. Read SCD41
  if (scd_detected && scd4x != nullptr) {
    bool isDataReady = false;
    scd4x->getDataReadyStatus(isDataReady);
    if (isDataReady) {
      uint16_t co2 = 0;
      float temp = 0.0f;
      float hum = 0.0f;
      uint16_t error = scd4x->readMeasurement(co2, temp, hum);
      if (!error && co2 > 0) {
        Serial.printf("SCD41: CO2=%dppm | T=%.2f°C | H=%.2f%%\n", co2, temp, hum);
        if (payload.count < 10) {
          payload.readings[payload.count].type = TYPE_SCD40_CO2;
          payload.readings[payload.count].value = (int32_t)co2;
          payload.count++;
        }
      } else {
        Serial.println("SCD41 read failed");
      }
    }
  }

  // 5. Read INA226 (Battery Voltage)
  if (ina_detected && ina != nullptr) {
    float v = ina->getBusVoltage(); // Voltage in Volts
    Serial.printf("INA226: Voltage=%.3fV\n", v);

    if (payload.count < 10) {
      payload.readings[payload.count].type = TYPE_INA226_VOLT;
      payload.readings[payload.count].value = (int32_t)(v * 1000.0f); // stored in mV (scale = 0.001 -> V)
      payload.count++;
    }
  }

  payload.reset_reason = last_reset_reason;
  payload.error_code = current_error_code;
  payload.tx_interval = config.tx_interval;
  memset(payload.name, 0, sizeof(payload.name));
  strncpy(payload.name, config.node_name, sizeof(payload.name) - 1);

  uint8_t frame[128];
  uint8_t iv[12] = {0};

  frame[0] = config.node_id;
  frame[1] = (seq >> 24) & 0xFF;
  frame[2] = (seq >> 16) & 0xFF;
  frame[3] = (seq >> 8)  & 0xFF;
  frame[4] = (seq)       & 0xFF;
  frame[5] = (node_random_id >> 24) & 0xFF;
  frame[6] = (node_random_id >> 16) & 0xFF;
  frame[7] = (node_random_id >> 8)  & 0xFF;
  frame[8] = (node_random_id)       & 0xFF;

  memcpy(iv, frame, 9);

  gcm.clear();
  gcm.setKey(config.aes_key, 16);
  gcm.setIV(iv, 12);
  gcm.addAuthData(frame, HDR_SIZE);
  gcm.encrypt(frame + HDR_SIZE, (uint8_t *)&payload, sizeof(payload));
  gcm.computeTag(frame + HDR_SIZE + sizeof(payload), TAG_SIZE);

  uint8_t len = HDR_SIZE + sizeof(payload) + TAG_SIZE;

  int state = radio->transmit(frame, len);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.printf("TX OK seq=%lu\n", seq++);
    if (current_error_code == ERR_TX_FAILED)
      current_error_code = ERR_NONE;
  } else {
    Serial.printf("TX failed: %d\n", state);
    current_error_code = ERR_TX_FAILED;
  }

  // Smart wait loop that reacts to the BOOT button
  uint32_t iterations = config.tx_interval * 10;
  if (iterations == 0) iterations = 600; // Safety fallback

  for (uint32_t i = 0; i < iterations; i++) {
    esp_task_wdt_reset();

    // Check BOOT button (active LOW)
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(50); // Debounce
      if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("BOOT button pressed! Saving force_config flag and rebooting for BLE configuration...");

        // Save the intent to enter config mode after reboot
        prefs.begin("lora_cfg", false);
        prefs.putBool("force_config", true);
        prefs.end();

        delay(500);
        ESP.restart();
      }
    }

    delay(100);
  }
}
