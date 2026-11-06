// Oscilloscope demo for the 1.28" round GC9A01 TFT (240x240, SPI).
// Green phosphor look: graticule with divisions and a running sine wave with a soft glow.
// Library and board are chosen in config.h, the pins live in boards.h.
//
// Copyright (c) 2026 Stefan Fambach (SteFam), https://www.fambach.net
// License: PolyForm Noncommercial 1.0.0, https://polyformproject.org/licenses/noncommercial/1.0.0/

#include <math.h>
#include "config.h"
#include "boards.h"

#if DISPLAY_LIBRARY == LIBRARY_ADAFRUIT
  #include "display_adafruit.h"
#elif DISPLAY_LIBRARY == LIBRARY_LOVYANGFX
  #include "display_lovyangfx.h"
#else
  #error "Unknown DISPLAY_LIBRARY in config.h"
#endif

constexpr int SIZE   = 240;
constexpr int CENTER = SIZE / 2;
constexpr int RADIUS = CENTER - 1;
constexpr int BAND_H = 24;                 // full screen in 10 bands of 11.5 KB

constexpr int DIVISION       = 30;         // pixels per graticule division
constexpr int SUBDIVISIONS   = 5;          // tick marks per division on the axes
constexpr float AMPLITUDE    = 2.0f;       // in divisions
constexpr float WAVELENGTH   = 4.0f;       // in divisions
constexpr float SPEED        = 1.5f;       // wavelengths per second
constexpr float BREATH_SPEED = 0.15f;      // slow amplitude change, cycles per second

static RoundDisplay display(SIZE, BAND_H);
static int16_t waveY[SIZE];

static int chordHalf(int offset) {
  return static_cast<int>(sqrtf(static_cast<float>(RADIUS * RADIUS - offset * offset)));
}

static void calculateWave(float seconds) {
  const float amplitude = AMPLITUDE * DIVISION * (0.75f + 0.25f * sinf(2.0f * PI * BREATH_SPEED * seconds));
  const float phase = 2.0f * PI * SPEED * seconds;
  for (int x = 0; x < SIZE; ++x) {
    const float angle = 2.0f * PI * (x - CENTER) / (WAVELENGTH * DIVISION) - phase;
    waveY[x] = static_cast<int16_t>(lroundf(CENTER - amplitude * sinf(angle)));
  }
}

static void drawGraticule(int bandTop) {
  const uint16_t gridColor = display.color(0, 55, 20);
  const uint16_t axisColor = display.color(0, 110, 40);

  for (int offset = -(RADIUS / DIVISION) * DIVISION; offset <= RADIUS; offset += DIVISION) {
    const int half = chordHalf(offset);
    const uint16_t ink = offset == 0 ? axisColor : gridColor;
    display.drawLine(CENTER + offset, CENTER - half - bandTop, CENTER + offset, CENTER + half - bandTop, ink);
    display.drawLine(CENTER - half, CENTER + offset - bandTop, CENTER + half, CENTER + offset - bandTop, ink);
  }

  const int tickStep = DIVISION / SUBDIVISIONS;
  for (int offset = -RADIUS + tickStep; offset < RADIUS; offset += tickStep) {
    display.drawLine(CENTER + offset, CENTER - 3 - bandTop, CENTER + offset, CENTER + 3 - bandTop, axisColor);
    display.drawLine(CENTER - 3, CENTER + offset - bandTop, CENTER + 3, CENTER + offset - bandTop, axisColor);
  }
}

static void drawWave(int bandTop) {
  const uint16_t glowColor  = display.color(0, 90, 30);
  const uint16_t traceColor = display.color(120, 255, 140);
  for (int x = 1; x < SIZE; ++x) {
    const int y0 = waveY[x - 1] - bandTop;
    const int y1 = waveY[x] - bandTop;
    if (max(y0, y1) + 2 < 0 || min(y0, y1) - 2 >= BAND_H) {
      continue;
    }
    display.drawLine(x - 1, y0 - 2, x, y1 - 2, glowColor);
    display.drawLine(x - 1, y0 + 2, x, y1 + 2, glowColor);
    display.drawLine(x - 1, y0 - 1, x, y1 - 1, traceColor);
    display.drawLine(x - 1, y0,     x, y1,     traceColor);
    display.drawLine(x - 1, y0 + 1, x, y1 + 1, traceColor);
  }
}

void setup() {
  Serial.begin(115200);

  if (PINS.backlight >= 0) {
    pinMode(PINS.backlight, OUTPUT);
    digitalWrite(PINS.backlight, HIGH);
  }

  if (!display.begin()) {
    Serial.println("Not enough RAM for the drawing band");
    while (true) {
      delay(1000);
    }
  }
}

void loop() {
  calculateWave(millis() / 1000.0f);
  const uint16_t labelColor = display.color(0, 170, 60);

  for (int bandTop = 0; bandTop < SIZE; bandTop += BAND_H) {
    display.clearBand();
    drawGraticule(bandTop);
    drawWave(bandTop);
    display.drawCenteredText("CH1  1V/DIV", RoundDisplay::FONT_COUNT - 1, CENTER, 205 - bandTop, labelColor);
    display.pushBand(0, bandTop);
  }
  delay(1);  // also feeds the ESP8266 watchdog
}
