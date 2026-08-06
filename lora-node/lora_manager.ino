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
    bool tslOk = false;
    for (int retry = 0; retry < 3; retry++) {
      if (tsl->begin()) {
        tslOk = true;
        break;
      }
      delay(50);
    }
    if (tslOk) {
      Serial.println("TSL2561 sensor initialized successfully!");
      tsl->enableAutoRange(true);
      tsl->setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
    } else {
      Serial.println("TSL2561 initialization failed!");
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
      int cal_err = ina->setMaxCurrentShunt(0.8, 0.1); // 0.1 Ohm (R100) shunt, max 0.8A (81.92mV max shunt voltage)
      if (cal_err != 0) {
        Serial.printf("INA226 calibration failed with error code: 0x%04X\n", cal_err);
      } else {
        Serial.println("INA226 sensor initialized and calibrated successfully!");
      }
    } else {
      Serial.println("INA226 initialization failed!");
      ina_detected = false;
      delete ina;
      ina = nullptr;
    }
  }

  Serial.printf("Radio Config: Node ID=%d, Name='%s', Freq=%.3f MHz, BW=%.1f kHz, SF=%d, Sync=0x%02X, Power=%d dBm\n",
                config.node_id, config.node_name, config.lora_freq, config.lora_bw, config.lora_sf, config.lora_sync, config.lora_power);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, LORA_CS);

  if (config.lora_chip == 2) {
    Serial.printf("Initializing SX1262 (CS=%d, DIO1=%d, RST=%d, BUSY=%d)...\n", LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
    Module* mod = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
    SX1262* radio62 = new SX1262(mod);
    uint32_t tInitStart = millis();
    int state = radio62->begin(config.lora_freq, config.lora_bw, config.lora_sf,
                               config.lora_cr, config.lora_sync, config.lora_power,
                               config.lora_preamble);
    esp_task_wdt_reset();
    if (state == RADIOLIB_ERR_NONE) {
      radio62->setDio2AsRfSwitch(true);
      radio62->setRegulatorDCDC();
      Serial.printf("SX1262 initialized successfully in %lu ms!\n", millis() - tInitStart);
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
    Serial.printf("Initializing SX1278 (CS=%d, DIO0=%d, RST=%d)...\n", LORA_CS, LORA_DIO0, LORA_RST);
    Module* mod = new Module(LORA_CS, LORA_DIO0, LORA_RST, -1);
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
      addLog("AHT20: T=%.1fC H=%.1f%%", t, h);
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
    addLog("BMP280: P=%.1fhPa", p / 100.0f);
    Serial.printf("BMP280: T=%.2f°C | P=%.1fhPa\n", t, p / 100.0f);

    payload.readings[payload.count].type = TYPE_BMP280_PRES;
    payload.readings[payload.count].value = (int32_t)(p / 10.0f); // Pressure in tenths of hPa (Pa / 10)
    payload.count++;
  }

  // 3. Read TSL2561 (Light Lux photodiode sensor)
  if (tsl_detected && tsl != nullptr) {
    sensors_event_t event;
    bool readSuccess = false;
    for (int retry = 0; retry < 5; retry++) {
      if (tsl->getEvent(&event)) {
        readSuccess = true;
        break;
      }
      delay(50);
    }

    if (readSuccess) {
      float lux = event.light;
      addLog("TSL2561: L=%.0flux", lux);
      Serial.printf("TSL2561: L=%.1flux\n", lux);

      if (payload.count < 10) {
        payload.readings[payload.count].type = TYPE_TSL2561_LUX;
        payload.readings[payload.count].value = (int32_t)lux;
        payload.count++;
      }
    } else {
      Serial.println("TSL2561 read failed");
    }
  }

  // 4. Read SCD41 (Photoacoustic CO2 NDIR sensor requires ~5s measurement window)
  if (scd_detected && scd4x != nullptr) {
    bool isDataReady = false;
    uint32_t startWait = millis();
    while (millis() - startWait < 5000) {
      esp_task_wdt_reset();
      scd4x->getDataReadyStatus(isDataReady);
      if (isDataReady) break;
      delay(100);
    }

    if (isDataReady) {
      uint16_t co2 = 0;
      float temp = 0.0f;
      float hum = 0.0f;
      uint16_t error = scd4x->readMeasurement(co2, temp, hum);
      if (!error && co2 > 0) {
        addLog("SCD41: CO2=%dppm", co2);
        Serial.printf("SCD41: CO2=%dppm | T=%.2f°C | H=%.2f%%\n", co2, temp, hum);
        if (payload.count < 10) {
          payload.readings[payload.count].type = TYPE_SCD40_CO2;
          payload.readings[payload.count].value = (int32_t)co2;
          payload.count++;
        }
      } else {
        Serial.println("SCD41 read failed");
      }
    } else {
      Serial.println("SCD41 measurement not ready after 5s wait");
    }
  }

  // 5. Read INA226 (Voltage, Current, Power)
  if (ina_detected && ina != nullptr) {
    float v = ina->getBusVoltage(); // Voltage in Volts
    float sv = ina->getShuntVoltage_mV(); // Shunt voltage in mV
    float c = sv / 0.1f; // Direct calculation: I (mA) = V_shunt (mV) / R_shunt (0.1 Ohm)
    float p = v * c;     // Direct calculation: P (mW) = V (V) * I (mA)
    addLog("INA226: V=%.2fV I=%.1fmA", v, c);
    Serial.printf("INA226: Voltage=%.3fV | Shunt=%.3fmV | Current=%.1fmA | Power=%.1fmW\n", v, sv, c, p);

    if (payload.count < 10) {
      payload.readings[payload.count].type = TYPE_INA226_VOLT;
      payload.readings[payload.count].value = (int32_t)(v * 1000.0f); // stored in mV (scale = 0.001 -> V)
      payload.count++;
    }
    if (payload.count < 10) {
      payload.readings[payload.count].type = TYPE_INA226_CURR;
      payload.readings[payload.count].value = (int32_t)(c * 10.0f); // stored in 0.1 mA resolution (scale = 0.1 -> mA)
      payload.count++;
    }
    if (payload.count < 10) {
      payload.readings[payload.count].type = TYPE_INA226_POWER;
      payload.readings[payload.count].value = (int32_t)(p * 10.0f); // stored in 0.1 mW resolution (scale = 0.1 -> mW)
      payload.count++;
    }
  }

  uint8_t actual_count = min((int)payload.count, 10);
  payload.count = actual_count;
  payload.reset_reason = last_reset_reason;
  payload.error_code = current_error_code;
  payload.tx_interval = config.tx_interval;
  memset(payload.name, 0, sizeof(payload.name));
  strncpy(payload.name, config.node_name, sizeof(payload.name) - 1);

  uint8_t actual_payload_len = SENSOR_PAYLOAD_HDR_SIZE + (actual_count * sizeof(SensorReading));

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
  gcm.encrypt(frame + HDR_SIZE, (uint8_t *)&payload, actual_payload_len);
  gcm.computeTag(frame + HDR_SIZE + actual_payload_len, TAG_SIZE);

  uint8_t len = HDR_SIZE + actual_payload_len + TAG_SIZE;

  // CAD (Channel Activity Detection) - Skipped for SX1262 without DIO1/BUSY interrupt wiring
  // int cad = radio->scanChannel();

  Serial.println("Starting radio transmit...");
  uint32_t tTxStart = millis();
  int state = radio->transmit(frame, len);
  uint32_t tTxDur = millis() - tTxStart;

  if (state == RADIOLIB_ERR_NONE) {
    addLog("TX OK: seq=%lu", seq);
    Serial.printf("TX OK seq=%lu (took %lu ms)\n", seq++, tTxDur);
    if (current_error_code == ERR_TX_FAILED)
      current_error_code = ERR_NONE;
  } else {
    addLog("TX FAIL: err=%d", state);
    Serial.printf("TX failed: %d\n", state);
    current_error_code = ERR_TX_FAILED;
  }

  // 6. Put LoRa radio transceiver into deep sleep mode
  if (radio != nullptr) {
    radio->sleep();
  }

  uint32_t sleepSec = config.tx_interval;
  if (sleepSec == 0) sleepSec = 60; // Safety fallback

  if (ENABLE_DEEP_SLEEP) {
    Serial.printf("Entering Deep Sleep for %u seconds...\n", sleepSec);
    Serial.flush();

    // Configure timer wakeup (in microseconds)
    esp_sleep_enable_timer_wakeup((uint64_t)sleepSec * 1000000ULL);

    // Turn OFF blue LED (LORA_DIO1) and hold its HIGH state during Deep Sleep
    if (LORA_DIO1 >= 0) {
      pinMode(LORA_DIO1, OUTPUT);
      digitalWrite(LORA_DIO1, HIGH);
      gpio_hold_en((gpio_num_t)LORA_DIO1);
      gpio_deep_sleep_hold_en();
    }

    // Start deep sleep (power consumption drops to ~5-10uA!)
    esp_deep_sleep_start();
  } else {
    Serial.printf("Deep Sleep disabled in config.h. Delaying %u seconds (USB Serial remains connected)...\n", sleepSec);
    for (uint32_t i = 0; i < sleepSec; i++) {
      esp_task_wdt_reset();
      delay(1000);
    }
  }
}
