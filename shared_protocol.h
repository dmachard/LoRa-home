#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <Arduino.h>

enum ReadingType {
  TYPE_DHT22_TEMP  = 1,
  TYPE_DHT22_HUM   = 2,
  TYPE_AHT20_TEMP  = 3,
  TYPE_AHT20_HUM   = 4,
  TYPE_BMP280_TEMP = 5,
  TYPE_BMP280_PRES = 6,
  TYPE_BH1750_LUX  = 7,
  TYPE_BATTERY     = 8
};

struct SensorReading {
  uint8_t type;       // Measurement type (e.g., TYPE_AHT20_TEMP...)
  int32_t value;      // Raw value
} __attribute__((packed));

struct SensorPayload {
  uint8_t count;               // Number of readings in array (max 6)
  SensorReading readings[6];   // Readings array
  uint8_t reset_reason;
  uint8_t error_code;
  uint16_t tx_interval;
  char name[8];
} __attribute__((packed));

struct ReadingTypeDefinition {
  uint8_t type;
  const char* name;
  const char* label;
  const char* unit;
  float scale;
};

inline ReadingTypeDefinition getReadingDefinition(uint8_t type) {
  switch(type) {
    case TYPE_DHT22_TEMP:  return {1, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_DHT22_HUM:   return {2, "humidity_percent", "Humidity", "%", 0.01f};
    case TYPE_AHT20_TEMP:  return {3, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_AHT20_HUM:   return {4, "humidity_percent", "Humidity", "%", 0.01f};
    case TYPE_BMP280_TEMP: return {5, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_BMP280_PRES: return {6, "pressure_hpa", "Pressure", "hPa", 0.1f};
    case TYPE_BH1750_LUX:  return {7, "light_lux", "Light", "lux", 1.0f};
    case TYPE_BATTERY:     return {8, "battery_millivolts", "Battery", "mV", 1.0f};
    default:               return {type, "unknown_raw", "Unk", "", 1.0f};
  }
}

#endif // SHARED_PROTOCOL_H
