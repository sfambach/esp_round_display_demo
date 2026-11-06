#pragma once

#include <stdint.h>
#include "config.h"

#if TARGET_BOARD == BOARD_AUTO
  #undef TARGET_BOARD
  #if defined(ESP8266)
    #define TARGET_BOARD BOARD_ESP8266_DEV
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    #define TARGET_BOARD BOARD_ESP32C3_SUPER_MINI
  #elif defined(CONFIG_IDF_TARGET_ESP32)
    #define TARGET_BOARD BOARD_ESP32_DEV
  #else
    #error "No default board for this chip, set TARGET_BOARD in config.h"
  #endif
#endif

struct BoardPins {
  int sck;
  int mosi;
  int cs;
  int dc;
  int rst;
  int backlight;          // -1: BLK wired to 3V3
  uint32_t spiFrequency;
};

// ESP8266 dev, D1 Mini ESP32 and C3 Mini share the D1 Mini pin positions:
// SCK = D5, MOSI = D7, CS = D8, DC = D3, RST = D0, BLK = 3V3.
#if TARGET_BOARD == BOARD_ESP8266_DEV
  #if !defined(ESP8266)
    #error "BOARD_ESP8266_DEV needs an ESP8266 board selected in the IDE"
  #endif
  // D3/D4/D8 are boot pins: only display inputs may hang there, BLK goes to 3V3.
  constexpr BoardPins PINS { 14, 13, 15, 0, 16, -1, 20000000 };
#elif TARGET_BOARD == BOARD_ESP32_DEV
  #if !defined(CONFIG_IDF_TARGET_ESP32)
    #error "BOARD_ESP32_DEV needs an ESP32 board selected in the IDE"
  #endif
  // WROVER modules use GPIO 16/17 for PSRAM: move DC to another pin there.
  constexpr BoardPins PINS { 18, 23, 5, 17, 26, -1, 40000000 };
#elif TARGET_BOARD == BOARD_LOLIN_C3_MINI
  #if !defined(CONFIG_IDF_TARGET_ESP32C3)
    #error "BOARD_LOLIN_C3_MINI needs an ESP32-C3 board selected in the IDE"
  #endif
  // GPIO 7 also drives the RGB LED, which may flicker while drawing.
  constexpr BoardPins PINS { 1, 4, 5, 7, 2, -1, 40000000 };
#elif TARGET_BOARD == BOARD_ESP32C3_SUPER_MINI
  #if !defined(CONFIG_IDF_TARGET_ESP32C3)
    #error "BOARD_ESP32C3_SUPER_MINI needs an ESP32-C3 board selected in the IDE"
  #endif
  // Default SPI pins of the ESP32-C3; GPIO 2, 8 and 9 are boot pins and stay free.
  constexpr BoardPins PINS { 4, 6, 7, 5, 3, 10, 40000000 };
#else
  #error "Unknown TARGET_BOARD in config.h"
#endif
