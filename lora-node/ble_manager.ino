#include <Update.h>
#include "esp_partition.h"
#include "esp_ota_ops.h"

extern uint32_t ota_total_size;
extern uint32_t ota_received_bytes;

// --- SETUP BLE CONFIGURATION ---
void setupBLE(bool isConfigured) {
  // Use last 2 bytes of MAC address to create a unique device name suffix
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char bleName[32];

  if (isConfigured && strlen(config.node_name) > 0) {
    snprintf(bleName, sizeof(bleName), "ESP32-LoRa-%s", config.node_name);
  } else {
    snprintf(bleName, sizeof(bleName), "ESP32-LoRa-%02X%02X", mac[4], mac[5]);
  }

  Serial.printf("Starting BLE server as: %s\n", bleName);

  bleDataPipe = new NimBLE_DataPipe(bleName, BLE_SERVICE_UUID, BLE_CHAR_UUID);

  bleDataPipe->setOnJson([](const JsonDocument &doc) {
    String cmd = doc["cmd"] | "";
    if (cmd == "get_config") {
      Serial.println("get_config request received! Sending parameters...");
      JsonDocument res;
      res["node_id"]   = config.node_id;
      res["node_name"] = String(config.node_name);
      res["lora_freq"] = config.lora_freq;
      res["lora_bw"]   = config.lora_bw;
      res["lora_sf"]   = config.lora_sf;
      res["lora_cr"]   = config.lora_cr;
      res["lora_sync"] = config.lora_sync;
      res["lora_power"]    = config.lora_power;
      res["lora_preamble"] = config.lora_preamble;

      char hexKey[33] = {0};
      for (int i = 0; i < 16; i++) {
        sprintf(hexKey + (i * 2), "%02x", config.aes_key[i]);
      }
      res["aes_key"]    = String(hexKey);
      res["tx_interval"] = config.tx_interval;

      JsonArray active_sensors = res["detected_sensors"].to<JsonArray>();
      if (aht_detected) active_sensors.add("AHT20");
      if (bmp_detected) active_sensors.add("BMP280");
      if (tsl_detected) active_sensors.add("TSL2561");
      if (scd_detected) active_sensors.add("SCD41");

      bleDataPipe->sendJson(res);
    }
    else if (cmd == "set_config") {
      Serial.println("set_config request received! Saving...");
      saveConfig(doc);
      JsonDocument res;
      res["status"] = "saved";
      bleDataPipe->sendJson(res);
      shouldReboot = true;
    }
    else if (cmd == "reset_config") {
      Serial.println("reset_config request received! Clearing NVM...");
      prefs.begin("lora_cfg", false);
      prefs.clear();
      prefs.end();

      JsonDocument res;
      res["status"] = "reseted";
      bleDataPipe->sendJson(res);
      shouldReboot = true;
    }
    else if (cmd == "start_ota") {
      ota_total_size     = doc["size"] | 0;
      ota_received_bytes = 0;
      Serial.printf("start_ota request received. Size = %d bytes\n", ota_total_size);

      // Ensure no previous OTA session is still active/suspended
      if (Update.isRunning()) {
        Serial.println("A previous OTA session is still running. Aborting old session...");
      }
      Update.abort(); // Reset the Update class instance

      // Partition diagnostics
      Serial.println("--- ESP32 partition list ---");
      esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
      while (it != NULL) {
        const esp_partition_t *part = esp_partition_get(it);
        Serial.printf("  Label: %s | Type: 0x%02X | SubType: 0x%02X | Size: %d\n",
                       part->label, part->type, part->subtype, part->size);
        it = esp_partition_next(it);
      }
      esp_partition_iterator_release(it);

      const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
      if (update_partition != NULL) {
        Serial.printf("Next OTA target partition: %s | Size: %d\n", update_partition->label, update_partition->size);
      } else {
        Serial.println("Next OTA target partition: NOT FOUND!");
      }

      if (ota_total_size > 0) {
        if (Update.begin(ota_total_size)) {
          Serial.println("OTA initialization successful!");
          JsonDocument res;
          res["status"] = "ota_started";
          bleDataPipe->sendJson(res);
        } else {
          Serial.printf("OTA initialization failed: %s, code: %d\n", Update.errorString(), Update.getError());
          JsonDocument res;
          res["status"] = "ota_error";
          res["error"]  = String(Update.errorString()) + " (code " + String(Update.getError()) + ")";
          bleDataPipe->sendJson(res);
        }
      } else {
        JsonDocument res;
        res["status"] = "ota_error";
        res["error"]  = "Invalid size";
        bleDataPipe->sendJson(res);
      }
    }
  });

  bleDataPipe->setOnBinary([](uint8_t type, const uint8_t *data, size_t len) {
    if (type == 0x02) { // TYPE_OTA_CHUNK
      if (!Update.isRunning()) {
        Serial.println("Error: Binary chunk received but OTA is not started!");
        return;
      }

      size_t written = Update.write(const_cast<uint8_t*>(data), len);
      ota_received_bytes += written;

      static uint32_t lastPrint = 0;
      if (millis() - lastPrint > 1000 || ota_received_bytes == ota_total_size) {
        Serial.printf("OTA: %d / %d bytes (%d%%)\n",
                      ota_received_bytes, ota_total_size,
                      (ota_received_bytes * 100) / ota_total_size);
        lastPrint = millis();
      }

      if (ota_received_bytes >= ota_total_size) {
        if (Update.end(true)) {
          Serial.println("OTA update completed successfully! Rebooting...");
          delay(1000);
          ESP.restart();
        } else {
          Serial.printf("OTA finalization failed: %s\n", Update.errorString());
        }
      }
    }
  });

  bleDataPipe->begin();

  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(false, false, true);

  // Custom BLE advertisement configuration for Linux/Chrome
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->stop(); // Stop DataPipe's default advertising

  // Primary advertisement data (name and flags)
  NimBLEAdvertisementData advData;
  advData.setName(bleName);
  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);

  // Scan response data (service UUID)
  NimBLEAdvertisementData scanResponseData;
  scanResponseData.setCompleteServices(NimBLEUUID(BLE_SERVICE_UUID));

  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponseData(scanResponseData);
  pAdvertising->start();

  bleStartMs = millis();
  Serial.println("BLE server active. Waiting for connection (60s)...");
}

void loopBLE() {
  // If a BLE client is connected to the DataPipe, extend config mode timeout
  bool isConnected = (bleDataPipe != nullptr && bleDataPipe->isConnected());
  if (isConnected) {
    bleStartMs = millis();
  }

  // Blink LED to indicate config mode
  if (LED_PIN >= 0) {
    static uint32_t lastBlink = 0;
    static bool ledState = false;
    uint32_t interval = isConnected ? 100 : 500;
    if (millis() - lastBlink > interval) {
      lastBlink = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    }
  }

  // Reboot if requested after config save
  if (shouldReboot) {
    if (LED_PIN >= 0) digitalWrite(LED_PIN, LOW); // Solid LED on
    Serial.println("Rebooting to apply settings...");
    delay(1500);
    ESP.restart();
  }

  // Exit config mode if 60-second timeout has elapsed
  if (millis() - bleStartMs > BLE_TIMEOUT_MS) {
    Serial.println("BLE config timeout. Exiting config mode...");
    inConfigMode = false;

    if (LED_PIN >= 0) digitalWrite(LED_PIN, HIGH); // Turn LED off

    NimBLEDevice::deinit(true);
    delay(500);

    startLoRaMode();
  }

  delay(50);
}
