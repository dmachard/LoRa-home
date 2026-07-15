#include <Update.h>
#include "esp_partition.h"
#include "esp_ota_ops.h"

extern uint32_t ota_total_size;
extern uint32_t ota_received_bytes;

// --- SETUP BLE CONFIGURATION ---
void setupBLE(bool isConfigured) {
  // Récupère les 2 derniers octets de l'adresse MAC pour créer un suffixe unique
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char bleName[32];
  
  if (isConfigured && strlen(config.node_name) > 0) {
    snprintf(bleName, sizeof(bleName), "ESP32-LoRa-%s", config.node_name);
  } else {
    snprintf(bleName, sizeof(bleName), "ESP32-LoRa-%02X%02X", mac[4], mac[5]);
  }

  Serial.printf("Démarrage du serveur BLE sous le nom : %s\n", bleName);

  bleDataPipe = new NimBLE_DataPipe(bleName, BLE_SERVICE_UUID, BLE_CHAR_UUID);
  
  bleDataPipe->setOnJson([](const JsonDocument &doc) {
    String cmd = doc["cmd"] | "";
    if (cmd == "get_config") {
      Serial.println("Requete get_config recue ! Envoi des parametres...");
      JsonDocument res;
      res["node_id"] = config.node_id;
      res["node_name"] = String(config.node_name);
      res["lora_freq"] = config.lora_freq;
      res["lora_bw"] = config.lora_bw;
      res["lora_sf"] = config.lora_sf;
      res["lora_cr"] = config.lora_cr;
      res["lora_sync"] = config.lora_sync;
      res["lora_power"] = config.lora_power;
      res["lora_preamble"] = config.lora_preamble;
      
      char hexKey[33] = {0};
      for (int i = 0; i < 16; i++) {
        sprintf(hexKey + (i * 2), "%02x", config.aes_key[i]);
      }
      res["aes_key"] = String(hexKey);
      res["tx_interval"] = config.tx_interval;

      JsonArray active_sensors = res["detected_sensors"].to<JsonArray>();
      if (aht_detected) active_sensors.add("AHT20");
      if (bmp_detected) active_sensors.add("BMP280");
      if (tsl_detected) active_sensors.add("TSL2561");
      
      bleDataPipe->sendJson(res);
    } 
    else if (cmd == "set_config") {
      Serial.println("Requete set_config recue ! Enregistrement...");
      saveConfig(doc);
      JsonDocument res;
      res["status"] = "saved";
      bleDataPipe->sendJson(res);
      shouldReboot = true;
    }
    else if (cmd == "reset_config") {
      Serial.println("Requete reset_config recue ! Remise a zero de la NVM...");
      prefs.begin("lora_cfg", false);
      prefs.clear();
      prefs.end();
      
      JsonDocument res;
      res["status"] = "reseted";
      bleDataPipe->sendJson(res);
      shouldReboot = true;
    }
    else if (cmd == "start_ota") {
      ota_total_size = doc["size"] | 0;
      ota_received_bytes = 0;
      Serial.printf("Requete start_ota recue. Taille = %d octets\n", ota_total_size);
      
      // S'assurer qu'aucune session OTA précédente n'est restée active/suspendue
      if (Update.isRunning()) {
        Serial.println("Une session OTA precedente est en cours. Annulation de l'ancienne session...");
        Update.abort();
      } else {
        Update.abort(); // Appel de sécurité inconditionnel pour réinitialiser la classe Update
      }
      
      // Diagnostics de partitions
      Serial.println("--- Liste des partitions de l'ESP32 ---");
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
        Serial.printf("Prochaine partition OTA cible : %s | Taille : %d\n", update_partition->label, update_partition->size);
      } else {
        Serial.println("Prochaine partition OTA cible : INTROUVABLE !");
      }
      
      if (ota_total_size > 0) {
        if (Update.begin(ota_total_size)) {
          Serial.println("Initialisation OTA reussie !");
          JsonDocument res;
          res["status"] = "ota_started";
          bleDataPipe->sendJson(res);
        } else {
          Serial.printf("Echec de l'initialisation OTA: %s, code: %d\n", Update.errorString(), Update.getError());
          JsonDocument res;
          res["status"] = "ota_error";
          res["error"] = String(Update.errorString()) + " (code " + String(Update.getError()) + ")";
          bleDataPipe->sendJson(res);
        }
      } else {
        JsonDocument res;
        res["status"] = "ota_error";
        res["error"] = "Taille invalide";
        bleDataPipe->sendJson(res);
      }
    }
  });

  bleDataPipe->setOnBinary([](uint8_t type, const uint8_t *data, size_t len) {
    if (type == 0x02) { // TYPE_OTA_CHUNK
      if (!Update.isRunning()) {
        Serial.println("Erreur: Ecriture binaire recue mais l'OTA n'est pas demarre !");
        return;
      }
      
      size_t written = Update.write(const_cast<uint8_t*>(data), len);
      ota_received_bytes += written;
      
      static uint32_t lastPrint = 0;
      if (millis() - lastPrint > 1000 || ota_received_bytes == ota_total_size) {
        Serial.printf("OTA: %d / %d octets (%d%%)\n", 
                      ota_received_bytes, ota_total_size, 
                      (ota_received_bytes * 100) / ota_total_size);
        lastPrint = millis();
      }
      
      if (ota_received_bytes >= ota_total_size) {
        if (Update.end(true)) {
          Serial.println("Mise a jour OTA reussie avec succes ! Redemarrage...");
          delay(1000);
          ESP.restart();
        } else {
          Serial.printf("Echec de la fin d'OTA: %s\n", Update.errorString());
        }
      }
    }
  });

  bleDataPipe->begin();

  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(false, false, true);

  bleStartMs = millis();
  Serial.println("Serveur BLE active. En attente de connexion (60s)...");
}

void loopBLE() {
  // Si un client BLE est connecté à la DataPipe, on prolonge le mode config
  bool isConnected = (bleDataPipe != nullptr && bleDataPipe->isConnected());
  if (isConnected) {
    bleStartMs = millis();
  }

  // Clignotement de la LED pour indiquer le mode config
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

  // Reboot si demandé par l'interface web (après enregistrement)
  if (shouldReboot) {
    if (LED_PIN >= 0) digitalWrite(LED_PIN, LOW); // LED allumée fixe
    Serial.println("Rebooting to apply settings...");
    delay(1500);
    ESP.restart();
  }

  // Quitter le mode config si le délai de 60 secondes est expiré
  if (millis() - bleStartMs > BLE_TIMEOUT_MS) {
    Serial.println("BLE config timeout. Exiting config mode...");
    inConfigMode = false;
    
    if (LED_PIN >= 0) digitalWrite(LED_PIN, HIGH); // Éteint la LED
    
    NimBLEDevice::deinit(true);
    delay(500);
    
    startLoRaMode();
  }
  
  delay(50);
}
