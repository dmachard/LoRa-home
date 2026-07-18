#include <AES.h>
#include <Arduino.h>
#include <GCM.h>
#include <RadioLib.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <Update.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_task_wdt.h>
#include "index_html.h"
#include "admin_html.h"
#include "update_html.h"

#define WDT_TIMEOUT_S 15

#define I2C_SDA 4
#define I2C_SCL 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PIN 9

int8_t last_active_node_id = -1;
uint8_t current_page = 0;          // Current page index
uint8_t scroll_index = 0;          // Scroll index offset
bool oled_initialized = false;

#include <Preferences.h>
#include <NimBLE-DataPipe.h>
#include <ArduinoJson.h>

Preferences prefs;
NimBLE_DataPipe* bleDataPipe = nullptr;
bool inConfigMode = false;
uint32_t bleStartMs = 0;
bool shouldReboot = false;

String wifi_ssid = "";
String wifi_pass = "";
String admin_pass = "admin";
bool use_static_ip = false;

IPAddress local_IP(0, 0, 0, 0);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(0, 0, 0, 0);

#define SPI_SCK 6
#define SPI_MISO 2
#define SPI_MOSI 7
#define LORA_CS 10
#define LORA_RST 11
#define LORA_BUSY 12
#define LORA_DIO1 13

#define LORA_FREQ 433.0
#define LORA_BW 125.0
#define LORA_SF 9
#define LORA_CR 5
#define LORA_SYNC RADIOLIB_SX126X_SYNC_WORD_PRIVATE
#define LORA_POWER 14
#define LORA_PREAMBLE 8

#define WINDOW_MS 300000UL

uint8_t AES_KEY[16] = {0};

#define TAG_SIZE 8
#define HDR_SIZE 9 // node_id(1) + seq(4) + random_id(4)

#include "shared_protocol.h"

GCM<AES128> gcm;
uint32_t global_malformed_packets = 0;
uint32_t global_unknown_nodes = 0;
uint32_t global_rx_interrupts = 0;
uint32_t boot_time_ms = 0;

SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
volatile bool rxFlag = false;
void IRAM_ATTR onReceive() { rxFlag = true; }

#define MAX_NODES 16
struct NodeData {
  char name[9]; // Local name with trailing \0
  uint32_t seq;
  uint32_t last_seen_ms;
  uint32_t packets_count;
  uint32_t auth_failures;
  uint32_t reboots;
  uint32_t window_seq_start;
  uint32_t window_received;
  uint32_t window_start_ms;
  float rssi;
  float snr;
  float loss_percent;
  uint8_t last_reset_reason;
  uint8_t last_error_code;
  uint16_t tx_interval;
  bool seen;
  
  uint8_t readings_count;
  SensorReading readings[6];
};

NodeData nodes[MAX_NODES];

const char* getRssiBars(float rssi) {
  if (rssi >= -85.0f) return "....";
  if (rssi >= -95.0f) return "...";
  if (rssi >= -108.0f) return "..";
  return ".";
}

WebServer server(8080);

// handleMetrics is now in web_server.ino
// Display manager functions are in display_manager.ino
// Web server endpoints are in web_server.ino

void loadConfig();
void setupBLE(bool isConfigured);
void loopBLE();
void setupWebServer();

void handleButtonInteraction(); // Declared in display_manager.ino
void handleDisplayRefresh();    // Declared in display_manager.ino
void processLoRaPacket();       // Declared in lora_manager.ino

void setup() {
  Serial.begin(115200);
  delay(1000);
  boot_time_ms = millis();
  memset(nodes, 0, sizeof(nodes));
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // I2C & OLED screen initialization
  Wire.begin(I2C_SDA, I2C_SCL);
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    oled_initialized = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("LoRa Gateway Init...");
    display.display();
  } else {
    Serial.println("SSD1306 allocation failed");
  }

  // Load configuration from NVM
  loadConfig();

  // Determine if BLE config mode should be forced
  prefs.begin("gw_cfg", false);
  bool configured = prefs.getBool("configured", false);
  prefs.end();

  // Read BOOT button (active LOW)
  delay(100);
  bool bootButtonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (!configured || bootButtonPressed) {
    inConfigMode = true;
    setupBLE(configured);
    return; // Stop normal setup
  }

  if (oled_initialized) {
    display.println("Init LoRa Radio...");
    display.display();
  }

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC,
                          LORA_POWER, LORA_PREAMBLE);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Init failed: %d\n", state);
    if (oled_initialized) {
      display.println("LoRa Radio: FAILED!");
      display.printf("Error code: %d\n", state);
      display.display();
    }
    while (true)
      delay(1000);
  }

  if (oled_initialized) {
    display.println("LoRa Radio: OK");
    display.println("Connecting WiFi...");
    display.display();
  }

  radio.setDio1Action(onReceive);
  radio.startReceive();

  WiFi.mode(WIFI_STA);
  if (use_static_ip) {
    if (!WiFi.config(local_IP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }
  }
  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  
  int wifi_retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_retry++;
    if (oled_initialized && wifi_retry % 2 == 0) {
      display.print(".");
      display.display();
    }
    if (wifi_retry > 40) { // 20 seconds timeout
      if (oled_initialized) {
        display.println("\nWiFi: FAILED!");
        display.display();
      }
      Serial.println("\nWiFi connection failed!");
      delay(2000);
      break;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());
  }
  updateDisplay();

  setupWebServer();

  esp_task_wdt_config_t wdt_config = {.timeout_ms = WDT_TIMEOUT_S * 1000,
                                      .idle_core_mask = 0,
                                      .trigger_panic = true};
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);
}

void loop() {
  if (inConfigMode) {
    loopBLE();
    return;
  }
  esp_task_wdt_reset();
  if (rxFlag) {
    processLoRaPacket();
  }
  server.handleClient();
  handleButtonInteraction();
  handleDisplayRefresh();
}
