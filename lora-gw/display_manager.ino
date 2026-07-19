const char* getRssiBars(float rssi) {
  if (rssi >= -85.0f) return "....";
  if (rssi >= -95.0f) return "...";
  if (rssi >= -108.0f) return "..";
  return ".";
}

void changePage() {
  int active_count = 0;
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i].seen) active_count++;
  }
  
  int total_pages = active_count + 2; // Overview (1) + System (1) + N nodes
  current_page = (current_page + 1) % total_pages;
  scroll_index = 0;
  updateDisplay();
}

void scrollDown() {
  // Collect active nodes
  int active_indices[MAX_NODES];
  int active_count = 0;
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i].seen) {
      active_indices[active_count++] = i;
    }
  }

  if (current_page == 0) {
    // Scroll only online nodes (less than 300s)
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
    // Scroll system list
    scroll_index = (scroll_index + 1) % 6;
  } 
  else {
    // Scroll details of current node (current_page - 2)
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

  // Collect seen nodes
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
    // --- Page 0: Overview ---
    display.setCursor(0, 0);
    display.print("GW LoRa | Nodes");
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    // Collect only active nodes (less than 300s)
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
    // --- Page 1: System ---
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
    // --- Page 2+: Details of current Node ---
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
      String sensors_list = "";
      for (int j = 0; j < n.readings_count; j++) {
        uint8_t t = n.readings[j].type;
        ReadingTypeDefinition def = getReadingDefinition(t);
        String type_name = "";
        if (strstr(def.name, "temperature") != nullptr) type_name = "Temp";
        else if (strstr(def.name, "humidity") != nullptr) type_name = "Hum";
        else if (strstr(def.name, "pressure") != nullptr) type_name = "Pres";
        else if (strstr(def.name, "light") != nullptr) type_name = "Lux";
        else if (strstr(def.name, "battery") != nullptr) type_name = "Batt";
        else type_name = def.label;
        
        if (type_name.length() > 0 && sensors_list.indexOf(type_name) == -1) {
          sensors_list += type_name + " ";
        }
      }
      sensors_list.trim();
      det_items[det_count++] = "Sensors: " + (sensors_list.length() > 0 ? sensors_list : "None");
      det_items[det_count++] = "Seq:  " + String(n.seq);
      for (int j = 0; j < n.readings_count; j++) {
        uint8_t t = n.readings[j].type;
        float val = n.readings[j].value;
        ReadingTypeDefinition def = getReadingDefinition(t);
        det_items[det_count++] = String(def.label) + ": " + String(val * def.scale, (def.scale < 1.0f ? 1 : 0)) + " " + def.unit;
      }
      det_items[det_count++] = "RSSI: " + String(n.rssi, 0) + " dBm " + String(getRssiBars(n.rssi));
      det_items[det_count++] = "SNR:  " + String(n.snr, 1);
      det_items[det_count++] = "Reboots:  " + String(n.reboots);

      // Elapsed time since last reception
      uint32_t elapsed_sec = (now - n.last_seen_ms) / 1000;
      if (elapsed_sec < 60) {
        det_items[det_count++] = "Seen: " + String(elapsed_sec) + "s ago";
      } else {
        det_items[det_count++] = "Seen: " + String(elapsed_sec / 60) + "m ago";
      }

      // Loss rate and packets total
      det_items[det_count++] = "Loss: " + String(n.loss_percent, 1) + " %";
      det_items[det_count++] = "Packets: " + String(n.packets_count);

      // Human-readable reset reason
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

void handleButtonInteraction() {
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

    if (press_duration >= 50) { // Debounce
      if (press_duration >= 600) {
        changePage(); // LONG press -> Change page
      } else {
        scrollDown(); // SHORT press -> Scroll current page down
      }
    }
  }
}

void handleDisplayRefresh() {
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

  // Faster refresh (500ms) to animate search dots
  if (current_page == 0 && online_count == 0) {
    update_interval = 500;
  }

  if (millis() - last_display_update >= update_interval) {
    last_display_update = millis();
    updateDisplay();
  }
}

void initDisplay() {
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
}
