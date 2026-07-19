/**
 * ESP32-C6 LoRa Gateway entry point.
 * Coordinates initialization of I2C/OLED, BLE Configuration, LoRa Radio,
 * Wi-Fi connection, Web Server, and Watchdog timer.
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <NimBLE-DataPipe.h>
#include <RadioLib.h>
#include <AES.h>
#include <GCM.h>
#include <esp_task_wdt.h>
#include <Adafruit_SSD1306.h>

#include "shared_protocol.h"
#include "node_data.h"
#include "config.h"

// ==========================================
// SYSTEM & WDT CONFIGURATION
// ==========================================
uint32_t boot_time_ms = 0;
bool shouldReboot = false;
uint32_t reboot_requested_ms = 0;

// ==========================================
// I2C & OLED SCREEN CONFIGURATION
// ==========================================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oled_initialized = false;

// ==========================================
// PERSISTENT STORAGE & CONFIG MODE
// ==========================================
Preferences prefs;
NimBLE_DataPipe* bleDataPipe = nullptr;
bool inConfigMode = false;
uint32_t bleStartMs = 0;

// ==========================================
// GATEWAY NETWORK CREDENTIALS & SETTINGS
// ==========================================
String wifi_ssid = "";
String wifi_pass = "";
String admin_pass = "admin";
bool use_static_ip = false;

IPAddress local_IP(0, 0, 0, 0);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(0, 0, 0, 0);

// ==========================================
// SECURITY & AES-GCM PROPERTIES
// ==========================================
uint8_t AES_KEY[16] = {0};
GCM<AES128> gcm;

// ==========================================
// MONITORING & DIAGNOSTIC COUNTERS
// ==========================================
uint32_t global_malformed_packets = 0;
uint32_t global_unknown_nodes = 0;
uint32_t global_rx_interrupts = 0;

// ==========================================
// LORA PACKET MANAGEMENT
// ==========================================
extern volatile bool rxFlag;

// ==========================================
// NODES DATABASE (RAM RESIDENT)
// ==========================================
NodeData nodes[MAX_NODES];
int8_t last_active_node_id = -1;
uint8_t current_page = 0;          // Current screen page index
uint8_t scroll_index = 0;          // Scroll index offset

// ==========================================
// WEB SERVER INSTANCE
// ==========================================
WebServer server(8080);

// ==========================================
// FUNCTION DECLARATIONS (PROTOTYPES)
// ==========================================

// Config Manager
bool loadConfig();
void saveConfig(const JsonDocument &doc);
bool checkConfigMode();

// BLE Manager
void setupBLE(bool isConfigured);
void loopBLE();

// Web Server
void setupWebServer();

// Display Manager
void initDisplay();
void changePage();
void scrollDown();
void updateDisplay();
void handleButtonInteraction();
void handleDisplayRefresh();

// LoRa Manager
void initLoRa();
void processLoRaPacket();

// WiFi Manager
void initWiFi();

// ==========================================
// COMPONENT INITIALIZATION HELPERS & MAIN
// ==========================================

void initWDT() {
  esp_task_wdt_config_t wdt_config = {.timeout_ms = WDT_TIMEOUT_S * 1000,
                                      .idle_core_mask = 0,
                                      .trigger_panic = true};
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);
}

void setup() {
  // Initialize serial logs
  Serial.begin(115200);
  delay(1000);
  boot_time_ms = millis();
  
  // Clear node list and set up the physical button pin
  memset(nodes, 0, sizeof(nodes));
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize SSD1306 OLED screen via I2C
  initDisplay();

  // If gateway is not configured or BOOT button is held, enter BLE config mode
  if (checkConfigMode()) {
    return; // Normal setup is bypassed, loop() will only handle BLE configuration
  }

  // Initialize core hardware and networking services
  initLoRa();          // Start LoRa radio and register interrupt
  initWiFi();          // Connect to WiFi network
  setupWebServer();    // Start HTTP server (Prometheus metrics & OTA)
  initWDT();           // Start Watchdog Timer for soft-lockup protection
}

void loop() {
  // If in config mode, execute only the BLE config loop
  if (inConfigMode) {
    loopBLE();
    return;
  }

  // Non-blocking reboot: trigger 1s after the request to let the HTTP response flush
  if (shouldReboot) {
    if (reboot_requested_ms == 0) reboot_requested_ms = millis();
    if (millis() - reboot_requested_ms >= 1000) {
      Serial.println("Rebooting ESP32...");
      ESP.restart();
    }
  }

  // Normal operational cycle
  esp_task_wdt_reset();      // Reset watchdog timer

  if (rxFlag) {
    processLoRaPacket();     // Decode and process received packet on interrupt
  }

  server.handleClient();      // Handle incoming HTTP client requests
  handleButtonInteraction();  // Poll button to scroll display pages
  handleDisplayRefresh();     // Periodically refresh the OLED dashboard
}
