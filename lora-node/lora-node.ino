#include <AES.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_TSL2561_U.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <GCM.h>
#include <NimBLE-DataPipe.h>
#include <Preferences.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <esp_mac.h>

#define WDT_TIMEOUT_S 30

// Default Pin Config (ESP32-C3 Hardware defaults)
#define SPI_SCK 6
#define SPI_MISO 2
#define SPI_MOSI 7
#define I2C_SDA 3
#define I2C_SCL 4

// LED and Button Config (BOOT button is always GPIO 9 on ESP32-C3)
#define BUTTON_PIN 9
int LED_PIN = 8;        // LED monochrome (ex: GPIO 10 sur Xiao ESP32-C3, GPIO 8 sur SuperMini. Mettre -1 si aucune LED)

// Node configuration structure
struct NodeConfig {
  uint8_t node_id;
  char node_name[8];
  float lora_freq;
  float lora_bw;
  uint8_t lora_sf;
  uint8_t lora_cr;
  uint8_t lora_sync;
  int8_t lora_power;
  uint16_t lora_preamble;
  uint8_t aes_key[16];
  uint16_t tx_interval;
};

NodeConfig config;
Preferences prefs;
uint32_t ota_total_size = 0;
uint32_t ota_received_bytes = 0;

// BLE Config parameters
#define BLE_SERVICE_UUID "F1E00001-C32A-4B28-86C7-67AB6B5D7A9F"
#define BLE_CHAR_UUID    "F1E00002-C32A-4B28-86C7-67AB6B5D7A9F"
NimBLE_DataPipe* bleDataPipe = nullptr;

bool inConfigMode = true;
uint32_t bleStartMs = 0;
const uint32_t BLE_TIMEOUT_MS = 60000; // 60 seconds advertising timeout
bool shouldReboot = false;

// Global random Node ID generated on boot
uint32_t node_random_id = 0;

// LoRa sequence number
uint32_t seq = 0;

enum ResetReason {
  RESET_POWERON = 1,
  RESET_EXT = 2,
  RESET_SW = 3,
  RESET_PANIC = 4,
  RESET_WDT = 5,
  RESET_BROWNOUT = 6,
  RESET_UNKNOWN = 0
};
ResetReason last_reset_reason = RESET_UNKNOWN;

enum ErrorCode {
  ERR_NONE = 0,
  ERR_SENSOR_READ = 1,
  ERR_TX_FAILED = 2
};
ErrorCode current_error_code = ERR_NONE;

Adafruit_AHTX0* aht = nullptr;
Adafruit_BMP280* bmp = nullptr;
Adafruit_TSL2561_Unified* tsl = nullptr;

bool aht_detected = false;
bool bmp_detected = false;
bool tsl_detected = false;
uint8_t bmp_addr = 0x77;
uint8_t tsl_addr = 0x39;
SX1278* radio = nullptr;
GCM<AES128> gcm;

// Forward Declarations
bool loadConfig();
void saveConfig(const JsonDocument &doc);
void setupBLE(bool isConfigured);
void startLoRaMode();
void loopBLE();
void loopLoRa();

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialisation I2C et scan des capteurs
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);

  // Scan I2C pour AHT20 (0x38)
  Wire.beginTransmission(0x38);
  if (Wire.endTransmission() == 0) {
    aht_detected = true;
  }

  // Scan I2C pour BMP280 (0x77 ou 0x76)
  Wire.beginTransmission(0x77);
  if (Wire.endTransmission() == 0) {
    bmp_detected = true;
    bmp_addr = 0x77;
  } else {
    Wire.beginTransmission(0x76);
    if (Wire.endTransmission() == 0) {
      bmp_detected = true;
      bmp_addr = 0x76;
    }
  }

  // Scan I2C pour TSL2561 (0x39, 0x29 ou 0x49)
  Wire.beginTransmission(0x39);
  if (Wire.endTransmission() == 0) {
    tsl_detected = true;
    tsl_addr = 0x39;
  } else {
    Wire.beginTransmission(0x29);
    if (Wire.endTransmission() == 0) {
      tsl_detected = true;
      tsl_addr = 0x29;
    } else {
      Wire.beginTransmission(0x49);
      if (Wire.endTransmission() == 0) {
        tsl_detected = true;
        tsl_addr = 0x49;
      }
    }
  }

  Serial.printf("Scanner I2C: AHT20=%d, BMP280=%d (0x%02X), TSL2561=%d (0x%02X)\n",
                aht_detected, bmp_detected, bmp_addr, tsl_detected, tsl_addr);

  // Détermine la cause du dernier reset
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);
  switch (reason) {
    case ESP_RST_POWERON: last_reset_reason = RESET_POWERON; break;
    case ESP_RST_EXT:     last_reset_reason = RESET_EXT; break;
    case ESP_RST_SW:      last_reset_reason = RESET_SW; break;
    case ESP_RST_PANIC:   last_reset_reason = RESET_PANIC; break;
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:     last_reset_reason = RESET_WDT; break;
    case ESP_RST_BROWNOUT:last_reset_reason = RESET_BROWNOUT; break;
    default:              last_reset_reason = RESET_UNKNOWN; break;
  }

  // Génère un ID aléatoire propre à cette session de boot
  node_random_id = esp_random();
  Serial.printf("Random Node ID: %08X\n", node_random_id);

  if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Éteint la LED par défaut
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 1. Charge la configuration NVM et récupère le statut
  bool isConfigured = loadConfig();

  // 2. Vérifie s'il faut forcer le mode configuration (via NVM flag ou bouton BOOT)
  prefs.begin("lora_cfg", false);
  bool forceConfig = prefs.getBool("force_config", false);
  if (forceConfig) {
    prefs.putBool("force_config", false); // Consomme le flag pour le prochain reboot
  }
  prefs.end();

  // Aussi vérifié si le bouton est physiquement enfoncé au boot
  if (digitalRead(BUTTON_PIN) == LOW) {
    forceConfig = true;
  }

  if (!isConfigured || forceConfig) {
    inConfigMode = true;
    if (forceConfig) {
      Serial.println("--- DÉMARRAGE MODE BLE CONFIG (Force par bouton BOOT / logiciel) ---");
    } else {
      Serial.println("--- DÉMARRAGE MODE BLE CONFIG (Non configure) ---");
    }
    setupBLE(isConfigured);
  } else {
    inConfigMode = false;
    Serial.println("--- PASSAGE DIRECT EN MODE LORA NORMAL ---");
    startLoRaMode();
  }
}

void loop() {
  if (inConfigMode) {
    loopBLE();
  } else {
    loopLoRa();
  }
}
