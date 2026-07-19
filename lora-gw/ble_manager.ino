#include <NimBLEDevice.h>
#include <NimBLE-DataPipe.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#define BLE_GW_SERVICE_UUID "F1E00003-C32A-4B28-86C7-67AB6B5D7A9F"
#define BLE_GW_CHAR_UUID    "F1E00004-C32A-4B28-86C7-67AB6B5D7A9F"

extern NimBLE_DataPipe* bleDataPipe;
extern bool inConfigMode;
extern uint32_t bleStartMs;
extern bool shouldReboot;
extern String wifi_ssid;
extern String wifi_pass;
extern String admin_pass;
extern bool use_static_ip;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern uint8_t AES_KEY[16];
extern bool oled_initialized;
extern Adafruit_SSD1306 display;

void saveConfig(const JsonDocument &doc);

void setupBLE(bool isConfigured) {
  // Configure OLED display for BLE mode
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String suffix = mac.substring(8);
  String devName = "ESP32-LoRa-Gateway-" + suffix;

  if (oled_initialized) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("[ BLE CONFIG MODE ]");
    display.println("");
    display.print("Name: GW-");
    display.println(suffix);
    display.println("");
    display.println("Connect using the");
    display.println("web portal.");
    display.display();
  }
  
  bleDataPipe = new NimBLE_DataPipe(devName.c_str(), BLE_GW_SERVICE_UUID, BLE_GW_CHAR_UUID);
  
  bleDataPipe->setOnJson([](const JsonDocument &doc) {
    String cmd = doc["cmd"] | "";
    if (cmd == "get_gw_config") {
      JsonDocument resp;
      resp["wifi_ssid"] = wifi_ssid;
      resp["wifi_pass"] = wifi_pass;
      resp["admin_pass"] = admin_pass;
      resp["use_static"] = use_static_ip;
      resp["local_ip"] = local_IP.toString();
      resp["gateway_ip"] = gateway.toString();
      resp["subnet_mask"] = subnet.toString();
      
      char keyHex[33];
      for (int i = 0; i < 16; i++) {
        sprintf(keyHex + i * 2, "%02x", AES_KEY[i]);
      }
      keyHex[32] = '\0';
      resp["aes_key"] = String(keyHex);
      
      bleDataPipe->sendJson(resp);
    } else if (cmd == "set_gw_config") {
      saveConfig(doc);
      JsonDocument resp;
      resp["status"] = "saved";
      bleDataPipe->sendJson(resp);
      shouldReboot = true;
    } else if (cmd == "reset_gw_config") {
      Preferences tempPrefs;
      tempPrefs.begin("gw_cfg", false);
      tempPrefs.clear();
      tempPrefs.end();
      
      JsonDocument resp;
      resp["status"] = "reseted";
      bleDataPipe->sendJson(resp);
      shouldReboot = true;
    }
  });
  
  bleDataPipe->begin();
  
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(false, false, true);

  // Custom BLE advertisement config for Linux/Chrome
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->stop(); // Stop default DataPipe advertising
  
  // Primary advertisement data (contains name and flags)
  NimBLEAdvertisementData advData;
  advData.setName(devName.c_str());
  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
  
  // Scan response data (contains service UUID)
  NimBLEAdvertisementData scanResponseData;
  scanResponseData.setCompleteServices(NimBLEUUID(BLE_GW_SERVICE_UUID));
  
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponseData(scanResponseData);
  pAdvertising->start();
  
  Serial.println("BLE Gateway server configured and active!");
  bleStartMs = millis();
}

void loopBLE() {
  if (shouldReboot) {
    Serial.println("Rebooting ESP32...");
    ESP.restart();
  }
}
