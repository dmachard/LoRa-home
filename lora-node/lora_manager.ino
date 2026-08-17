// LoRa transmission structures and constants
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
    scd4x->powerDown();
    Serial.println("SCD41 sensor initialized in Power Down mode!");
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
    }
  }
}

bool gcm_decrypt(const uint8_t *frame, uint8_t frame_len, uint8_t *payload, uint8_t payload_size) {
  if (frame_len < HDR_SIZE + TAG_SIZE) return false;
  uint8_t payload_len = frame_len - HDR_SIZE - TAG_SIZE;
  if (payload_len > payload_size) return false;

  uint8_t iv[12] = {0};
  memcpy(iv, frame, 9);

  const uint8_t *ciphertext = frame + HDR_SIZE;
  const uint8_t *tag = frame + frame_len - TAG_SIZE;

  GCM<AES128> local_gcm;
  local_gcm.setKey(config.aes_key, 16);
  local_gcm.setIV(iv, 12);
  local_gcm.addAuthData(frame, HDR_SIZE);
  local_gcm.decrypt(payload, ciphertext, payload_len);

  uint8_t computed_tag[TAG_SIZE];
  local_gcm.computeTag(computed_tag, TAG_SIZE);
  return memcmp(computed_tag, tag, TAG_SIZE) == 0;
}

bool sendDataAndWaitForAck(const uint8_t* frame, uint8_t len, uint32_t current_seq) {
  uint8_t ack_total_len = HDR_SIZE + sizeof(AckPayload) + TAG_SIZE;
  uint32_t ack_toa_ms = (uint32_t)(radio->getTimeOnAir(ack_total_len) / 1000);
  uint32_t ack_timeout_ms = ack_toa_ms + 500;

  tx_total++;
  bool had_retry = false;

  const int MAX_ATTEMPTS = 3;
  for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
    if (attempt > 1) {
      had_retry = true;
    }
    esp_task_wdt_reset();
    Serial.printf("LoRa TX seq=%lu attempt=%d/%d (ACK timeout=%lums)...\n",
                  current_seq, attempt, MAX_ATTEMPTS, ack_timeout_ms);

    int tx_state = radio->transmit(frame, len);
    if (tx_state != RADIOLIB_ERR_NONE) {
      Serial.printf("TX transmit failed: %d\n", tx_state);
      delay(50);
      continue;
    }

    uint8_t rx_buf[64] = {0};
    uint32_t rx_start = millis();
    int rx_state = radio->receive(rx_buf, sizeof(rx_buf), (uint16_t)ack_timeout_ms);
    uint32_t rx_duration = millis() - rx_start;

    if (rx_state == RADIOLIB_ERR_NONE) {
      int rx_len = radio->getPacketLength();
      if (rx_len >= (int)(HDR_SIZE + sizeof(AckPayload) + TAG_SIZE)) {
        uint8_t rx_node_id = rx_buf[0];
        if (rx_node_id == config.node_id) {
          uint8_t ack_payload_buf[64] = {0};
          if (gcm_decrypt(rx_buf, rx_len, ack_payload_buf, sizeof(ack_payload_buf))) {
            AckPayload* ack = (AckPayload*)ack_payload_buf;
            if (ack->node_id == config.node_id && ack->seq == current_seq) {
              Serial.printf("ACK received! seq=%lu status=%d (took %lu ms)\n",
                            current_seq, ack->status, rx_duration);
              tx_success++;
              if (had_retry) {
                tx_retries++;
              }
              return true;
            }
          } else {
            Serial.println("ACK decryption failed (Auth Fail)");
          }
        }
      }
    } else {
      Serial.printf("ACK timeout (attempt %d/%d, err=%d)\n", attempt, MAX_ATTEMPTS, rx_state);
    }

    if (attempt < MAX_ATTEMPTS) {
      delay(random(50, 150));
    }
  }

  tx_failed++;
  if (had_retry) {
    tx_retries++;
  }
  return false;
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
    bmp->setSampling(Adafruit_BMP280::MODE_NORMAL);
    float t = bmp->readTemperature();
    float p = bmp->readPressure();
    addLog("BMP280: P=%.1fhPa", p / 100.0f);
    Serial.printf("BMP280: T=%.2f°C | P=%.1fhPa\n", t, p / 100.0f);

    payload.readings[payload.count].type = TYPE_BMP280_PRES;
    payload.readings[payload.count].value = (int32_t)(p / 10.0f);
    payload.count++;
  }

  // 3. Read TSL2561
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

  // 4. Read SCD41
  if (scd_detected && scd4x != nullptr) {
    scd4x->wakeUp();
    delay(20);
    uint16_t scd_err = scd4x->measureSingleShot();
    if (!scd_err) {
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
    } else {
      Serial.printf("SCD41 measureSingleShot failed: 0x%04X\n", scd_err);
    }
    scd4x->powerDown();
  }

  // 5. Read INA226
  if (ina_detected && ina != nullptr) {
    ina->setModeShuntBusContinuous();
    delay(10);
    float v = ina->getBusVoltage();
    float sv = ina->getShuntVoltage_mV();
    float c = sv / 0.1f;
    float p = v * c;
    addLog("INA226: V=%.2fV I=%.1fmA", v, c);
    Serial.printf("INA226: Voltage=%.3fV | Shunt=%.3fmV | Current=%.1fmA | Power=%.1fmW\n", v, sv, c, p);

    if (payload.count < 10) {
      payload.readings[payload.count].type = TYPE_INA226_VOLT;
      payload.readings[payload.count].value = (int32_t)(v * 1000.0f);
      payload.count++;
    }
    if (payload.count < 10) {
      payload.readings[payload.count].type = TYPE_INA226_CURR;
      payload.readings[payload.count].value = (int32_t)(c * 10.0f);
      payload.count++;
    }

  }

  // 6. Add LoRa statistics
  if (payload.count < 16) {
    payload.readings[payload.count].type = TYPE_LORA_TX_TOTAL;
    payload.readings[payload.count].value = (int32_t)tx_total;
    payload.count++;
  }
  if (payload.count < 16) {
    payload.readings[payload.count].type = TYPE_LORA_TX_SUCCESS;
    payload.readings[payload.count].value = (int32_t)tx_success;
    payload.count++;
  }
  if (payload.count < 16) {
    payload.readings[payload.count].type = TYPE_LORA_TX_FAILED;
    payload.readings[payload.count].value = (int32_t)tx_failed;
    payload.count++;
  }
  if (payload.count < 16) {
    payload.readings[payload.count].type = TYPE_LORA_TX_RETRIES;
    payload.readings[payload.count].value = (int32_t)tx_retries;
    payload.count++;
  }
  if (payload.count < 16) {
    payload.readings[payload.count].type = TYPE_LORA_TX_POWER;
    payload.readings[payload.count].value = (int32_t)config.lora_power;
    payload.count++;
  }

  addLog("LoRa Stats: Tot=%lu OK=%lu Fail=%lu Rtr=%lu Pwr=%ddBm", tx_total, tx_success, tx_failed, tx_retries, config.lora_power);
  Serial.printf("LoRa Stats: Total=%lu | Success=%lu | Failed=%lu | Retries=%lu | Power=%ddBm (Payload count=%d)\n",
                tx_total, tx_success, tx_failed, tx_retries, config.lora_power, payload.count);

  uint8_t actual_count = min((int)payload.count, 16);
  payload.count = actual_count;
  payload.reset_reason = last_reset_reason;
  payload.error_code = current_error_code;
  payload.tx_interval = config.tx_interval;
  memset(payload.name, 0, sizeof(payload.name));
  strncpy(payload.name, config.node_name, sizeof(payload.name) - 1);

  uint8_t actual_payload_len = SENSOR_PAYLOAD_HDR_SIZE + (actual_count * sizeof(SensorReading));

  uint8_t frame[128];
  frame[0] = config.node_id;
  frame[1] = (seq >> 24) & 0xFF;
  frame[2] = (seq >> 16) & 0xFF;
  frame[3] = (seq >> 8)  & 0xFF;
  frame[4] = (seq)       & 0xFF;
  frame[5] = MSG_TYPE_DATA;
  frame[6] = (node_random_id >> 16) & 0xFF;
  frame[7] = (node_random_id >> 8)  & 0xFF;
  frame[8] = (node_random_id)       & 0xFF;

  uint8_t iv[12] = {0};
  memcpy(iv, frame, 9);

  GCM<AES128> local_gcm;
  local_gcm.setKey(config.aes_key, 16);
  local_gcm.setIV(iv, 12);
  local_gcm.addAuthData(frame, 9);
  local_gcm.encrypt(frame + 9, (uint8_t *)&payload, actual_payload_len);
  local_gcm.computeTag(frame + 9 + actual_payload_len, TAG_SIZE);

  uint8_t len = 9 + actual_payload_len + TAG_SIZE;

  Serial.println("Starting reliable radio transmit...");
  bool success = sendDataAndWaitForAck(frame, len, seq);

  if (success) {
    addLog("TX OK: seq=%lu", seq);
    Serial.printf("TX OK seq=%lu ACK received!\n", seq++);
  } else {
    addLog("TX FAIL: seq=%lu", seq);
    Serial.printf("TX failed (no ACK received for seq=%lu)\n", seq);
    seq++;
  }

  if (bmp_detected && bmp != nullptr) {
    bmp->setSampling(Adafruit_BMP280::MODE_SLEEP);
  }
  if (ina_detected && ina != nullptr) {
    ina->shutDown();
  }

  if (radio != nullptr) {
    radio->sleep();
  }

  uint32_t sleepSec = config.tx_interval;
  if (sleepSec == 0) sleepSec = 60;

  if (ENABLE_DEEP_SLEEP) {
    Serial.printf("Entering Deep Sleep for %u seconds...\n", sleepSec);
    if (Serial) {
      Serial.flush();
    }

    esp_sleep_enable_timer_wakeup((uint64_t)sleepSec * 1000000ULL);

    if (LORA_CS >= 0) {
      pinMode(LORA_CS, OUTPUT);
      digitalWrite(LORA_CS, HIGH);
      gpio_hold_en((gpio_num_t)LORA_CS);
    }

    if (LED_PIN >= 0) {
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(LED_PIN, HIGH);
      gpio_hold_en((gpio_num_t)LED_PIN);
    }

    gpio_deep_sleep_hold_en();
    esp_deep_sleep_start();
  } else {
    Serial.printf("Deep Sleep disabled in config.h. Delaying %u seconds...\n", sleepSec);
    for (uint32_t i = 0; i < sleepSec; i++) {
      esp_task_wdt_reset();
      delay(1000);
    }
  }
}
