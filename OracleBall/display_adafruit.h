#pragma once

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "boards.h"

/// GC9A01 output through Adafruit GC9A01A, drawn band by band from a GFXcanvas16.
class RoundDisplay {
public:
  static constexpr int FONT_COUNT = 4;

  RoundDisplay(int bandWidth, int bandHeight)
    : tft(PINS.cs, PINS.dc, PINS.rst), band(bandWidth, bandHeight) {}

  bool begin() {
#if defined(ESP32)
    SPI.begin(PINS.sck, -1, PINS.mosi, PINS.cs);
#endif
    tft.begin(PINS.spiFrequency);
    tft.fillScreen(GC9A01A_BLACK);
    return band.getBuffer() != nullptr;
  }

  uint16_t color(uint8_t red, uint8_t green, uint8_t blue) {
    return tft.color565(red, green, blue);
  }

  int textWidth(const char* text, int fontIndex) {
    int16_t x = 0, y = 0;
    uint16_t width = 0, height = 0;
    band.setFont(FONTS[fontIndex]);
    band.getTextBounds(text, 0, 0, &x, &y, &width, &height);
    return width;
  }

  void clearBand() {
    band.fillScreen(GC9A01A_BLACK);
  }

  void drawCenteredText(const char* text, int fontIndex, int centerX, int centerY, uint16_t ink) {
    int16_t x = 0, y = 0;
    uint16_t width = 0, height = 0;
    band.setFont(FONTS[fontIndex]);
    band.getTextBounds(text, 0, 0, &x, &y, &width, &height);
    band.setTextColor(ink);
    band.setCursor(centerX - static_cast<int>(width) / 2 - x, centerY - static_cast<int>(height) / 2 - y);
    band.print(text);
  }

  void pushBand(int x, int y) {
    tft.drawRGBBitmap(x, y, band.getBuffer(), band.width(), band.height());
  }

private:
  static constexpr const GFXfont* FONTS[FONT_COUNT] = {
    &FreeSansBold24pt7b, &FreeSansBold18pt7b, &FreeSansBold12pt7b, &FreeSansBold9pt7b
  };

  Adafruit_GC9A01A tft;
  GFXcanvas16 band;
};
