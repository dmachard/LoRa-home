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
  uint8_t type;       // Type de mesure (ex: TYPE_AHT20_TEMP...)
  int32_t value;      // Valeur brute
} __attribute__((packed));

struct SensorPayload {
  uint8_t count;               // Nombre de mesures dans le tableau (max 6)
  SensorReading readings[6];   // Tableau des mesures
  uint8_t reset_reason;
  uint8_t error_code;
  uint16_t tx_interval;
  char name[8];
} __attribute__((packed));

#endif // SHARED_PROTOCOL_H
