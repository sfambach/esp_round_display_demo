#pragma once

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "boards.h"

/// GC9A01 output through Adafruit GC9A01A, drawing straight to the panel.
class RoundDisplay {
public:
  RoundDisplay() : tft(PINS.cs, PINS.dc, PINS.rst) {}

  bool begin() {
#if defined(ESP32)
    SPI.begin(PINS.sck, -1, PINS.mosi, PINS.cs);
#endif
    tft.begin(PINS.spiFrequency);
    tft.fillScreen(GC9A01A_BLACK);
    return true;
  }

  int width() { return tft.width(); }
  int height() { return tft.height(); }

  uint16_t color(uint8_t red, uint8_t green, uint8_t blue) {
    return tft.color565(red, green, blue);
  }

  void fillCircle(int centerX, int centerY, int radius, uint16_t ink) {
    tft.fillCircle(centerX, centerY, radius, ink);
  }

private:
  Adafruit_GC9A01A tft;
};
