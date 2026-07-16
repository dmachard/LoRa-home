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

#define TYPE_DHT22_TEMP  0x01
#define TYPE_DHT22_HUM   0x02
#define TYPE_AHT20_TEMP  0x03
#define TYPE_AHT20_HUM   0x04
#define TYPE_BMP280_TEMP 0x05
#define TYPE_BMP280_PRES 0x06
#define TYPE_BH1750_LUX  0x07
#define TYPE_BATTERY     0x08

struct __attribute__((packed)) SensorReading {
  uint8_t type;       // Type de mesure (ex: TYPE_AHT20_TEMP...)
  int32_t value;      // Valeur brute
};

struct __attribute__((packed)) SensorPayload {
  uint8_t count;               // Nombre de mesures dans le tableau (max 6)
  SensorReading readings[6];   // Tableau des mesures
  uint8_t reset_reason;
  uint8_t error_code;
  uint16_t tx_interval;
  char name[8];
};

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
  float temperature;
  float humidity;
  float pressure;
  float temp_aht;
  float temp_bmp;
  uint16_t light;
  uint16_t battery;
  uint8_t type;
  uint8_t last_reset_reason;
  uint8_t last_error_code;
  uint16_t tx_interval;
  bool seen;
  bool has_sensor;
  bool has_pressure;
  bool has_light;
  bool has_battery;
  bool has_temp_aht;
  bool has_temp_bmp;
  bool has_humidity;
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

void handleMetrics() {
  String out;
  out.reserve(2048);
  out += "# HELP lora_rssi_dbm Last RSSI\n";
  out += "# TYPE lora_rssi_dbm gauge\n";
  out += "# HELP lora_snr_db Last SNR\n";
  out += "# TYPE lora_snr_db gauge\n";
  out += "# HELP lora_last_seen_seconds Seconds since last packet\n";
  out += "# TYPE lora_last_seen_seconds gauge\n";
  out += "# HELP lora_packet_seq Last sequence number\n";
  out += "# TYPE lora_packet_seq counter\n";
  out += "# HELP lora_packet_loss_percent Packet loss over last window\n";
  out += "# TYPE lora_packet_loss_percent gauge\n";
  out += "# HELP lora_auth_failures Total authentication failures\n";
  out += "# TYPE lora_auth_failures counter\n";
  out += "# HELP lora_reboots Total node reboots detected\n";
  out += "# TYPE lora_reboots counter\n";
  out += "# HELP lora_temperature_celsius Temperature from node\n";
  out += "# TYPE lora_temperature_celsius gauge\n";
  out += "# HELP lora_humidity_percent Humidity from node\n";
  out += "# TYPE lora_humidity_percent gauge\n";
  out += "# HELP lora_pressure_hpa Atmospheric pressure in hPa\n";
  out += "# TYPE lora_pressure_hpa gauge\n";
  out += "# HELP lora_light_lux Light level in Lux\n";
  out += "# TYPE lora_light_lux gauge\n";
  out += "# HELP lora_battery_millivolts Node battery voltage in mV\n";
  out += "# TYPE lora_battery_millivolts gauge\n";
  out += "# HELP lora_node_reset_reason Last ESP Reset Reason (1=POWERON)\n";
  out += "# TYPE lora_node_reset_reason gauge\n";
  out += "# HELP lora_node_error_code Node Error Code (0=OK)\n";
  out += "# TYPE lora_node_error_code gauge\n";
  out += "# HELP lora_global_malformed_packets_total Total malformed packets "
         "received\n";
  out += "# HELP lora_global_malformed_packets_total Total malformed packets "
         "received\n";
  out += "# TYPE lora_global_malformed_packets_total counter\n";
  out += "# HELP lora_global_unknown_nodes_total Total packets from unknown "
         "node IDs\n";
  out += "# TYPE lora_global_unknown_nodes_total counter\n";
  out += "# HELP lora_global_rx_interrupts_total Total radio interrupts "
         "received\n";
  out += "# TYPE lora_global_rx_interrupts_total counter\n";
  out += "# HELP lora_gateway_uptime_seconds Gateway uptime in seconds\n";
  out += "# TYPE lora_gateway_uptime_seconds gauge\n";
  out +=
      "# HELP lora_packets_received_total Total successful packets per node\n";
  out += "# TYPE lora_packets_received_total counter\n";

  uint32_t now = millis();
  for (int i = 0; i < MAX_NODES; i++) {
    if (!nodes[i].seen)
      continue;
    char label[64];
    if (nodes[i].name[0] != '\0') {
      snprintf(label, sizeof(label), "{node=\"%d\",name=\"%s\"}", i, nodes[i].name);
    } else {
      snprintf(label, sizeof(label), "{node=\"%d\"}", i);
    }
    char line[128];
    snprintf(line, sizeof(line), "lora_rssi_dbm%s %.1f\n", label,
             nodes[i].rssi);
    out += line;
    snprintf(line, sizeof(line), "lora_snr_db%s %.1f\n", label, nodes[i].snr);
    out += line;
    snprintf(line, sizeof(line), "lora_last_seen_seconds%s %.1f\n", label,
             (now - nodes[i].last_seen_ms) / 1000.0f);
    out += line;
    snprintf(line, sizeof(line), "lora_packet_seq%s %lu\n", label,
             nodes[i].seq);
    out += line;
    snprintf(line, sizeof(line), "lora_packet_loss_percent%s %.1f\n", label,
             nodes[i].loss_percent);
    out += line;
    snprintf(line, sizeof(line), "lora_auth_failures%s %lu\n", label,
             nodes[i].auth_failures);
    out += line;
    snprintf(line, sizeof(line), "lora_reboots%s %lu\n", label,
             nodes[i].reboots);
    out += line;
    snprintf(line, sizeof(line), "lora_packets_received_total%s %lu\n", label,
             nodes[i].packets_count);
    out += line;
    snprintf(line, sizeof(line), "lora_node_reset_reason%s %u\n", label,
             nodes[i].last_reset_reason);
    out += line;
    snprintf(line, sizeof(line), "lora_node_error_code%s %u\n", label,
             nodes[i].last_error_code);
    out += line;
    if (nodes[i].has_sensor) {
      snprintf(line, sizeof(line), "lora_temperature_celsius%s %.2f\n", label,
               nodes[i].temperature);
      out += line;
      snprintf(line, sizeof(line), "lora_humidity_percent%s %.2f\n", label,
               nodes[i].humidity);
      out += line;
    }
    if (nodes[i].has_pressure) {
      snprintf(line, sizeof(line), "lora_pressure_hpa%s %.2f\n", label,
               nodes[i].pressure);
      out += line;
    }
    if (nodes[i].has_light) {
      snprintf(line, sizeof(line), "lora_light_lux%s %u\n", label,
               nodes[i].light);
      out += line;
    }
    if (nodes[i].has_battery) {
      snprintf(line, sizeof(line), "lora_battery_millivolts%s %u\n", label,
               nodes[i].battery);
      out += line;
    }
  }
  char global_line[128];
  snprintf(global_line, sizeof(global_line),
           "lora_global_malformed_packets_total %lu\n",
           global_malformed_packets);
  out += global_line;
  snprintf(global_line, sizeof(global_line),
           "lora_global_unknown_nodes_total %lu\n", global_unknown_nodes);
  out += global_line;

  server.send(200, "text/plain; version=0.0.4; charset=utf-8", out);
}

void changePage() {
  int active_count = 0;
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i].seen) active_count++;
  }
  
  int total_pages = active_count + 2; // Overview (1) + Système (1) + N nœuds
  current_page = (current_page + 1) % total_pages;
  scroll_index = 0;
  updateDisplay();
}

void scrollDown() {
  // Collecter les nœuds actifs
  int active_indices[MAX_NODES];
  int active_count = 0;
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i].seen) {
      active_indices[active_count++] = i;
    }
  }

  if (current_page == 0) {
    // Défiler uniquement les nœuds en ligne (moins de 300s)
    int online_count = 0;
    uint32_t now = millis();
    for (int i = 0; i < MAX_NODES; i++) {
      if (nodes[i].seen) {
        uint32_t elapsed_sec = (now - nodes[i].last_seen_ms) / 1000;
        if (elapsed_sec < 300) {
          online_count++;
        }
      }
    }
    if (online_count > 0) {
      scroll_index = (scroll_index + 1) % online_count;
    }
  }
  else if (current_page == 1) {
    // Défiler la liste système
    scroll_index = (scroll_index + 1) % 6;
  } 
  else {
    // Défiler les détails du nœud courant (current_page - 2)
    scroll_index = scroll_index + 1;
  }
  updateDisplay();
}

void updateDisplay() {
  if (!oled_initialized) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  uint32_t now = millis();

  // Collecter les nœuds vus
  int active_indices[MAX_NODES];
  int active_count = 0;
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i].seen) {
      active_indices[active_count++] = i;
    }
  }

  int total_pages = active_count + 2;
  if (current_page >= total_pages) {
    current_page = 0;
  }

  if (current_page == 0) {
    // --- Page 0: Overview (Vue d'ensemble) ---
    display.setCursor(0, 0);
    display.print("GW LoRa | Nodes");
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    // Collecter uniquement les nœuds actifs (moins de 300s)
    int online_indices[MAX_NODES];
    int online_count = 0;
    for (int i = 0; i < MAX_NODES; i++) {
      if (nodes[i].seen) {
        uint32_t elapsed_sec = (now - nodes[i].last_seen_ms) / 1000;
        if (elapsed_sec < 300) {
          online_indices[online_count++] = i;
        }
      }
    }

    if (online_count > 0) {
      scroll_index = scroll_index % online_count;
      for (int row = 0; row < 4; row++) {
        if (row >= online_count) break;
        int idx = online_indices[(scroll_index + row) % online_count];
        int y = 15 + row * 11;
        display.setCursor(0, y);

        const char* bars = getRssiBars(nodes[idx].rssi);
        char node_label[9];
        if (nodes[idx].name[0] != '\0') {
          snprintf(node_label, sizeof(node_label), "%-8.8s", nodes[idx].name);
        } else {
          snprintf(node_label, sizeof(node_label), "Node %-2d", idx);
        }
        display.printf("%-16.16s%-4.4s", node_label, bars);
      }
    } else {
      display.setCursor(0, 24);
      display.print("No active nodes");
      display.setCursor(0, 42);
      int dot_count = (millis() / 500) % 4;
      char dots[4] = {0};
      for (int i = 0; i < dot_count; i++) dots[i] = '.';
      display.printf("Searching signal%s", dots);
    }
  } 
  else if (current_page == 1) {
    // --- Page 1: Système ---
    display.setCursor(0, 0);
    display.print("GW LoRa | System");
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    String sys_items[6];
    sys_items[0] = "IP:   " + WiFi.localIP().toString();
    sys_items[1] = "SSID: " + wifi_ssid;

    uint32_t sec = now / 1000;
    uint32_t min = sec / 60;
    uint32_t hr = min / 60;
    char up_str[32];
    if (hr > 0) {
      snprintf(up_str, sizeof(up_str), "Up:   %luh%02lum%02lus", hr, min % 60, sec % 60);
    } else {
      snprintf(up_str, sizeof(up_str), "Up:   %lum %lus", min, sec % 60);
    }
    sys_items[2] = String(up_str);
    sys_items[3] = "WiFi: " + String(WiFi.RSSI()) + " dBm";
    sys_items[4] = "RAM:  " + String(ESP.getFreeHeap() / 1024) + " KB";
    sys_items[5] = "Ints: " + String(global_rx_interrupts);

    scroll_index = scroll_index % 6;
    for (int row = 0; row < 4; row++) {
      int idx = (scroll_index + row) % 6;
      display.setCursor(0, 15 + row * 11);
      display.print(sys_items[idx]);
    }
  }
  else {
    // --- Page 2+: Détails du Nœud courant ---
    int idx = current_page - 2;
    if (idx < active_count) {
      int node_id = active_indices[idx];
      NodeData &n = nodes[node_id];

      display.setCursor(0, 0);
      if (n.name[0] != '\0') {
        display.printf("GW LoRa | %.7s", n.name);
      } else {
        display.printf("GW LoRa | Node %d", node_id);
      }
      display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

      String det_items[24];
      int det_count = 0;

      if (n.name[0] != '\0') {
        det_items[det_count++] = "Name: " + String(n.name);
      }
      det_items[det_count++] = "Node: ID " + String(node_id);
      String type_str;
      if (n.type == 1) type_str = "DHT22";
      else if (n.type == 2) type_str = "AHT20";
      else if (n.type == 3) type_str = "BMP280";
      else if (n.type == 4) type_str = "Multi";
      else type_str = "Unknown";
      det_items[det_count++] = "Type: " + type_str;
      det_items[det_count++] = "Seq:  " + String(n.seq);
      if (n.has_sensor) {
        det_items[det_count++] = "Temp: " + String(n.temperature, 1) + " C";
        det_items[det_count++] = "Hum:  " + String(n.humidity, 1) + " %";
      }
      if (n.has_pressure) {
        det_items[det_count++] = "Pres: " + String(n.pressure, 1) + " hPa";
      }
      if (n.has_light) {
        det_items[det_count++] = "Lux:  " + String(n.light) + " lx";
      }
      if (n.has_battery) {
        det_items[det_count++] = "Batt: " + String(n.battery) + " mV";
      }
      det_items[det_count++] = "RSSI: " + String(n.rssi, 0) + " dBm " + String(getRssiBars(n.rssi));
      det_items[det_count++] = "SNR:  " + String(n.snr, 1);
      det_items[det_count++] = "Reboots:  " + String(n.reboots);

      // Temps écoulé depuis la dernière réception
      uint32_t elapsed_sec = (now - n.last_seen_ms) / 1000;
      if (elapsed_sec < 60) {
        det_items[det_count++] = "Seen: " + String(elapsed_sec) + "s ago";
      } else {
        det_items[det_count++] = "Seen: " + String(elapsed_sec / 60) + "m ago";
      }

      // Taux de perte et total des paquets
      det_items[det_count++] = "Loss: " + String(n.loss_percent, 1) + " %";
      det_items[det_count++] = "Packets: " + String(n.packets_count);

      // Raison de reboot humaine
      String rst_str;
      switch(n.last_reset_reason) {
        case 1: rst_str = "PowerOn"; break;
        case 3: rst_str = "Software"; break;
        case 4: rst_str = "Panic"; break;
        case 5:
        case 6:
        case 7: rst_str = "Watchdog"; break;
        case 8: rst_str = "DeepSleep"; break;
        case 9: rst_str = "Brownout"; break;
        default: rst_str = "Code " + String(n.last_reset_reason);
      }
      det_items[det_count++] = "Reset: " + rst_str;
      det_items[det_count++] = "ErrCode: " + String(n.last_error_code);

      scroll_index = scroll_index % det_count;
      for (int row = 0; row < 4; row++) {
        int idx_det = (scroll_index + row) % det_count;
        display.setCursor(0, 15 + row * 11);
        display.print(det_items[idx_det]);
      }
    }
  }
  display.display();
}


void handleNodesJson() {
  String json = "{\"nodes\":[";
  bool first = true;
  uint32_t now = millis();
  for (int i = 0; i < MAX_NODES; i++) {
    if (!nodes[i].seen) continue;
    
    // Cache les nœuds inactifs depuis plus de 5 minutes
    if (now - nodes[i].last_seen_ms > 300000UL) continue;

    if (!first) json += ",";
    first = false;

    json += "{";
    json += "\"id\":" + String(i) + ",";
    json += "\"name\":\"" + String(nodes[i].name) + "\",";
    json += "\"seq\":" + String(nodes[i].seq) + ",";
    json += "\"rssi\":" + String(nodes[i].rssi, 1) + ",";
    json += "\"snr\":" + String(nodes[i].snr, 1) + ",";
    json += "\"reboots\":" + String(nodes[i].reboots) + ",";
    json += "\"loss_percent\":" + String(nodes[i].loss_percent, 1) + ",";
    json += "\"packets_count\":" + String(nodes[i].packets_count) + ",";
    json += "\"last_reset_reason\":" + String(nodes[i].last_reset_reason) + ",";
    json += "\"last_error_code\":" + String(nodes[i].last_error_code) + ",";
    json += "\"tx_interval\":" + String(nodes[i].tx_interval) + ",";
    
    String type_str;
    if (nodes[i].type == 1) type_str = "DHT22";
    else if (nodes[i].type == 2) type_str = "AHT20";
    else if (nodes[i].type == 3) type_str = "BMP280";
    else if (nodes[i].type == 4) type_str = "Multi";
    else type_str = "Unknown";
    json += "\"type\":\"" + type_str + "\",";

    json += "\"has_sensor\":" + String(nodes[i].has_sensor ? "true" : "false") + ",";
    json += "\"temperature\":" + String(nodes[i].temperature, 2) + ",";
    json += "\"has_humidity\":" + String(nodes[i].has_humidity ? "true" : "false") + ",";
    json += "\"humidity\":" + String(nodes[i].humidity, 2) + ",";
    json += "\"has_temp_aht\":" + String(nodes[i].has_temp_aht ? "true" : "false") + ",";
    json += "\"temp_aht\":" + String(nodes[i].temp_aht, 2) + ",";
    json += "\"has_temp_bmp\":" + String(nodes[i].has_temp_bmp ? "true" : "false") + ",";
    json += "\"temp_bmp\":" + String(nodes[i].temp_bmp, 2) + ",";

    json += "\"has_pressure\":" + String(nodes[i].has_pressure ? "true" : "false") + ",";
    json += "\"pressure\":" + String(nodes[i].pressure, 1) + ",";

    json += "\"has_light\":" + String(nodes[i].has_light ? "true" : "false") + ",";
    json += "\"light\":" + String(nodes[i].light) + ",";

    json += "\"has_battery\":" + String(nodes[i].has_battery ? "true" : "false") + ",";
    json += "\"battery\":" + String(nodes[i].battery) + ",";

    uint32_t elapsed_sec = (now - nodes[i].last_seen_ms) / 1000;
    json += "\"elapsed_sec\":" + String(elapsed_sec);
    json += "}";
  }
  json += "],";
  json += "\"gateway\":{";
  json += "\"uptime_sec\":" + String(millis() / 1000) + ",";
  json += "\"free_heap_kb\":" + String(ESP.getFreeHeap() / 1024) + ",";
  json += "\"rx_interrupts\":" + String(global_rx_interrupts) + ",";
  json += "\"malformed_packets\":" + String(global_malformed_packets) + ",";
  json += "\"wifi_rssi\":" + String(WiFi.RSSI());
  json += "}";
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleAdminHtml() {
  if (!server.authenticate("admin", admin_pass.c_str())) {
    server.requestAuthentication(BASIC_AUTH, "LoRa Gateway Admin", "Authentification requise");
    return;
  }
  server.send(200, "text/html", ADMIN_HTML);
}

void saveConfig(const JsonDocument &doc);

void handleSaveConfigHttp() {
  if (!server.authenticate("admin", admin_pass.c_str())) {
    server.requestAuthentication(BASIC_AUTH, "LoRa Gateway Admin", "Authentification requise");
    return;
  }
  String body = server.arg("plain");
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  saveConfig(doc);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"status\":\"saved\"}");
  delay(1000);
  shouldReboot = true;
}

void handleResetConfigHttp() {
  if (!server.authenticate("admin", admin_pass.c_str())) {
    server.requestAuthentication(BASIC_AUTH, "LoRa Gateway Admin", "Authentification requise");
    return;
  }
  Preferences tempPrefs;
  tempPrefs.begin("gw_cfg", false);
  tempPrefs.clear();
  tempPrefs.end();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"status\":\"reseted\"}");
  delay(1000);
  shouldReboot = true;
}

void handleGetConfigHttp() {
  if (!server.authenticate("admin", admin_pass.c_str())) {
    server.requestAuthentication(BASIC_AUTH, "LoRa Gateway Admin", "Authentification requise");
    return;
  }
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
  
  String out;
  serializeJson(resp, out);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", out);
}

void handleRootHtml() {
  server.send(200, "text/html", INDEX_HTML);
}

void loadConfig();
void setupBLE(bool isConfigured);
void loopBLE();

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

  server.on("/admin", handleAdminHtml);
  server.on("/metrics", handleMetrics);
  server.on("/api/nodes", handleNodesJson);
  server.on("/api/gw_config", HTTP_GET, handleGetConfigHttp);
  server.on("/api/gw_config", HTTP_POST, handleSaveConfigHttp);
  server.on("/api/gw_reset", HTTP_POST, handleResetConfigHttp);
  server.on("/", handleRootHtml);

  // Page de formulaire d'upload OTA
  server.on("/update", HTTP_GET, []() {
    if (!server.authenticate("admin", admin_pass.c_str())) {
      server.requestAuthentication(BASIC_AUTH, "LoRa Gateway Admin", "Authentification requise");
      return;
    }
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", UPDATE_HTML);
  });

  // Handler POST pour le flashage
  server.on("/update", HTTP_POST, []() {
    if (!server.authenticate("admin", admin_pass.c_str())) {
      server.requestAuthentication(BASIC_AUTH, "LoRa Gateway Admin", "Authentification requise");
      return;
    }
    server.sendHeader("Connection", "close");
    if (Update.hasError()) {
      server.send(200, "text/html", UPDATE_ERR_HTML);
    } else {
      server.send(200, "text/html", UPDATE_OK_HTML);
      delay(1000);
      ESP.restart();
    }
  }, []() {
    if (!server.authenticate("admin", admin_pass.c_str())) {
      return;
    }
    HTTPUpload& upload = server.upload();
    esp_task_wdt_reset(); // Securité watchdog pendant le transfert de fichier
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Mise a jour: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Reussi: %u octets. Redemarrage...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();

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
              n.type = 0; // Déterminé par les mesures
              n.has_sensor = false;
              n.has_pressure = false;
              n.has_light = false;
              n.has_battery = false;
              n.has_temp_aht = false;
              n.has_temp_bmp = false;
              n.has_humidity = false;

              memset(n.name, 0, sizeof(n.name));
              strncpy(n.name, sp.name, sizeof(n.name) - 1);

              for (int j = 0; j < sp.count && j < 6; j++) {
                switch (sp.readings[j].type) {
                  case TYPE_DHT22_TEMP:
                    n.temperature = sp.readings[j].value / 100.0f;
                    n.has_sensor = true;
                    if (n.type == 0) n.type = 1; // DHT22
                    break;
                  case TYPE_DHT22_HUM:
                    n.humidity = sp.readings[j].value / 100.0f;
                    n.has_sensor = true;
                    n.has_humidity = true;
                    if (n.type == 0) n.type = 1; // DHT22
                    break;
                  case TYPE_AHT20_TEMP:
                    n.temperature = sp.readings[j].value / 100.0f;
                    n.temp_aht = sp.readings[j].value / 100.0f;
                    n.has_sensor = true;
                    n.has_temp_aht = true;
                    if (n.type == 0) n.type = 2; // AHT20
                    break;
                  case TYPE_AHT20_HUM:
                    n.humidity = sp.readings[j].value / 100.0f;
                    n.has_sensor = true;
                    n.has_humidity = true;
                    if (n.type == 0) n.type = 2; // AHT20
                    break;
                  case TYPE_BMP280_TEMP:
                    n.temperature = sp.readings[j].value / 100.0f;
                    n.temp_bmp = sp.readings[j].value / 100.0f;
                    n.has_sensor = true;
                    n.has_temp_bmp = true;
                    if (n.type != 0 && n.type != 3) n.type = 4; // Multi
                    else if (n.type == 0) n.type = 3; // BMP280
                    break;
                  case TYPE_BMP280_PRES:
                    n.pressure = sp.readings[j].value / 10.0f;
                    n.has_pressure = true;
                    if (n.type != 0 && n.type != 3) n.type = 4; // Multi
                    else if (n.type == 0) n.type = 3; // BMP280
                    break;
                  case TYPE_BH1750_LUX:
                    n.light = (uint16_t)sp.readings[j].value;
                    n.has_light = true;
                    break;
                  case TYPE_BATTERY:
                    n.battery = (uint16_t)sp.readings[j].value;
                    n.has_battery = true;
                    break;
                }
              }
            } else {
              n.has_sensor = false;
              n.has_pressure = false;
              n.has_light = false;
              n.has_battery = false;
              n.has_temp_aht = false;
              n.has_temp_bmp = false;
              n.has_humidity = false;
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
