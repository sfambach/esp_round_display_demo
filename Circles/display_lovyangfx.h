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

/// GC9A01 output through LovyanGFX, drawing straight to the panel.
class RoundDisplay {
public:
  bool begin() {
    lcd.init();
    lcd.fillScreen(TFT_BLACK);
    return true;
  }

  int width() { return lcd.width(); }
  int height() { return lcd.height(); }

  uint16_t color(uint8_t red, uint8_t green, uint8_t blue) {
    return lcd.color565(red, green, blue);
  }

  void fillCircle(int centerX, int centerY, int radius, uint16_t ink) {
    lcd.fillCircle(centerX, centerY, radius, ink);
  }

private:
  Gc9a01Device lcd;
};
