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
uint8_t current_page = 0;          // Index de la page courante
uint8_t scroll_index = 0;          // Décalage de ligne pour le défilement
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

bool gcm_decrypt(const uint8_t *frame, uint8_t frame_len, uint8_t *payload,
                 uint8_t payload_size) {
  if (frame_len < HDR_SIZE + TAG_SIZE)
    return false;
  uint8_t payload_len = frame_len - HDR_SIZE - TAG_SIZE;
  if (payload_len > payload_size)
    return false;

  uint8_t iv[12] = {0};
  memcpy(iv, frame, 9);

  const uint8_t *ciphertext = frame + HDR_SIZE;
  const uint8_t *tag = frame + frame_len - TAG_SIZE;

  gcm.clear();
  gcm.setKey(AES_KEY, 16);
  gcm.setIV(iv, 12);
  gcm.addAuthData(frame, HDR_SIZE);
  gcm.decrypt(payload, ciphertext, payload_len);

  uint8_t computed_tag[TAG_SIZE];
  gcm.computeTag(computed_tag, TAG_SIZE);
  return memcmp(computed_tag, tag, TAG_SIZE) == 0;
}

SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
volatile bool rxFlag = false;
void IRAM_ATTR onReceive() { rxFlag = true; }

#define MAX_NODES 16
struct NodeData {
  char name[9]; // Nom local avec fin de chaine \0
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

void resetWindow(struct NodeData &n, uint32_t seq) {
  n.window_seq_start = seq;
  n.window_received = 1;
  n.window_start_ms = millis();
  n.loss_percent = 0.0f;
}

void updateWindow(struct NodeData &n, uint32_t seq) {
  uint32_t now = millis();
  if (now - n.window_start_ms >= WINDOW_MS) {
    uint32_t expected = seq - n.window_seq_start;
    if (expected > 0) {
      uint32_t lost = expected - min(n.window_received, expected);
      n.loss_percent = (lost * 100.0f) / expected;
    }
    resetWindow(n, seq);
    return;
  }
  n.window_received++;
}

WebServer server(8080);

// handleMetrics is now in web_server.ino
// Display manager functions are in display_manager.ino
// Web server endpoints are in web_server.ino

void loadConfig();
void setupBLE(bool isConfigured);
void loopBLE();
void setupWebServer();

void setup() {
  Serial.begin(115200);
  delay(1000);
  boot_time_ms = millis();
  memset(nodes, 0, sizeof(nodes));
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialisation I2C & Ecran OLED
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

  // Charger la configuration depuis la NVM
  loadConfig();

  // Déterminer s'il faut forcer le mode BLE de configuration
  prefs.begin("gw_cfg", false);
  bool configured = prefs.getBool("configured", false);
  prefs.end();

  // Lecture du bouton BOOT (actif à l'état BAS)
  delay(100);
  bool bootButtonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (!configured || bootButtonPressed) {
    inConfigMode = true;
    setupBLE(configured);
    return; // Arrêter le setup normal
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
    if (wifi_retry > 40) { // 20 secondes timeout
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
    rxFlag = false;
    global_rx_interrupts++;
    uint8_t frame[64];
    int state = radio.readData(frame, sizeof(frame));
    int len = radio.getPacketLength();

    if (state == RADIOLIB_ERR_NONE) {
      if (len >= HDR_SIZE + TAG_SIZE) {
        uint8_t node_id = frame[0];
        uint32_t seq = ((uint32_t)frame[1] << 24) | ((uint32_t)frame[2] << 16) |
                       ((uint32_t)frame[3] << 8) | ((uint32_t)frame[4]);
        uint8_t payload_len = len - HDR_SIZE - TAG_SIZE;
        uint8_t payload[64] = {0};

        if (node_id < MAX_NODES) {
          if (payload_len > sizeof(payload)) {
            global_malformed_packets++;
            Serial.printf("Node %d | Paquet trop grand (%d bytes)\n", node_id,
                          len);
            radio.startReceive();
            return;
          }
          NodeData &n = nodes[node_id];
          if (gcm_decrypt(frame, len, payload, sizeof(payload))) {
            uint8_t current_reset_reason = 0;
            uint8_t current_error_code = 0;

            bool is_sensor_payload = (payload_len >= sizeof(SensorPayload));
            SensorPayload sp;
            memset(&sp, 0, sizeof(sp));

            if (is_sensor_payload) {
              memcpy(&sp, payload, sizeof(SensorPayload));
              current_reset_reason = sp.reset_reason;
              current_error_code = sp.error_code;
            }

            if (n.seen) {
              if (seq == 0 &&
                  current_reset_reason == 1) { // 1 = ESP_RST_POWERON
                Serial.printf("Node %d | LEGITIMATE REBOOT (POWERON) DETECTE\n",
                              node_id);
                n.reboots++;
                resetWindow(n, seq);
              } else if (seq < n.seq) {
                Serial.printf("Node %d | WARNING: UNEXPECTED REBOOT DETECTE "
                              "(seq < n.seq)\n",
                              node_id);
                n.reboots++;
                resetWindow(n, seq);
              } else if (seq == n.seq) {
                // doublon, ignorer
              } else {
                updateWindow(n, seq);
              }
            } else {
              resetWindow(n, seq);
            }
            n.seen = true;
            n.seq = seq;
            n.rssi = radio.getRSSI();
            n.snr = radio.getSNR();
            n.last_seen_ms = millis();
            n.packets_count++;
            n.last_reset_reason = current_reset_reason;
            n.last_error_code = current_error_code;

            if (is_sensor_payload) {
              n.tx_interval = sp.tx_interval;
              memset(n.name, 0, sizeof(n.name));
              strncpy(n.name, sp.name, sizeof(n.name) - 1);

              n.readings_count = min((int)sp.count, 6);
              memcpy(n.readings, sp.readings, n.readings_count * sizeof(SensorReading));
            } else {
              n.readings_count = 0;
              Serial.printf("Node %d | Payload ignoré car taille inattendue : "
                            "recu=%d bytes\n",
                            node_id, payload_len);
            }
            last_active_node_id = node_id;
            updateDisplay();
          } else {
            n.auth_failures++;
            Serial.printf("Node %d | AUTH FAILED\n", node_id);
          }
        } else {
          global_unknown_nodes++;
          Serial.printf("Node %d | UNKNOWN NODE\n", node_id);
        }
      } else {
        global_malformed_packets++;
        Serial.printf("Packet too short: %d bytes\n", len);
      }
    } else {
      global_malformed_packets++;
    }
    radio.startReceive();
  }
  server.handleClient();

  // Lecture et gestion du bouton BOOT (Gestion Appui Court / Long)
  static bool last_btn_state = HIGH;
  static uint32_t press_start_time = 0;
  static bool was_pressed = false;

  bool btn_state = digitalRead(BUTTON_PIN);
  if (btn_state == LOW && !was_pressed) {
    press_start_time = millis();
    was_pressed = true;
  }
  else if (btn_state == HIGH && was_pressed) {
    uint32_t press_duration = millis() - press_start_time;
    was_pressed = false;

    if (press_duration >= 50) { // Anti-rebond
      if (press_duration >= 600) {
        // Appui LONG -> Changer de page
        changePage();
      } else {
        // Appui COURT -> Défiler la page courante vers le bas
        scrollDown();
      }
    }
  }
  last_btn_state = btn_state;

  // Mise à jour périodique des valeurs affichées (sans changer le défilement)
  static uint32_t last_display_update = 0;
  uint32_t update_interval = 2000;

  int online_count = 0;
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i].seen) {
      uint32_t elapsed_sec = (millis() - nodes[i].last_seen_ms) / 1000;
      if (elapsed_sec < 300) {
        online_count++;
      }
    }
  }

  // Rafraîchissement plus rapide (500ms) pour animer les points de recherche
  if (current_page == 0 && online_count == 0) {
    update_interval = 500;
  }

  if (millis() - last_display_update >= update_interval) {
    last_display_update = millis();
    updateDisplay();
  }
}
