// --- CHARGEMENT / SAUVEGARDE CONFIG NVM ---
bool loadConfig() {
  prefs.begin("lora_cfg", false);
  bool isConfigured = prefs.getBool("configured", false);
  
  if (!isConfigured) {
    Serial.println("NVM non configures. Chargement des valeurs par defaut...");
    config.node_id = 3;
    strncpy(config.node_name, "NODE", sizeof(config.node_name) - 1);
    config.node_name[sizeof(config.node_name) - 1] = '\0';
    config.lora_freq = 433.0f;
    config.lora_bw = 125.0f;
    config.lora_sf = 9;
    config.lora_cr = 5;
    config.lora_sync = 0x12;
    config.lora_power = 10;
    config.lora_preamble = 8;
    memset(config.aes_key, 0, 16);
    config.tx_interval = 60;
  } else {
    Serial.println("Configuration NVM chargee !");
    config.node_id = prefs.getUChar("node_id", 3);
    prefs.getString("node_name", "NODE").toCharArray(config.node_name, sizeof(config.node_name));
    config.lora_freq = prefs.getFloat("lora_freq", 433.0f);
    config.lora_bw = prefs.getFloat("lora_bw", 125.0f);
    config.lora_sf = prefs.getUChar("lora_sf", 9);
    config.lora_cr = prefs.getUChar("lora_cr", 5);
    config.lora_sync = prefs.getUChar("lora_sync", 0x12);
    config.lora_power = prefs.getChar("lora_power", 10);
    config.lora_preamble = prefs.getUShort("lora_preamble", 8);
    prefs.getBytes("aes_key", config.aes_key, 16);
    config.tx_interval = prefs.getUShort("tx_interval", 60);
  }
  
  prefs.end();
  
  Serial.printf("Config: ID=%d, Nom=%s, Freq=%.2f, SF=%d, BW=%.1f\n", 
                config.node_id, config.node_name, config.lora_freq, config.lora_sf, config.lora_bw);
                
  return isConfigured;
}

void saveConfig(const JsonDocument &doc) {
  prefs.begin("lora_cfg", false);
  
  if (doc.containsKey("node_id")) prefs.putUChar("node_id", doc["node_id"].as<uint8_t>());
  if (doc.containsKey("node_name")) prefs.putString("node_name", doc["node_name"].as<const char*>());
  if (doc.containsKey("lora_freq")) prefs.putFloat("lora_freq", doc["lora_freq"].as<float>());
  if (doc.containsKey("lora_bw")) prefs.putFloat("lora_bw", doc["lora_bw"].as<float>());
  if (doc.containsKey("lora_sf")) prefs.putUChar("lora_sf", doc["lora_sf"].as<uint8_t>());
  if (doc.containsKey("lora_cr")) prefs.putUChar("lora_cr", doc["lora_cr"].as<uint8_t>());
  if (doc.containsKey("lora_sync")) prefs.putUChar("lora_sync", doc["lora_sync"].as<uint8_t>());
  if (doc.containsKey("lora_power")) prefs.putChar("lora_power", doc["lora_power"].as<int8_t>());
  if (doc.containsKey("lora_preamble")) prefs.putUShort("lora_preamble", doc["lora_preamble"].as<uint16_t>());
  if (doc.containsKey("tx_interval")) prefs.putUShort("tx_interval", doc["tx_interval"].as<uint16_t>());
  
  if (doc.containsKey("aes_key")) {
    String hexKey = doc["aes_key"].as<String>();
    uint8_t keyBytes[16] = {0};
    for (int i = 0; i < 16; i++) {
      String byteStr = hexKey.substring(i * 2, i * 2 + 2);
      keyBytes[i] = strtol(byteStr.c_str(), NULL, 16);
    }
    prefs.putBytes("aes_key", keyBytes, 16);
    Serial.println("Nouvelle cle AES enregistree.");
  }
  
  prefs.putBool("configured", true);
  prefs.end();
  Serial.println("Configuration enregistree dans la NVM.");
}
