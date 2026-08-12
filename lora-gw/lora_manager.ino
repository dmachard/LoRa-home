#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

extern uint8_t gw_lora_chip;
extern float gw_lora_freq;
extern uint8_t gw_lora_sync;
extern uint8_t gw_lora_sf;
extern float gw_lora_bw;
extern uint8_t gw_lora_cr;
PhysicalLayer* radio = nullptr;

QueueHandle_t loraQueue = NULL;

extern uint32_t global_rx_interrupts;
extern uint32_t global_queue_overflows;
extern uint32_t global_radio_reads;
extern uint32_t global_radio_errors;
extern uint32_t global_radio_err_crc;
extern uint32_t global_radio_err_header;
extern uint32_t global_radio_err_timeout;
extern uint32_t global_radio_err_other;
extern uint32_t global_valid_packets;
extern uint32_t global_rx_processing_us;
extern uint32_t global_total_processing_us;

void IRAM_ATTR onReceive() {
  global_rx_interrupts++;
  uint8_t dummy = 1;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xQueueSendFromISR(loraQueue, &dummy, &xHigherPriorityTaskWoken) != pdTRUE) {
    global_queue_overflows++;
  }
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

void loraTask(void *pvParameters) {
  uint8_t dummy;
  for (;;) {
    if (xQueueReceive(loraQueue, &dummy, portMAX_DELAY) == pdTRUE) {
      processLoRaPacket();
    }
  }
}

bool gcm_encrypt_ack(uint8_t target_node_id, uint32_t seq, uint8_t status, uint8_t *out_frame, uint8_t *out_len) {
  out_frame[0] = target_node_id;
  out_frame[1] = (seq >> 24) & 0xFF;
  out_frame[2] = (seq >> 16) & 0xFF;
  out_frame[3] = (seq >> 8)  & 0xFF;
  out_frame[4] = (seq)       & 0xFF;
  out_frame[5] = MSG_TYPE_ACK;
  uint32_t rnd = esp_random();
  out_frame[6] = (rnd >> 16) & 0xFF;
  out_frame[7] = (rnd >> 8)  & 0xFF;
  out_frame[8] = (rnd)       & 0xFF;

  AckPayload ack;
  ack.node_id = target_node_id;
  ack.seq = seq;
  ack.status = status;

  uint8_t iv[12] = {0};
  memcpy(iv, out_frame, 9);

  GCM<AES128> local_gcm;
  local_gcm.setKey(AES_KEY, 16);
  local_gcm.setIV(iv, 12);
  local_gcm.addAuthData(out_frame, 9);
  local_gcm.encrypt(out_frame + 9, (uint8_t*)&ack, sizeof(AckPayload));
  local_gcm.computeTag(out_frame + 9 + sizeof(AckPayload), TAG_SIZE);

  *out_len = 9 + sizeof(AckPayload) + TAG_SIZE;
  return true;
}

extern uint32_t global_ack_sent_total;

void sendAck(uint8_t target_node_id, uint32_t seq, uint8_t status) {
  uint8_t ack_frame[64];
  uint8_t ack_len = 0;
  if (gcm_encrypt_ack(target_node_id, seq, status, ack_frame, &ack_len)) {
    int state = radio->transmit(ack_frame, ack_len);
    if (state == RADIOLIB_ERR_NONE) {
      global_ack_sent_total++;
      addGwLog("Node %d | ACK SENT seq=%lu", target_node_id, seq);
    } else {
      addGwLog("Node %d | ACK TX FAILED code=%d", target_node_id, state);
    }
    radio->startReceive();
  }
}

bool gcm_decrypt(const uint8_t *frame, uint8_t frame_len, uint8_t *payload,
                 uint8_t payload_size) {
  if (frame_len < 9 + TAG_SIZE)
    return false;
  uint8_t payload_len = frame_len - 9 - TAG_SIZE;
  if (payload_len > payload_size)
    return false;

  uint8_t iv[12] = {0};
  memcpy(iv, frame, 9);

  const uint8_t *ciphertext = frame + 9;
  const uint8_t *tag = frame + frame_len - TAG_SIZE;

  GCM<AES128> local_gcm;
  local_gcm.setKey(AES_KEY, 16);
  local_gcm.setIV(iv, 12);
  local_gcm.addAuthData(frame, 9);
  local_gcm.decrypt(payload, ciphertext, payload_len);

  uint8_t computed_tag[TAG_SIZE];
  local_gcm.computeTag(computed_tag, TAG_SIZE);
  return memcmp(computed_tag, tag, TAG_SIZE) == 0;
}

void processLoRaPacket() {
  uint32_t t0 = micros();
  global_radio_reads++;

  uint8_t frame[128];
  int state = radio->readData(frame, sizeof(frame));
  uint32_t t1 = micros();

  int len = radio->getPacketLength();
  uint32_t t2 = micros();

  float rssi = radio->getRSSI();
  uint32_t t3 = micros();

  float snr = radio->getSNR();
  uint32_t t4 = micros();

  if (state != RADIOLIB_ERR_NONE) {
    global_radio_errors++;
    if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      global_radio_err_crc++;
    } else if (state == RADIOLIB_ERR_LORA_HEADER_DAMAGED) {
      global_radio_err_header++;
    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
      global_radio_err_timeout++;
    } else {
      global_radio_err_other++;
    }
    addGwLog("Radio RX error (code: %d | RSSI: %.0fdBm | SNR: %.1fdB)", state, rssi, snr);
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  if (len < 9 + TAG_SIZE) {
    global_malformed_packets++;
    addGwLog("Packet too short: %d bytes", len);
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  uint8_t node_id = frame[0];
  if (node_id >= MAX_NODES) {
    global_unknown_nodes++;
    addGwLog("Node %d | UNKNOWN NODE ID", node_id);
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  uint8_t msg_type = frame[5];
  if (msg_type == MSG_TYPE_ACK) {
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  if (msg_type != MSG_TYPE_DATA) {
    global_malformed_packets++;
    addGwLog("Node %d | UNKNOWN MSG TYPE: 0x%02X", node_id, msg_type);
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  uint8_t payload_len = len - 9 - TAG_SIZE;
  uint8_t payload[128] = {0};

  if (payload_len > sizeof(payload)) {
    global_malformed_packets++;
    addGwLog("Node %d | Packet too large (%d bytes)", node_id, len);
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  NodeData &n = nodes[node_id];
  if (!gcm_decrypt(frame, len, payload, sizeof(payload))) {
    n.auth_failures++;
    addGwLog("Node %d | AUTH FAILED (AES key mismatch)", node_id);
    radio->startReceive();
    global_total_processing_us = micros() - t0;
    return;
  }

  global_valid_packets++;

  uint32_t seq = ((uint32_t)frame[1] << 24) | ((uint32_t)frame[2] << 16) |
                 ((uint32_t)frame[3] << 8) | ((uint32_t)frame[4]);
  uint32_t random_id = ((uint32_t)frame[6] << 16) | ((uint32_t)frame[7] << 8) | ((uint32_t)frame[8]);

  // Session-based Reboot, Duplicate, and Packet Loss Decision Tree
  if (n.seen) {
    if (random_id != n.last_random_id) {
      // New boot session detected (TRNG session_id changed)
      addGwLog("Node %d | NEW SESSION / REBOOT (seq %lu -> %lu, session %06lX -> %06lX)",
               node_id, n.seq, seq, n.last_random_id, random_id);
      n.reboots++;
      n.seq = seq;
      n.last_random_id = random_id;
    } else if (seq == n.seq) {
      // Same session + same sequence number = Retry / Duplicate
      n.duplicate_packets++;
      addGwLog("Node %d | DUPLICATE seq=%lu (resending ACK)", node_id, seq);
      sendAck(node_id, seq, 0);
      global_total_processing_us = micros() - t0;
      return;
    } else if (seq > n.seq + 1) {
      // Same session + sequence gap = Packet loss
      uint32_t lost = seq - (n.seq + 1);
      n.packets_lost += lost;
      Serial.printf("Node %d | PACKET LOSS DETECTED: %lu packet(s) lost (seq %lu -> %lu)\n", node_id, lost, n.seq, seq);
      addGwLog("Node %d | PACKET LOSS: %lu packet(s) lost (seq %lu -> %lu)", node_id, lost, n.seq, seq);
    }
  }

  uint8_t current_reset_reason = 0;
  uint8_t current_error_code = 0;
  bool is_sensor_payload = (payload_len >= SENSOR_PAYLOAD_HDR_SIZE);
  SensorPayload sp;
  memset(&sp, 0, sizeof(sp));

  if (is_sensor_payload) {
    memcpy(&sp, payload, min((size_t)payload_len, sizeof(sp)));
    current_reset_reason = sp.reset_reason;
    current_error_code = sp.error_code;
  }

  n.seen = true;
  n.seq = seq;
  n.last_random_id = random_id;
  n.rssi = rssi;
  n.snr = snr;
  n.last_seen_ms = millis();
  n.packets_count++;
  n.last_reset_reason = current_reset_reason;
  n.last_error_code = current_error_code;

  addGwLog("Node %d (%s) | RX OK seq=%lu (RSSI: %.1fdBm, SNR: %.1fdB)", node_id, n.name[0] ? n.name : "Node", seq, n.rssi, n.snr);

  if (is_sensor_payload) {
    n.tx_interval = sp.tx_interval;
    memset(n.name, 0, sizeof(n.name));
    strncpy(n.name, sp.name, sizeof(n.name) - 1);

    n.readings_count = min((int)sp.count, 16);
    memcpy(n.readings, sp.readings, n.readings_count * sizeof(SensorReading));
  } else {
    n.readings_count = 0;
    Serial.printf("Node %d | Payload ignored due to unexpected size: received=%d bytes\n", node_id, payload_len);
  }

  last_active_node_id = node_id;

  uint32_t t5 = micros();
  sendAck(node_id, seq, 0);
  uint32_t t6 = micros();

  global_rx_processing_us = t6 - t5;
  global_total_processing_us = t6 - t0;

  Serial.printf("LoRa RX Breakdown | readData: %lu us | getPacketLength: %lu us | getRSSI: %lu us | getSNR: %lu us | sendAck+RX: %lu us | total: %lu us\n",
                t1 - t0, t2 - t1, t3 - t2, t4 - t3, global_rx_processing_us, global_total_processing_us);
}

void initLoRa() {
  if (oled_initialized) {
    display.println("Init LoRa Radio...");
    display.display();
  }

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  int state = RADIOLIB_ERR_UNKNOWN;

  if (gw_lora_chip == 2) {
    Serial.println("Initializing Gateway Radio: SX1262...");
    Module* mod = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
    SX1262* radio62 = new SX1262(mod);
    state = radio62->begin(gw_lora_freq, gw_lora_bw, gw_lora_sf, gw_lora_cr, gw_lora_sync,
                           LORA_POWER, LORA_PREAMBLE);
    if (state == RADIOLIB_ERR_NONE) {
      radio62->setDio2AsRfSwitch(true);
      radio62->setRegulatorDCDC();
      Serial.println("Gateway SX1262 initialized successfully!");
      radio = radio62;
    }
  } else {
    Serial.println("Initializing Gateway Radio: SX1278...");
    Module* mod = new Module(LORA_CS, LORA_DIO0, LORA_RST, -1);
    SX1278* radio78 = new SX1278(mod);
    state = radio78->begin(gw_lora_freq, gw_lora_bw, gw_lora_sf, gw_lora_cr, gw_lora_sync,
                           LORA_POWER, LORA_PREAMBLE);
    if (state == RADIOLIB_ERR_NONE) {
      radio = radio78;
    }
  }

  if (state != RADIOLIB_ERR_NONE || radio == nullptr) {
    Serial.printf("LoRa Radio Init failed: %d\n", state);
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
    display.display();
  }

  loraQueue = xQueueCreate(10, sizeof(uint8_t));
  if (loraQueue != NULL) {
    xTaskCreate(loraTask, "LoRaTask", 4096, NULL, 6, NULL);
  }

  radio->setPacketReceivedAction(onReceive);
  radio->startReceive();
}
