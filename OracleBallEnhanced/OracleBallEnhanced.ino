// Oracle ball demo, enhanced: no triangle, the answers slowly fade in from black to white.
// Answers fade in from black to white: Ja -> Nein -> Vielleicht -> dark, endlessly.
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

// Only the text area is redrawn, in horizontal bands: flicker-free like a frame
// buffer, but 2 x 16 KB instead of 115 KB, so it also fits into the ESP8266.
constexpr int AREA_X = 10, AREA_Y = 84, AREA_W = 220, AREA_H = 72;
constexpr int BAND_H = 36;

constexpr int TEXT_X     = AREA_W / 2;   // area coordinates
constexpr int TEXT_Y     = AREA_H / 2;
constexpr int TEXT_MAX_W = 200;

constexpr uint32_t FADE_IN_MS  = 3000;
constexpr uint32_t HOLD_MS     = 2000;
constexpr uint32_t FADE_OUT_MS = 1500;
constexpr uint32_t DARK_MS     = 1000;
constexpr uint32_t CYCLE_MS    = FADE_IN_MS + HOLD_MS + FADE_OUT_MS + DARK_MS;

static RoundDisplay display(AREA_W, BAND_H);
static int answerFont[ANSWER_COUNT];

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

static void drawAnswer(int answer, float level) {
  const uint8_t brightness = static_cast<uint8_t>(255 * level);
  const uint16_t textColor = display.color(brightness, brightness, brightness);

  for (int bandTop = 0; bandTop < AREA_H; bandTop += BAND_H) {
    display.clearBand();
    display.drawCenteredText(ANSWERS[answer], answerFont[answer], TEXT_X, TEXT_Y - bandTop, textColor);
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
  if (phaseTime < FADE_IN_MS) {
    level = smooth(static_cast<float>(phaseTime) / FADE_IN_MS);
  } else if (phaseTime < FADE_IN_MS + HOLD_MS) {
    level = 1.0f;
  } else if (phaseTime < FADE_IN_MS + HOLD_MS + FADE_OUT_MS) {
    level = 1.0f - smooth(static_cast<float>(phaseTime - FADE_IN_MS - HOLD_MS) / FADE_OUT_MS);
  }

  drawAnswer(answer, level);
  delay(10);  // also feeds the ESP8266 watchdog
}
