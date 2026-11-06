#pragma once

// Graphics library
#define LIBRARY_ADAFRUIT   1   // Adafruit GC9A01A + Adafruit GFX: small, easy
#define LIBRARY_LOVYANGFX  2   // LovyanGFX: faster, DMA on ESP32
#define DISPLAY_LIBRARY    LIBRARY_ADAFRUIT

// Board, BOARD_AUTO takes the default board of the chip selected in the Arduino IDE:
// ESP8266 -> BOARD_ESP8266_DEV, ESP32 -> BOARD_ESP32_DEV, ESP32-C3 -> BOARD_ESP32C3_SUPER_MINI
#define BOARD_AUTO                0
#define BOARD_ESP8266_DEV         1   // NodeMCU, Wemos D1 Mini and other ESP8266 boards
#define BOARD_ESP32_DEV           2   // ESP32 DevKit, D1 Mini ESP32
#define BOARD_ESP32C3_SUPER_MINI  3
#define BOARD_LOLIN_C3_MINI       4   // Wemos/LOLIN C3 Mini (D1 Mini form factor)
#define TARGET_BOARD              BOARD_AUTO
