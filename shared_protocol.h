#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <Arduino.h>

#define TAG_SIZE 8
#define HDR_SIZE 9 // Exact 9-byte header: node_id (1B) + seq (4B) + msg_type (1B) + entropy (3B)

enum MessageType {
  MSG_TYPE_DATA = 0x01,
  MSG_TYPE_ACK  = 0x02
};

struct AckPayload {
  uint8_t  node_id;     // Destination Node ID
  uint32_t seq;         // Confirmed sequence number
  uint8_t  status;      // Status (0 = OK)
} __attribute__((packed));

enum ReadingType {
  TYPE_AHT20_TEMP     = 3,
  TYPE_AHT20_HUM      = 4,
  TYPE_BMP280_TEMP    = 5,
  TYPE_BMP280_PRES    = 6,
  TYPE_TSL2561_LUX    = 7,
  TYPE_SCD40_CO2      = 9,
  TYPE_INA226_VOLT    = 10,
  TYPE_INA226_CURR    = 11,
  TYPE_LORA_TX_TOTAL  = 13,
  TYPE_LORA_TX_SUCCESS= 14,
  TYPE_LORA_TX_FAILED = 15,
  TYPE_LORA_TX_RETRIES= 16,
  TYPE_LORA_TX_POWER  = 17
};

struct SensorReading {
  uint8_t type;       // Measurement type (e.g., TYPE_AHT20_TEMP...)
  int32_t value;      // Raw value
} __attribute__((packed));

struct SensorPayload {
  uint8_t count;               // Number of readings in array (max 16)
  uint8_t reset_reason;        // Node reset reason
  uint8_t error_code;          // Node error code
  uint16_t tx_interval;        // Transmission interval (seconds)
  char name[8];                // Node name string
  SensorReading readings[16];  // Readings array (AT THE END for dynamic length transmission)
} __attribute__((packed));

#define SENSOR_PAYLOAD_HDR_SIZE 13

struct ReadingTypeDefinition {
  uint8_t type;
  const char* name;
  const char* label;
  const char* unit;
  float scale;
};

inline ReadingTypeDefinition getReadingDefinition(uint8_t type) {
  switch(type) {
    case TYPE_AHT20_TEMP:     return {3, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_AHT20_HUM:      return {4, "humidity_percent", "Humidity", "%", 0.01f};
    case TYPE_BMP280_TEMP:    return {5, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_BMP280_PRES:    return {6, "pressure_hpa", "Pressure", "hPa", 0.1f};
    case TYPE_TSL2561_LUX:    return {7, "light_lux", "Light", "lux", 1.0f};
    case TYPE_SCD40_CO2:      return {9, "co2_ppm", "CO2", "ppm", 1.0f};
    case TYPE_INA226_VOLT:    return {10, "bus_voltage_volts", "Voltage", "V", 0.001f};
    case TYPE_INA226_CURR:    return {11, "bus_current_ma", "Current", "mA", 0.1f};
    case TYPE_LORA_TX_TOTAL:  return {13, "lora_tx_total", "Tx Total", "msg", 1.0f};
    case TYPE_LORA_TX_SUCCESS:return {14, "lora_tx_success", "Tx Success", "msg", 1.0f};
    case TYPE_LORA_TX_FAILED: return {15, "lora_tx_failed", "Tx Failed", "msg", 1.0f};
    case TYPE_LORA_TX_RETRIES:return {16, "lora_tx_retries", "Tx Retries", "msg", 1.0f};
    case TYPE_LORA_TX_POWER:  return {17, "lora_tx_power_dbm", "Tx Power", "dBm", 1.0f};
    default:                  return {type, "unknown_raw", "Unk", "", 1.0f};
  }
}

#endif // SHARED_PROTOCOL_H
