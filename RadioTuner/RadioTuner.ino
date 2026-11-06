// Radio tuner demo for the 1.28" round GC9A01 TFT (240x240, SPI).
// A tuning marker glides along an FM scale (87.5-108 MHz) from station to station,
// the frequency counts along, and the station name fades in once it is tuned.
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

struct Station {
  const char* name;
  int frequencyTenths;   // 1059 = 105.9 MHz
  uint8_t red, green, blue;
};

// Example frequencies for the Frankfurt area (Radio Bob unverified); change them to your local ones.
const Station STATIONS[] = {
  { "FFH",       1059, 255, 150,  30 },
  { "Radio Bob", 1014, 230,  40,  40 },
  { "hr3",        893,  40, 200, 255 },
  { "hr1",        944,  80, 220, 100 },
};
constexpr int STATION_COUNT = sizeof(STATIONS) / sizeof(STATIONS[0]);

constexpr int BAND_MIN_TENTHS = 875;
constexpr int BAND_MAX_TENTHS = 1080;

constexpr int CENTER = 120;
constexpr float SCALE_START_DEG = -135.0f;   // 0 deg = top, clockwise
constexpr float SCALE_SWEEP_DEG = 270.0f;
constexpr int TICK_OUTER    = 117;
constexpr int TICK_MINOR    = 113;           // every 0.5 MHz
constexpr int TICK_MEDIUM   = 110;           // every 1 MHz
constexpr int TICK_MAJOR    = 104;           // every 5 MHz
constexpr int MARKER_TRACK  = 97;
constexpr int MARKER_RADIUS = 5;
constexpr int STATION_DOT   = 2;

// Centre text area, inside the marker track, rendered in bands.
constexpr int AREA_X = 45, AREA_Y = 72, AREA_W = 150, AREA_H = 96;
constexpr int BAND_H = 24;
constexpr int NAME_Y      = 28;   // area coordinates
constexpr int FREQUENCY_Y = 70;
constexpr int TEXT_MAX_W  = 140;

constexpr uint32_t TUNE_MS     = 1800;
constexpr uint32_t FADE_IN_MS  = 800;
constexpr uint32_t HOLD_MS     = 3000;
constexpr uint32_t FADE_OUT_MS = 500;

static RoundDisplay display(AREA_W, BAND_H);
static int nameFont[STATION_COUNT];
static int frequencyFont;

static float smooth(float t) {
  return t * t * (3.0f - 2.0f * t);
}

static float angleOf(float frequencyTenths) {
  const float position = (frequencyTenths - BAND_MIN_TENTHS) / (BAND_MAX_TENTHS - BAND_MIN_TENTHS);
  return (SCALE_START_DEG + position * SCALE_SWEEP_DEG) * static_cast<float>(M_PI) / 180.0f;
}

static int pointX(float angle, int radius) {
  return CENTER + static_cast<int>(lroundf(radius * sinf(angle)));
}

static int pointY(float angle, int radius) {
  return CENTER - static_cast<int>(lroundf(radius * cosf(angle)));
}

static int largestFittingFont(const char* text) {
  for (int font = 0; font < RoundDisplay::FONT_COUNT - 1; ++font) {
    if (display.textWidth(text, font) <= TEXT_MAX_W) {
      return font;
    }
  }
  return RoundDisplay::FONT_COUNT - 1;
}

static void drawScale() {
  const uint16_t minorColor = display.color(90, 90, 90);
  const uint16_t majorColor = display.color(230, 230, 230);
  for (int tenths = BAND_MIN_TENTHS; tenths <= BAND_MAX_TENTHS; tenths += 5) {
    const bool major  = tenths % 50 == 0;
    const bool medium = tenths % 10 == 0;
    const int inner = major ? TICK_MAJOR : (medium ? TICK_MEDIUM : TICK_MINOR);
    const float angle = angleOf(tenths);
    display.drawLineOnScreen(pointX(angle, inner), pointY(angle, inner),
                             pointX(angle, TICK_OUTER), pointY(angle, TICK_OUTER),
                             major ? majorColor : minorColor);
  }
}

static void drawStationDots() {
  for (const Station& station : STATIONS) {
    const float angle = angleOf(station.frequencyTenths);
    display.fillCircleOnScreen(pointX(angle, MARKER_TRACK), pointY(angle, MARKER_TRACK), STATION_DOT,
                               display.color(station.red / 2, station.green / 2, station.blue / 2));
  }
}

static void moveMarker(float fromTenths, float toTenths, uint16_t markerColor) {
  const float from = angleOf(fromTenths);
  display.fillCircleOnScreen(pointX(from, MARKER_TRACK), pointY(from, MARKER_TRACK), MARKER_RADIUS,
                             display.color(0, 0, 0));
  drawStationDots();
  const float to = angleOf(toTenths);
  display.fillCircleOnScreen(pointX(to, MARKER_TRACK), pointY(to, MARKER_TRACK), MARKER_RADIUS, markerColor);
}

static void formatFrequency(int tenths, char* text, size_t size) {
  snprintf(text, size, "%d,%d MHz", tenths / 10, tenths % 10);
}

static void drawCentre(const Station& station, int stationIndex, int frequencyTenths, float nameLevel) {
  char frequencyText[16];
  formatFrequency(frequencyTenths, frequencyText, sizeof(frequencyText));
  const uint16_t nameColor = display.color(static_cast<uint8_t>(station.red * nameLevel),
                                           static_cast<uint8_t>(station.green * nameLevel),
                                           static_cast<uint8_t>(station.blue * nameLevel));
  const uint16_t frequencyColor = display.color(255, 255, 255);

  for (int bandTop = 0; bandTop < AREA_H; bandTop += BAND_H) {
    display.clearBand();
    display.drawCenteredText(station.name, nameFont[stationIndex], AREA_W / 2, NAME_Y - bandTop, nameColor);
    display.drawCenteredText(frequencyText, frequencyFont, AREA_W / 2, FREQUENCY_Y - bandTop, frequencyColor);
    display.pushBand(AREA_X, AREA_Y + bandTop);
  }
}

static void tune(int fromIndex, int toIndex) {
  const Station& target = STATIONS[toIndex];
  const uint16_t markerColor = display.color(target.red, target.green, target.blue);
  const float fromTenths = STATIONS[fromIndex].frequencyTenths;
  const float toTenths   = target.frequencyTenths;

  float markerTenths = fromTenths;
  const uint32_t start = millis();
  for (uint32_t elapsed = 0; elapsed < TUNE_MS; elapsed = millis() - start) {
    const float progress = smooth(static_cast<float>(elapsed) / TUNE_MS);
    const float tenths = fromTenths + (toTenths - fromTenths) * progress;
    moveMarker(markerTenths, tenths, markerColor);
    markerTenths = tenths;
    drawCentre(target, toIndex, static_cast<int>(lroundf(tenths)), 0.0f);
    delay(5);
  }
  moveMarker(markerTenths, toTenths, markerColor);
}

static void showName(int index, uint32_t durationMs, bool fadeIn) {
  const uint32_t start = millis();
  for (uint32_t elapsed = 0; elapsed < durationMs; elapsed = millis() - start) {
    const float progress = smooth(static_cast<float>(elapsed) / durationMs);
    drawCentre(STATIONS[index], index, STATIONS[index].frequencyTenths, fadeIn ? progress : 1.0f - progress);
    delay(5);
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

  for (int index = 0; index < STATION_COUNT; ++index) {
    nameFont[index] = largestFittingFont(STATIONS[index].name);
  }
  frequencyFont = largestFittingFont("108,0 MHz");

  drawScale();
  drawStationDots();
}

void loop() {
  static int current = STATION_COUNT - 1;
  const int next = (current + 1) % STATION_COUNT;

  Serial.println(STATIONS[next].name);
  tune(current, next);
  showName(next, FADE_IN_MS, true);
  drawCentre(STATIONS[next], next, STATIONS[next].frequencyTenths, 1.0f);
  delay(HOLD_MS);
  showName(next, FADE_OUT_MS, false);
  current = next;
}
