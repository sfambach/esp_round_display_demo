// Oracle ball demo for the 1.28" round GC9A01 TFT (240x240, SPI): the answer rises on a blue triangle, like in a fortune-telling ball.
// Answers fade in on a black screen: Ja -> Nein -> Vielleicht -> dark, endlessly.
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

const char* const ANSWERS[] = { "Ja", "Nein", "Vielleicht" };
constexpr int ANSWER_COUNT = sizeof(ANSWERS) / sizeof(ANSWERS[0]);

// The area around the triangle is rendered in horizontal bands: flicker-free like a
// full-frame buffer, but 14 KB instead of 72 KB, so it also fits into the ESP8266.
constexpr int AREA_X = 20, AREA_Y = 45, AREA_W = 200, AREA_H = 180;
constexpr int BAND_H = 36;

constexpr int TRI_TOP    = 10;    // area coordinates
constexpr int TRI_BOTTOM = 160;
constexpr int TRI_HALF_W = 100;
constexpr int TRI_CENTER = AREA_W / 2;
constexpr int TEXT_Y     = 50;
constexpr int TEXT_MAX_W = TRI_HALF_W * 2 * (TRI_BOTTOM - TEXT_Y) / (TRI_BOTTOM - TRI_TOP) * 85 / 100;
constexpr int MAX_LIFT   = 16;    // the die rises a little while it appears

constexpr uint32_t FADE_IN_MS  = 1500;
constexpr uint32_t HOLD_MS     = 2500;
constexpr uint32_t FADE_OUT_MS = 1000;
constexpr uint32_t DARK_MS     = 1500;
constexpr uint32_t CYCLE_MS    = FADE_IN_MS + HOLD_MS + FADE_OUT_MS + DARK_MS;

static RoundDisplay display(AREA_W, BAND_H);
static int answerFont[ANSWER_COUNT];

static uint8_t dim(uint8_t channel, float level) {
  return static_cast<uint8_t>(channel * level);
}

static float smooth(float t) {
  return t * t * (3.0f - 2.0f * t);
}

static int largestFittingFont(const char* text) {
  for (int font = 0; font < RoundDisplay::FONT_COUNT - 1; ++font) {
    if (display.textWidth(text, font) <= TEXT_MAX_W) {
      return font;
    }
  }
  return RoundDisplay::FONT_COUNT - 1;
}

static void drawAnswer(int answer, float level, int lift) {
  const uint16_t dieColor  = display.color(dim(20, level),  dim(40, level),  dim(150, level));
  const uint16_t textColor = display.color(dim(210, level), dim(225, level), dim(255, level));

  for (int bandTop = 0; bandTop < AREA_H; bandTop += BAND_H) {
    const int y = lift - bandTop;
    display.clearBand();
    display.fillTriangle(TRI_CENTER - TRI_HALF_W, TRI_TOP + y,
                         TRI_CENTER + TRI_HALF_W, TRI_TOP + y,
                         TRI_CENTER,              TRI_BOTTOM + y, dieColor);
    display.drawCenteredText(ANSWERS[answer], answerFont[answer], TRI_CENTER, TEXT_Y + y, textColor);
    display.pushBand(AREA_X, AREA_Y + bandTop);
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

  for (int answer = 0; answer < ANSWER_COUNT; ++answer) {
    answerFont[answer] = largestFittingFont(ANSWERS[answer]);
  }
}

void loop() {
  static int lastAnswer = -1;

  const uint32_t now       = millis();
  const int      answer    = (now / CYCLE_MS) % ANSWER_COUNT;
  const uint32_t phaseTime = now % CYCLE_MS;

  if (answer != lastAnswer) {
    lastAnswer = answer;
    Serial.println(ANSWERS[answer]);
  }

  float level = 0.0f;
  int lift = 0;
  if (phaseTime < FADE_IN_MS) {
    const float progress = smooth(static_cast<float>(phaseTime) / FADE_IN_MS);
    level = progress;
    lift  = static_cast<int>(MAX_LIFT * (1.0f - progress));
  } else if (phaseTime < FADE_IN_MS + HOLD_MS) {
    level = 1.0f;
  } else if (phaseTime < FADE_IN_MS + HOLD_MS + FADE_OUT_MS) {
    level = 1.0f - smooth(static_cast<float>(phaseTime - FADE_IN_MS - HOLD_MS) / FADE_OUT_MS);
  }

  drawAnswer(answer, level, lift);
  delay(10);  // also feeds the ESP8266 watchdog
}
