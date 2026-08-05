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
#include <SensirionI2cScd4x.h>
#include <INA226.h>
#include <esp_mac.h>
#include "config.h"

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
  uint8_t lora_chip; // 0 = Auto, 1 = SX1278, 2 = SX1262
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

// Global random Node ID generated on boot (persisted in RTC memory across deep sleep)
RTC_DATA_ATTR uint32_t node_random_id = 0;

// LoRa sequence number stored in RTC memory across deep sleep
RTC_DATA_ATTR uint32_t seq = 0;

enum ResetReason {
  RESET_POWERON = 1,
  RESET_EXT = 2,
  RESET_SW = 3,
  RESET_PANIC = 4,
  RESET_WDT = 5,
  RESET_BROWNOUT = 6,
  RESET_DEEPSLEEP = 8,
  RESET_UNKNOWN = 0
};
ResetReason last_reset_reason = RESET_UNKNOWN;

enum ErrorCode {
  ERR_NONE = 0,
  ERR_SENSOR_READ = 1,
  ERR_TX_FAILED = 2
};
ErrorCode current_error_code = ERR_NONE;

struct LogEntry {
  uint32_t timestamp_ms;
  char message[48];
};
RTC_DATA_ATTR LogEntry logBuffer[10];
RTC_DATA_ATTR uint8_t logHead = 0;
RTC_DATA_ATTR uint8_t logCount = 0;

void addLog(const char* fmt, ...) {
  char buf[48];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  logBuffer[logHead].timestamp_ms = millis();
  strncpy(logBuffer[logHead].message, buf, sizeof(logBuffer[logHead].message) - 1);
  logBuffer[logHead].message[sizeof(logBuffer[logHead].message) - 1] = '\0';

  logHead = (logHead + 1) % 10;
  if (logCount < 10) logCount++;
}

Adafruit_AHTX0* aht = nullptr;
Adafruit_BMP280* bmp = nullptr;
Adafruit_TSL2561_Unified* tsl = nullptr;
SensirionI2cScd4x* scd4x = nullptr;
INA226* ina = nullptr;

bool aht_detected = false;
bool bmp_detected = false;
bool tsl_detected = false;
bool scd_detected = false;
bool ina_detected = false;
uint8_t bmp_addr = 0x77;
uint8_t tsl_addr = 0x39;
uint8_t ina_addr = 0x40;
PhysicalLayer* radio = nullptr;
GCM<AES128> gcm;

// Forward Declarations
bool loadConfig();
void saveConfig(const JsonDocument &doc);
void setupBLE(bool isConfigured);
void startLoRaMode();
void loopBLE();
void loopLoRa();

void setup() {
  gpio_hold_dis((gpio_num_t)8);
  Serial.begin(115200);
  if (esp_reset_reason() != ESP_RST_DEEPSLEEP) {
    delay(1000);
  }

  // I2C initialization and sensor scan
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);

  // Full I2C bus scan log for debugging
  Serial.print("I2C Bus Scan found devices at: ");
  int found_count = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("0x%02X ", address);
      found_count++;
    }
  }
  if (found_count == 0) Serial.print("None");
  Serial.println();

  // I2C scan for AHT20 (0x38)
  Wire.beginTransmission(0x38);
  if (Wire.endTransmission() == 0) {
    aht_detected = true;
  }

  // I2C scan for BMP280 (0x77 or 0x76)
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

  // I2C scan for TSL2561 (0x39, 0x29 or 0x49)
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

  // I2C scan for SCD41 (0x62)
  Wire.beginTransmission(0x62);
  if (Wire.endTransmission() == 0) {
    scd_detected = true;
  }

  // I2C scan for INA226 (0x40 through 0x4F)
  for (uint8_t addr = 0x40; addr <= 0x4F; addr++) {
    if (tsl_detected && addr == tsl_addr) continue;
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      ina_detected = true;
      ina_addr = addr;
      break;
    }
  }

  Serial.printf("I2C Scanner: AHT20=%d, BMP280=%d (0x%02X), TSL2561=%d (0x%02X), SCD41=%d, INA226=%d (0x%02X)\n",
                aht_detected, bmp_detected, bmp_addr, tsl_detected, tsl_addr, scd_detected, ina_detected, ina_addr);

  // Determine cause of last reset
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);
  switch (reason) {
    case ESP_RST_POWERON:   last_reset_reason = RESET_POWERON; break;
    case ESP_RST_EXT:       last_reset_reason = RESET_EXT; break;
    case ESP_RST_SW:        last_reset_reason = RESET_SW; break;
    case ESP_RST_PANIC:     last_reset_reason = RESET_PANIC; break;
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:       last_reset_reason = RESET_WDT; break;
    case ESP_RST_BROWNOUT:  last_reset_reason = RESET_BROWNOUT; break;
    case ESP_RST_DEEPSLEEP: last_reset_reason = RESET_DEEPSLEEP; break;
    default:                last_reset_reason = RESET_UNKNOWN; break;
  }

  // Generate random ID specific to this boot session (persisted during Deep Sleep)
  if (node_random_id == 0 || last_reset_reason != RESET_DEEPSLEEP) {
    node_random_id = esp_random();
  }
  addLog("Boot: reason=%d", (int)last_reset_reason);
  Serial.printf("Random Node ID: %08X\n", node_random_id);

  if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Turn off LED by default
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  if (EXT_BUTTON_PIN >= 0) pinMode(EXT_BUTTON_PIN, INPUT_PULLUP);

  // 1. Load NVM config and check status
  bool isConfigured = loadConfig();

  // Fallback for legacy NVM value 0 (Auto)
  if (config.lora_chip == 0) {
    config.lora_chip = LORA_HARDWARE_CHIP; // 2 for SX1262
  }

  // 2. Check if config mode should be forced (via NVM flag, BOOT button, or external RTC button)
  prefs.begin("lora_cfg", false);
  bool forceConfig = prefs.getBool("force_config", false);
  if (forceConfig) {
    prefs.putBool("force_config", false); // Consume flag for next reboot
  }
  prefs.end();

  // Check if woken up from Deep Sleep via external RTC button (GPIO 5)
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO || wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    forceConfig = true;
    Serial.println("External RTC button (GPIO 5) wakeup detected from Deep Sleep! Entering BLE Config Mode...");
  }

  // 3. Give 1.5s boot window allowing user to press BOOT button to enter BLE mode (cold boot only)
  if (last_reset_reason != RESET_DEEPSLEEP) {
    Serial.println("Press button to enter BLE Config Mode...");
    uint32_t startCheck = millis();
    while (millis() - startCheck < 1500) {
      if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);
        if (digitalRead(BUTTON_PIN) == LOW) {
          forceConfig = true;
          Serial.println("BOOT button press detected during startup window!");
          break;
        }
      }
      delay(20);
    }
  }

  if (!isConfigured || forceConfig) {
    inConfigMode = true;
    if (forceConfig) {
      Serial.println("--- STARTING BLE CONFIG MODE (Forced via BOOT button / software) ---");
    } else {
      Serial.println("--- STARTING BLE CONFIG MODE (Unconfigured) ---");
    }
    setupBLE(isConfigured);
  } else {
    inConfigMode = false;
    Serial.println("--- ENTERING NORMAL LORA MODE ---");
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
