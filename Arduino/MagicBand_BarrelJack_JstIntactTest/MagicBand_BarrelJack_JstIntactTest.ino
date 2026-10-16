/*
   Magic Band -- Barrel-Jack variant, JST-INTACT WIRING TEST build
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   Purpose (2026-10-13, after the second smoke incident): isolate whether
   the smoke was caused ONLY by a wiring/color-convention mixup, not by
   the LED strip's current draw or anything about the board itself. This
   build runs ONE strip through the EXISTING J6 JST connector, kept fully
   intact -- data, +5V, AND ground all through the connector normally, no
   direct-to-barrel-jack power bypass this time. That's the one variable
   under test: does a correctly-wired full JST connection (even with the
   pigtail's inverted black=+5V/red=GND convention) run safely?

   @user: wire the strip to match the JST pigtail's ACTUAL convention
   (black=+5V, red=GND -- backwards of the red=+/black=GND standard,
   confirmed in 4. Technical Reference.md / WP4.1 from 2026-10-02), not
   the conventional colors. The point here is correctness, not convention.

   No RFID, no RF -- pure visual/thermal test, cycles through a rainbow
   wheel plus solid-color holds (including full white, the worst case for
   current draw) so you can watch for anomalies and feel for heat. Power-
   cycle to restart.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels:
     Single strip -> D5 (J6's data line). LED_COUNT defaults to 38 (the
     outer-ring count) -- change it if testing a different strip/count.
*/

#include <Adafruit_NeoPixel.h>

// -- LED Strip Configuration ------------------------------------------------
#define LED_COUNT  38  // change to match whichever strip is actually plugged into J6

const uint8_t PIN_LEDS = 5;

Adafruit_NeoPixel leds(LED_COUNT, PIN_LEDS, NEO_GRB + NEO_KHZ800);

// -- Timing -----------------------------------------------------------------
const uint16_t RAINBOW_STEP_MS = 20;   // delay per step of the rainbow wheel
const unsigned long SOLID_HOLD_MS = 4000UL;  // how long each solid color holds

//*******************************************************************************//

// Classic rainbow-wheel helper (0-255 -> a color around the color wheel)
uint32_t wheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return leds.Color(255 - pos * 3, 0, pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return leds.Color(0, pos * 3, 255 - pos * 3);
  } else {
    pos -= 170;
    return leds.Color(pos * 3, 255 - pos * 3, 0);
  }
}

void rainbowCycle() {
  for (uint16_t j = 0; j < 256; j++) {
    for (uint16_t i = 0; i < leds.numPixels(); i++) {
      leds.setPixelColor(i, wheel((i * 256 / leds.numPixels() + j) & 255));
    }
    leds.show();
    delay(RAINBOW_STEP_MS);
  }
}

void solidHold(uint8_t r, uint8_t g, uint8_t b) {
  leds.fill(leds.Color(r, g, b));
  leds.show();
  delay(SOLID_HOLD_MS);
}

void setup() {
  Serial.begin(9600);
  leds.begin();
  leds.fill(0);
  leds.show();
  Serial.println(F("JST-INTACT WIRING TEST -- do not ship. Rainbow + solid-color cycle, no RFID/RF."));
  Serial.flush();
}

void loop() {
  rainbowCycle();
  solidHold(255, 0, 0);    // red
  solidHold(0, 255, 0);    // green
  solidHold(0, 0, 255);    // blue
  solidHold(255, 255, 255); // white -- worst case for current draw, watch/feel for heat here
}
