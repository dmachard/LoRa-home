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

void processLoRaPacket() {
  rxFlag = false;
  global_rx_interrupts++;
  uint8_t frame[64];
  int state = radio.readData(frame, sizeof(frame));
  int len = radio.getPacketLength();

  if (state != RADIOLIB_ERR_NONE) {
    global_malformed_packets++;
    radio.startReceive();
    return;
  }

  if (len < HDR_SIZE + TAG_SIZE) {
    global_malformed_packets++;
    Serial.printf("Packet too short: %d bytes\n", len);
    radio.startReceive();
    return;
  }

  uint8_t node_id = frame[0];
  if (node_id >= MAX_NODES) {
    global_unknown_nodes++;
    Serial.printf("Node %d | UNKNOWN NODE\n", node_id);
    radio.startReceive();
    return;
  }

  uint8_t payload_len = len - HDR_SIZE - TAG_SIZE;
  uint8_t payload[64] = {0};

  if (payload_len > sizeof(payload)) {
    global_malformed_packets++;
    Serial.printf("Node %d | Packet too large (%d bytes)\n", node_id, len);
    radio.startReceive();
    return;
  }

  NodeData &n = nodes[node_id];
  if (!gcm_decrypt(frame, len, payload, sizeof(payload))) {
    n.auth_failures++;
    Serial.printf("Node %d | AUTH FAILED\n", node_id);
    radio.startReceive();
    return;
  }

  uint32_t seq = ((uint32_t)frame[1] << 24) | ((uint32_t)frame[2] << 16) |
                 ((uint32_t)frame[3] << 8) | ((uint32_t)frame[4]);
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
    if (seq == 0 && current_reset_reason == 1) { // 1 = ESP_RST_POWERON
      Serial.printf("Node %d | LEGITIMATE REBOOT (POWERON) DETECTED\n", node_id);
      n.reboots++;
      resetWindow(n, seq);
    } else if (seq < n.seq) {
      Serial.printf("Node %d | WARNING: UNEXPECTED REBOOT DETECTED (seq < n.seq)\n", node_id);
      n.reboots++;
      resetWindow(n, seq);
    } else if (seq == n.seq) {
      // duplicate, ignore
      radio.startReceive();
      return;
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
    Serial.printf("Node %d | Payload ignored due to unexpected size: received=%d bytes\n", node_id, payload_len);
  }

  last_active_node_id = node_id;
  updateDisplay();
  radio.startReceive();
}
