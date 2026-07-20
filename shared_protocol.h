#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <Arduino.h>

enum ReadingType {
  TYPE_AHT20_TEMP  = 3,
  TYPE_AHT20_HUM   = 4,
  TYPE_BMP280_TEMP = 5,
  TYPE_BMP280_PRES = 6,
  TYPE_TSL2561_LUX = 7,
  TYPE_SCD40_CO2   = 9,
  TYPE_INA226_VOLT = 10
};

struct SensorReading {
  uint8_t type;       // Measurement type (e.g., TYPE_AHT20_TEMP...)
  int32_t value;      // Raw value
} __attribute__((packed));

struct SensorPayload {
  uint8_t count;               // Number of readings in array (max 10)
  SensorReading readings[10];  // Readings array
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
    case TYPE_AHT20_TEMP:  return {3, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_AHT20_HUM:   return {4, "humidity_percent", "Humidity", "%", 0.01f};
    case TYPE_BMP280_TEMP: return {5, "temperature_celsius", "Temperature", "°C", 0.01f};
    case TYPE_BMP280_PRES: return {6, "pressure_hpa", "Pressure", "hPa", 0.1f};
    case TYPE_TSL2561_LUX: return {7, "light_lux", "Light", "lux", 1.0f};
    case TYPE_SCD40_CO2:   return {9, "co2_ppm", "CO2", "ppm", 1.0f};
    case TYPE_INA226_VOLT: return {10, "bus_voltage_volts", "Voltage", "V", 0.001f};
    default:               return {type, "unknown_raw", "Unk", "", 1.0f};
  }
}

#endif // SHARED_PROTOCOL_H
