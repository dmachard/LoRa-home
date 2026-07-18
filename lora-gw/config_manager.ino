#include <Preferences.h>
#include <ArduinoJson.h>

extern Preferences prefs;
extern String wifi_ssid;
extern String wifi_pass;
extern String admin_pass;
extern bool use_static_ip;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern uint8_t AES_KEY[16];

void loadConfig() {
  prefs.begin("gw_cfg", false);
  bool configured = prefs.getBool("configured", false);
  
  if (configured) {
    wifi_ssid = prefs.getString("wifi_ssid", "");
    wifi_pass = prefs.getString("wifi_pass", "");
    admin_pass = prefs.getString("admin_pass", "admin");
    use_static_ip = prefs.getBool("use_static", false);
    local_IP = IPAddress(prefs.getUInt("local_ip", 0));
    gateway = IPAddress(prefs.getUInt("gateway_ip", 0));
    subnet = IPAddress(prefs.getUInt("subnet_mask", 0));
    prefs.getBytes("aes_key", AES_KEY, 16);
    Serial.println("Configuration loaded from NVM!");
  } else {
    Serial.println("No config in NVM. Using default values.");
  }
  prefs.end();
}

void saveConfig(const JsonDocument &doc) {
  prefs.begin("gw_cfg", false);
  
  if (doc.containsKey("wifi_ssid")) {
    prefs.putString("wifi_ssid", doc["wifi_ssid"].as<const char*>());
  }
  if (doc.containsKey("wifi_pass")) {
    prefs.putString("wifi_pass", doc["wifi_pass"].as<const char*>());
  }
  if (doc.containsKey("admin_pass")) {
    prefs.putString("admin_pass", doc["admin_pass"].as<const char*>());
  }
  if (doc.containsKey("use_static")) {
    prefs.putBool("use_static", doc["use_static"].as<bool>());
  }
  
  if (doc.containsKey("local_ip")) {
    IPAddress ip;
    if (ip.fromString(doc["local_ip"].as<const char*>())) {
      prefs.putUInt("local_ip", (uint32_t)ip);
    }
  }
  if (doc.containsKey("gateway_ip")) {
    IPAddress ip;
    if (ip.fromString(doc["gateway_ip"].as<const char*>())) {
      prefs.putUInt("gateway_ip", (uint32_t)ip);
    }
  }
  if (doc.containsKey("subnet_mask")) {
    IPAddress ip;
    if (ip.fromString(doc["subnet_mask"].as<const char*>())) {
      prefs.putUInt("subnet_mask", (uint32_t)ip);
    }
  }
  
  if (doc.containsKey("aes_key")) {
    String hexStr = doc["aes_key"] | "";
    if (hexStr.length() == 32) {
      uint8_t keyBytes[16];
      for (int i = 0; i < 16; i++) {
        char bytePart[3] = { hexStr[i * 2], hexStr[i * 2 + 1], 0 };
        keyBytes[i] = (uint8_t)strtol(bytePart, NULL, 16);
      }
      prefs.putBytes("aes_key", keyBytes, 16);
    }
  }
  
  prefs.putBool("configured", true);
  prefs.end();
  Serial.println("New configuration saved to NVM!");
}
