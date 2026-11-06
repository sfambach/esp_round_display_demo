#pragma once

#include <LovyanGFX.hpp>
#include "boards.h"

/// LovyanGFX device for the GC9A01 panel, wired as given in boards.h.
class Gc9a01Device : public lgfx::LGFX_Device {
public:
  Gc9a01Device() {
    auto busConfig = bus.config();
#if defined(CONFIG_IDF_TARGET_ESP32)
    busConfig.spi_host    = SPI3_HOST;
#elif defined(ESP32)
    busConfig.spi_host    = SPI2_HOST;
#endif
#if defined(ESP32)
    busConfig.use_lock    = true;
    busConfig.dma_channel = SPI_DMA_CH_AUTO;
#endif
    busConfig.spi_mode   = 0;
    busConfig.freq_write = PINS.spiFrequency;
    busConfig.spi_3wire  = true;
    busConfig.pin_sclk   = PINS.sck;
    busConfig.pin_mosi   = PINS.mosi;
    busConfig.pin_miso   = -1;
    busConfig.pin_dc     = PINS.dc;
    bus.config(busConfig);
    panel.setBus(&bus);

    auto panelConfig = panel.config();
    panelConfig.pin_cs       = PINS.cs;
    panelConfig.pin_rst      = PINS.rst;
    panelConfig.pin_busy     = -1;
    panelConfig.panel_width  = 240;
    panelConfig.panel_height = 240;
    panelConfig.readable     = false;
    panelConfig.invert       = true;   // most GC9A01 modules; set false if colours look inverted
    panelConfig.rgb_order    = false;  // set true if red and blue are swapped
    panel.config(panelConfig);

    setPanel(&panel);
  }

private:
  lgfx::Panel_GC9A01 panel;
  lgfx::Bus_SPI bus;
};

/// GC9A01 output through LovyanGFX, drawn band by band from a sprite.
class RoundDisplay {
public:
  static constexpr int FONT_COUNT = 4;

  RoundDisplay(int bandWidth, int bandHeight)
    : band(&lcd), bandWidth(bandWidth), bandHeight(bandHeight) {}

  bool begin() {
    lcd.init();
    lcd.fillScreen(TFT_BLACK);
    band.setColorDepth(16);
    band.setTextDatum(textdatum_t::middle_center);
    return band.createSprite(bandWidth, bandHeight) != nullptr;
  }

  uint16_t color(uint8_t red, uint8_t green, uint8_t blue) {
    return lcd.color565(red, green, blue);
  }

  int textWidth(const char* text, int fontIndex) {
    band.setFont(FONTS[fontIndex]);
    return band.textWidth(text);
  }

  void clearBand() {
    band.fillScreen(TFT_BLACK);
  }

  void drawLine(int x0, int y0, int x1, int y1, uint16_t ink) {
    band.drawLine(x0, y0, x1, y1, ink);
  }

  void drawCircle(int centerX, int centerY, int radius, uint16_t ink) {
    band.drawCircle(centerX, centerY, radius, ink);
  }

  void fillCircle(int centerX, int centerY, int radius, uint16_t ink) {
    band.fillCircle(centerX, centerY, radius, ink);
  }

  void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t fill) {
    band.fillTriangle(x0, y0, x1, y1, x2, y2, fill);
  }

  void drawCenteredText(const char* text, int fontIndex, int centerX, int centerY, uint16_t ink) {
    band.setFont(FONTS[fontIndex]);
    band.setTextColor(ink);
    band.drawString(text, centerX, centerY);
  }

  void pushBand(int x, int y) {
    band.pushSprite(x, y);
  }

private:
  static constexpr const lgfx::IFont* FONTS[FONT_COUNT] = {
    &fonts::FreeSansBold24pt7b, &fonts::FreeSansBold18pt7b,
    &fonts::FreeSansBold12pt7b, &fonts::FreeSansBold9pt7b
  };

  Gc9a01Device lcd;
  LGFX_Sprite band;
  int bandWidth;
  int bandHeight;
};
