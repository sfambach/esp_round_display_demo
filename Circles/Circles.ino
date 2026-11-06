// Circles demo for the 1.28" round GC9A01 TFT (240x240, SPI).
// Rings appear from the edge inwards until the centre is reached, then they are
// erased again from the centre outwards. Ring positions and colours are calculated.
// Library and board are chosen in config.h, the pins live in boards.h.
//
// Copyright (c) 2026 Stefan Fambach (SteFam), https://www.fambach.net
// License: PolyForm Noncommercial 1.0.0, https://polyformproject.org/licenses/noncommercial/1.0.0/

#include "config.h"
#include "boards.h"

#if DISPLAY_LIBRARY == LIBRARY_ADAFRUIT
  #include "display_adafruit.h"
#elif DISPLAY_LIBRARY == LIBRARY_LOVYANGFX
  #include "display_lovyangfx.h"
#else
  #error "Unknown DISPLAY_LIBRARY in config.h"
#endif

constexpr int RING_COUNT        = 12;     // rings from the edge to the centre
constexpr int RING_FILL_PERCENT = 50;    // ring thickness in percent of the ring spacing
constexpr uint32_t DRAW_MS     = 3000;   // time for all rings to appear
constexpr uint32_t ERASE_MS    = 3000;   // time for all rings to disappear
constexpr uint32_t PAUSE_MS    = 500;    // pause when full and when empty

static RoundDisplay display;

static int centerX;
static int centerY;
static int outerRadius;
static int ringSpacing;
static int ringThickness;

// Hue 0..359 to a fully saturated, full brightness RGB565 colour.
static uint16_t hueColor(int hue) {
  const int sector = hue / 60;
  const uint8_t rising  = static_cast<uint8_t>((hue % 60) * 255 / 60);
  const uint8_t falling = 255 - rising;
  switch (sector) {
    case 0:  return display.color(255, rising, 0);
    case 1:  return display.color(falling, 255, 0);
    case 2:  return display.color(0, 255, rising);
    case 3:  return display.color(0, falling, 255);
    case 4:  return display.color(rising, 0, 255);
    default: return display.color(255, 0, falling);
  }
}

static int ringRadius(int ring) {
  return outerRadius - ring * ringSpacing;
}

// Drawn from the edge inwards, so filling the inside black never hits a ring yet to come.
static void drawRing(int ring, uint16_t ink) {
  const int radius = ringRadius(ring);
  display.fillCircle(centerX, centerY, radius, ink);
  display.fillCircle(centerX, centerY, radius - ringThickness, display.color(0, 0, 0));
}

// Erased from the centre outwards, so everything inside the ring is already black.
static void eraseRing(int ring) {
  display.fillCircle(centerX, centerY, ringRadius(ring), display.color(0, 0, 0));
}

void setup() {
  Serial.begin(115200);

  if (PINS.backlight >= 0) {
    pinMode(PINS.backlight, OUTPUT);
    digitalWrite(PINS.backlight, HIGH);
  }

  display.begin();

  centerX       = display.width() / 2;
  centerY       = display.height() / 2;
  outerRadius   = min(centerX, centerY) - 1;
  ringSpacing   = max(1, outerRadius / RING_COUNT);
  ringThickness = max(1, ringSpacing * RING_FILL_PERCENT / 100);
}

void loop() {
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    drawRing(ring, hueColor(ring * 360 / RING_COUNT));
    delay(DRAW_MS / RING_COUNT);
  }
  delay(PAUSE_MS);

  for (int ring = RING_COUNT - 1; ring >= 0; --ring) {
    eraseRing(ring);
    delay(ERASE_MS / RING_COUNT);
  }
  delay(PAUSE_MS);
}
