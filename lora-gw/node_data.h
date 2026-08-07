#ifndef NODE_DATA_H
#define NODE_DATA_H

#include "shared_protocol.h"

#define MAX_NODES 16

struct NodeData {
  char name[9]; // Local name with trailing \0
  uint32_t seq;
  uint32_t last_random_id;
  uint32_t last_seen_ms;
  uint32_t packets_count;
  uint32_t packets_lost;
  uint32_t duplicate_packets;
  uint32_t auth_failures;
  uint32_t reboots;
  float rssi;
  float snr;
  uint8_t last_reset_reason;
  uint8_t last_error_code;
  uint16_t tx_interval;
  bool seen;
  
  uint8_t readings_count;
  SensorReading readings[10];
};

struct GwLogEntry {
  uint32_t timestamp_ms;
  char message[64];
};

#endif // NODE_DATA_H
