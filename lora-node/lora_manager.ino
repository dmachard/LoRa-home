// Structures et constantes pour les messages LoRa
#define HDR_SIZE 9
#define TAG_SIZE 8

enum ReadingType {
  TYPE_AHT20_TEMP = 3,
  TYPE_AHT20_HUM = 4,
  TYPE_BMP280_TEMP = 5,
  TYPE_BMP280_PRES = 6,
  TYPE_BH1750_LUX = 7
};

struct SensorReading {
  uint8_t type;
  int32_t value;
} __attribute__((packed));

struct SensorPayload {
  uint8_t count;
  SensorReading readings[6];
  uint8_t reset_reason;
  uint8_t error_code;
  char name[8];
} __attribute__((packed));

void startLoRaMode() {
  Serial.println("--- DÉMARRAGE MODE LORA NORMAL ---");
  
  if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Éteint la LED
  }
  
  esp_task_wdt_config_t wdt_config = {.timeout_ms = WDT_TIMEOUT_S * 1000,
                                      .idle_core_mask = 0,
                                      .trigger_panic = true};
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);

  // Initialise uniquement les capteurs I2C détectés lors du scan de setup
  if (aht_detected) {
    aht = new Adafruit_AHTX0();
    if (aht->begin()) {
      Serial.println("Capteur AHT20 initialise avec succes !");
    } else {
      Serial.println("Erreur d'initialisation de l'AHT20 !");
      aht_detected = false;
      delete aht;
      aht = nullptr;
    }
  }

  if (bmp_detected) {
    bmp = new Adafruit_BMP280();
    if (bmp->begin(bmp_addr)) {
      Serial.println("Capteur BMP280 initialise avec succes !");
    } else {
      Serial.println("Erreur d'initialisation du BMP280 !");
      bmp_detected = false;
      delete bmp;
      bmp = nullptr;
    }
  }

  if (tsl_detected) {
    tsl = new Adafruit_TSL2561_Unified(tsl_addr, 12345);
    if (tsl->begin()) {
      Serial.println("Capteur TSL2561 initialise avec succes !");
      tsl->enableAutoRange(true);
      tsl->setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
    } else {
      Serial.println("Erreur d'initialisation du TSL2561 !");
      tsl_detected = false;
      delete tsl;
      tsl = nullptr;
    }
  }

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, 10);
  
  radio = new SX1278(new Module(10, 1, 0, -1));
  
  Serial.println("Initialisation du module SX1278...");
  int state = radio->begin(config.lora_freq, config.lora_bw, config.lora_sf, 
                          config.lora_cr, config.lora_sync, config.lora_power, 
                          config.lora_preamble);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SX1278 initialise avec succes !");
  } else {
    Serial.printf("Echec de l'initialisation du SX1278. Code erreur: %d\n", state);
    while (true) {
      esp_task_wdt_reset();
      delay(1000);
    }
  }
}

void loopLoRa() {
  esp_task_wdt_reset();

  SensorPayload payload;
  memset(&payload, 0, sizeof(payload));
  payload.count = 0;

  // 1. Lecture AHT20
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
      Serial.println("Echec de lecture AHT20");
    }
  }

  // 2. Lecture BMP280
  if (bmp_detected && bmp != nullptr) {
    float t = bmp->readTemperature();
    float p = bmp->readPressure();
    Serial.printf("BMP280: T=%.2f°C | P=%.1fhPa\n", t, p / 100.0f);
    
    payload.readings[payload.count].type = TYPE_BMP280_PRES;
    payload.readings[payload.count].value = (int32_t)(p / 10.0f); // Pression en dixièmes de hPa (ou Pa / 10)
    payload.count++;
  }

  // 3. Lecture TSL2561
  if (tsl_detected && tsl != nullptr) {
    sensors_event_t event;
    tsl->getEvent(&event);
    if (event.light) {
      float lux = event.light;
      Serial.printf("TSL2561: L=%.1flux\n", lux);
      
      payload.readings[payload.count].type = TYPE_BH1750_LUX;
      payload.readings[payload.count].value = (int32_t)lux;
      payload.count++;
    } else {
      Serial.println("Echec de lecture TSL2561");
    }
  }

  payload.reset_reason = last_reset_reason;
  payload.error_code = current_error_code;
  memset(payload.name, 0, sizeof(payload.name));
  strncpy(payload.name, config.node_name, sizeof(payload.name) - 1);

  uint8_t frame[64];
  uint8_t iv[12] = {0};
  
  frame[0] = config.node_id;
  frame[1] = (seq >> 24) & 0xFF;
  frame[2] = (seq >> 16) & 0xFF;
  frame[3] = (seq >> 8) & 0xFF;
  frame[4] = (seq) & 0xFF;
  frame[5] = (node_random_id >> 24) & 0xFF;
  frame[6] = (node_random_id >> 16) & 0xFF;
  frame[7] = (node_random_id >> 8) & 0xFF;
  frame[8] = (node_random_id) & 0xFF;

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
    Serial.printf("Transmission OK seq=%lu\n", seq++);
    if (current_error_code == ERR_TX_FAILED)
      current_error_code = ERR_NONE;
  } else {
    Serial.printf("Transmission failed: %d\n", state);
    current_error_code = ERR_TX_FAILED;
  }

  // Boucle d'attente intelligente qui réagit au bouton BOOT
  uint32_t iterations = config.tx_interval * 10;
  if (iterations == 0) iterations = 600; // Sécurité par défaut
  
  for (uint32_t i = 0; i < iterations; i++) {
    esp_task_wdt_reset();
    
    // Vérifie le bouton BOOT (actif LOW)
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(50); // Anti-rebond
      if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("Bouton BOOT pressé ! Enregistrement flag force_config et redémarrage pour configuration BLE...");
        
        // Enregistre l'intention d'entrer en mode config après le reboot
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
