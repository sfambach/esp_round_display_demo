# ESP Round Display Demo

Demos for the 1.28" round GC9A01 TFT (240×240, SPI, module ZJY_IPS128_8P) on ESP8266, ESP32 and ESP32-C3.

| Sketch | What it shows |
|---|---|
| `Magic8Ball` | "Ja", "Nein", "Vielleicht" slowly fade in from black to white on a black screen |
| `Magic8BallEnhanced` | the same answers on a blue die triangle that rises and fades in, like in the real ball |
| `RadioTuner` | an FM tuner: a marker glides along the 87.5–108 MHz scale from station to station (FFH, Radio Bob, hr3, hr1), the frequency counts along and the station name fades in |
| `Oscilloscope` | an old green phosphor oscilloscope: graticule with divisions and a running sine wave with a soft glow |
| `Radar` | a radar sweep with fading trail over range rings; now and then a random hostile contact appears, lights up on each pass and fades |
| `Circles` | coloured rings appear from the edge inwards to the centre, then are erased from the centre outwards; positions and colours are calculated from `RING_COUNT` |

Each sketch folder is self-contained for the Arduino IDE, so `config.h` and `boards.h` exist in every folder (the `display_*.h` files differ per demo); change them everywhere when you change one.
On a black screen the answers "Ja", "Nein" and "Vielleicht" slowly fade in from black to white, hold, fade out, then the screen stays dark before the next answer.

Blog post (German): https://www.fambach.net (link follows)

## Magic 8 Ball

Folders `Magic8Ball` and `Magic8BallEnhanced`. Only the area that changes is redrawn, in 36-line bands (14–16 KB) instead of a full frame buffer, so it also fits into the ESP8266.

### Configuration (`config.h`)

| Setting | Values |
|---|---|
| `DISPLAY_LIBRARY` | `LIBRARY_ADAFRUIT` (Adafruit GC9A01A + Adafruit GFX, default), `LIBRARY_LOVYANGFX` |
| `TARGET_BOARD` | `BOARD_AUTO` (default), `BOARD_ESP8266_DEV`, `BOARD_ESP32_DEV`, `BOARD_ESP32C3_SUPER_MINI`, `BOARD_LOLIN_C3_MINI` |

`BOARD_AUTO` picks the board from the chip selected in the Arduino IDE: ESP8266 → ESP8266 dev, ESP32 → ESP32 dev, ESP32-C3 → C3 Super Mini. Pins are in `boards.h`.

### Wiring

ESP8266 dev, ESP32 dev and LOLIN C3 Mini use the same D1 Mini pin positions.

| Display | D1 pin | ESP8266 dev (D1 Mini, NodeMCU) | ESP32 dev (D1 Mini ESP32) | LOLIN C3 Mini | ESP32-C3 Super Mini |
|---|---|---|---|---|---|
| VCC | 3V3 | 3.3 V | 3.3 V | 3.3 V | 3.3 V |
| GND | G | GND | GND | GND | GND |
| SCL (SCK) | D5 | GPIO 14 | GPIO 18 | GPIO 1 | GPIO 4 |
| SDA (MOSI) | D7 | GPIO 13 | GPIO 23 | GPIO 4 | GPIO 6 |
| CS | D8 | GPIO 15 | GPIO 5 | GPIO 5 | GPIO 7 |
| DC | D3 | GPIO 0 | GPIO 17 | GPIO 7 | GPIO 5 |
| RES | D0 | GPIO 16 | GPIO 26 | GPIO 2 | GPIO 3 |
| BLK | – | 3.3 V | 3.3 V | 3.3 V | GPIO 10 |

- The module is rated 2.8–3.3 V: do not connect VCC to 5 V.
- ESP8266: D3/D4/D8 are boot pins, so BLK goes to 3V3.
- ESP32 with WROVER module: GPIO 16/17 belong to the PSRAM, move DC.
- LOLIN C3 Mini: the RGB LED also sits on GPIO 7 and may flicker while drawing.
- ESP32-C3 Super Mini: boot pins GPIO 2, 8, 9 stay free.

## Libraries

Install via the Arduino Library Manager:

| Library | Needed for | URL |
|---|---|---|
| Adafruit GC9A01A | `LIBRARY_ADAFRUIT` | https://github.com/adafruit/Adafruit_GC9A01A |
| Adafruit GFX Library | `LIBRARY_ADAFRUIT` | https://github.com/adafruit/Adafruit-GFX-Library |
| Adafruit BusIO | `LIBRARY_ADAFRUIT` | https://github.com/adafruit/Adafruit_BusIO |
| LovyanGFX | `LIBRARY_LOVYANGFX` | https://github.com/lovyan03/LovyanGFX |

Board packages: [esp8266](https://github.com/esp8266/Arduino) for the ESP8266, [esp32 by Espressif](https://github.com/espressif/arduino-esp32) for ESP32 and ESP32-C3.

## Status

- `Magic8Ball` and `Magic8BallEnhanced` run on an ESP32-C3 Super Mini (tested by Stefan, 2026-11-06).
- Everything else is not yet tested on hardware.

## License

PolyForm Noncommercial 1.0.0, see [LICENSE.md](LICENSE.md). Copyright (c) 2026 Stefan Fambach (SteFam).
The libraries used have their own licenses.

## Authors

- **Stefan Fambach** (SteFam) – idea and concept, review and testing – https://www.fambach.net
- **Claude** (Anthropic AI) – implementation, via Claude Code

## AI disclosure

This project is built with AI assistance. Stefan Fambach had the idea, wrote the concept and reviews and tests every change; the implementation (code and documentation) is written largely by Claude (Anthropic) via Claude Code.
