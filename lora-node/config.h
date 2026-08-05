#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// SYSTEM & WATCHDOG CONFIGURATION
// ==========================================
#define WDT_TIMEOUT_S 30

// ==========================================
// HARDWARE BUTTON & LED CONFIGURATION
// ==========================================
#define BUTTON_PIN 9         // BOOT button for BLE Mode
#define EXT_BUTTON_PIN -1    // External RTC Wakeup button (-1 if disabled)
#define LED_PIN -1           // Status LED (-1 if disabled to avoid pin conflict)

// ==========================================
// I2C BUS CONFIGURATION
// ==========================================
#define I2C_SDA 3
#define I2C_SCL 4

// ==========================================
// LORA PHYSICAL TRANSCEIVER CHIP & PIN CONFIG
// 1 = SX1278, 2 = SX1262
// ==========================================
#ifndef LORA_HARDWARE_CHIP
#define LORA_HARDWARE_CHIP 2
#endif

#define SPI_SCK 6
#define SPI_MISO 2
#define SPI_MOSI 7
#define LORA_CS 10
#define LORA_RST 0

// SX1262 Specific Pins
#define LORA_DIO1 8
#define LORA_BUSY 5

// SX1278 Specific Pins
#define LORA_DIO0 1

#endif // CONFIG_H
