// Radar demo for the 1.28" round GC9A01 TFT (240x240, SPI).
// A sweep with a fading trail turns over range rings; now and then a random hostile
// contact appears, lights up when the sweep passes and fades until the next pass.
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
constexpr int RADIUS = CENTER - 2;
constexpr int BAND_H = 24;                    // full screen in 10 bands of 11.5 KB

constexpr int RING_COUNT          = 4;
constexpr float RANGE_KM          = 40.0f;    // range of the outer ring
constexpr float SWEEP_DEG_PER_SEC = 90.0f;    // one turn in 4 s
constexpr float TRAIL_DEG         = 60.0f;
constexpr int TRAIL_STEPS         = 20;

constexpr int MAX_CONTACTS           = 3;
constexpr uint32_t SPAWN_MIN_MS      = 4000;  // "now and then": a new contact every 4-12 s
constexpr uint32_t SPAWN_MAX_MS      = 12000;
constexpr uint32_t CONTACT_LIFE_MS   = 16000; // about four turns
constexpr float CONTACT_FADE_SECONDS = 3.5f;  // time constant of the afterglow after a hit

struct Contact {
  bool active;
  float angleDeg;         // 0 = north, clockwise
  float distance;         // 0..1 of RADIUS
  float driftDegPerSec;
  uint32_t bornMs;
  uint32_t lastHitMs;
};

static RoundDisplay display(SIZE, BAND_H);
static Contact contacts[MAX_CONTACTS];
static uint32_t nextSpawnMs;
static float sweepDeg;
static float previousSweepDeg;
static uint32_t lastFrameMs;

static float toRadians(float degrees) {
  return degrees * PI / 180.0f;
}

static int pointX(float degrees, float radius) {
  return CENTER + static_cast<int>(lroundf(radius * sinf(toRadians(degrees))));
}

static int pointY(float degrees, float radius) {
  return CENTER - static_cast<int>(lroundf(radius * cosf(toRadians(degrees))));
}

static bool sweptOver(float angleDeg) {
  float from = previousSweepDeg;
  float to = sweepDeg;
  if (to < from) {
    to += 360.0f;
  }
  float angle = angleDeg;
  if (angle < from) {
    angle += 360.0f;
  }
  return angle >= from && angle < to;
}

static void spawnContact(uint32_t now) {
  for (Contact& contact : contacts) {
    if (!contact.active) {
      contact.active         = true;
      contact.angleDeg       = random(0, 3600) / 10.0f;
      contact.distance       = random(25, 95) / 100.0f;
      contact.driftDegPerSec = random(-30, 31) / 10.0f;
      contact.bornMs         = now;
      contact.lastHitMs      = 0;
      Serial.printf("Contact: bearing %03d deg, %d km\n",
                    static_cast<int>(contact.angleDeg), static_cast<int>(contact.distance * RANGE_KM));
      return;
    }
  }
}

static void updateContacts(uint32_t now, float seconds) {
  if (now >= nextSpawnMs) {
    spawnContact(now);
    nextSpawnMs = now + random(SPAWN_MIN_MS, SPAWN_MAX_MS);
  }
  for (Contact& contact : contacts) {
    if (!contact.active) {
      continue;
    }
    contact.angleDeg = fmodf(contact.angleDeg + contact.driftDegPerSec * seconds + 360.0f, 360.0f);
    if (sweptOver(contact.angleDeg)) {
      contact.lastHitMs = now;
    }
    if (now - contact.bornMs > CONTACT_LIFE_MS && now - contact.lastHitMs > 1000) {
      contact.active = false;
    }
  }
}

static float contactBrightness(const Contact& contact, uint32_t now) {
  if (contact.lastHitMs == 0) {
    return 0.0f;
  }
  return expf(-(now - contact.lastHitMs) / 1000.0f / CONTACT_FADE_SECONDS);
}

static void drawBackground(int bandTop) {
  const uint16_t ringColor = display.color(0, 70, 25);
  for (int ring = 1; ring <= RING_COUNT; ++ring) {
    display.drawCircle(CENTER, CENTER - bandTop, RADIUS * ring / RING_COUNT, ringColor);
  }
  display.drawLine(CENTER, CENTER - RADIUS - bandTop, CENTER, CENTER + RADIUS - bandTop, ringColor);
  display.drawLine(CENTER - RADIUS, CENTER - bandTop, CENTER + RADIUS, CENTER - bandTop, ringColor);
  for (int degrees = 0; degrees < 360; degrees += 10) {
    const int inner = degrees % 30 == 0 ? RADIUS - 8 : RADIUS - 4;
    display.drawLine(pointX(degrees, inner), pointY(degrees, inner) - bandTop,
                     pointX(degrees, RADIUS), pointY(degrees, RADIUS) - bandTop, ringColor);
  }
}

static void drawSweep(int bandTop) {
  const float step = TRAIL_DEG / TRAIL_STEPS;
  for (int index = TRAIL_STEPS - 1; index >= 0; --index) {
    const float level = 1.0f - static_cast<float>(index) / TRAIL_STEPS;
    const float to = sweepDeg - index * step;
    const float from = to - step;
    const uint16_t ink = display.color(0, static_cast<uint8_t>(110 * level * level), static_cast<uint8_t>(40 * level * level));
    display.fillTriangle(CENTER, CENTER - bandTop,
                         pointX(from, RADIUS), pointY(from, RADIUS) - bandTop,
                         pointX(to, RADIUS), pointY(to, RADIUS) - bandTop, ink);
  }
  display.drawLine(CENTER, CENTER - bandTop, pointX(sweepDeg, RADIUS), pointY(sweepDeg, RADIUS) - bandTop,
                   display.color(160, 255, 170));
}

static void drawContacts(int bandTop, uint32_t now) {
  for (const Contact& contact : contacts) {
    const float level = contact.active ? contactBrightness(contact, now) : 0.0f;
    if (level < 0.05f) {
      continue;
    }
    const int x = pointX(contact.angleDeg, contact.distance * RADIUS);
    const int y = pointY(contact.angleDeg, contact.distance * RADIUS) - bandTop;
    display.drawCircle(x, y, 7, display.color(static_cast<uint8_t>(120 * level), 0, 0));
    display.fillCircle(x, y, 4, display.color(static_cast<uint8_t>(255 * level), static_cast<uint8_t>(40 * level), static_cast<uint8_t>(30 * level)));
  }
}

static void drawLabel(int bandTop, uint32_t now) {
  const Contact* strongest = nullptr;
  float best = 0.3f;
  for (const Contact& contact : contacts) {
    const float level = contact.active ? contactBrightness(contact, now) : 0.0f;
    if (level > best) {
      best = level;
      strongest = &contact;
    }
  }
  if (strongest == nullptr) {
    return;
  }
  char text[24];
  snprintf(text, sizeof(text), "BRG %03d  %d km", static_cast<int>(strongest->angleDeg),
           static_cast<int>(strongest->distance * RANGE_KM));
  display.drawCenteredText(text, RoundDisplay::FONT_COUNT - 1, CENTER, 200 - bandTop,
                           display.color(static_cast<uint8_t>(255 * best), static_cast<uint8_t>(60 * best), static_cast<uint8_t>(40 * best)));
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());

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

  lastFrameMs = millis();
  nextSpawnMs = lastFrameMs + SPAWN_MIN_MS;
}

void loop() {
  const uint32_t now = millis();
  const float seconds = (now - lastFrameMs) / 1000.0f;
  lastFrameMs = now;

  previousSweepDeg = sweepDeg;
  sweepDeg = fmodf(sweepDeg + SWEEP_DEG_PER_SEC * seconds, 360.0f);
  updateContacts(now, seconds);

  for (int bandTop = 0; bandTop < SIZE; bandTop += BAND_H) {
    display.clearBand();
    drawSweep(bandTop);
    drawBackground(bandTop);
    drawContacts(bandTop, now);
    drawLabel(bandTop, now);
    display.pushBand(0, bandTop);
  }
  delay(1);  // also feeds the ESP8266 watchdog
}
