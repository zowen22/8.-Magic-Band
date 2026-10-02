/*
   Minimal LED-only bench test for MagicBand_BarrelJack's U1 + J6.
   No MFRC522/RF dependency at all -- isolates "is the chip running
   code" and "does the NeoPixel strip respond to real commands" from
   everything else. Same LED_COUNT/PIN_LEDS as the real firmware.

   Expected behavior if this is working: strip visibly cycles
   red -> green -> blue -> off, repeating, about once per second per
   color. Anything else (stuck color, no change, stays off) means the
   chip isn't running this program correctly.
*/

#include <Adafruit_NeoPixel.h>

#define LED_COUNT  16
const uint8_t PIN_LEDS = 5;

Adafruit_NeoPixel leds(LED_COUNT, PIN_LEDS, NEO_GRB + NEO_KHZ800);

void fillAndShow(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < leds.numPixels(); i++) {
    leds.setPixelColor(i, leds.Color(r, g, b));
  }
  leds.show();
}

void setup() {
  leds.begin();
  leds.show(); // all off immediately on boot
}

void loop() {
  fillAndShow(255, 0, 0);   // red
  delay(1000);
  fillAndShow(0, 255, 0);   // green
  delay(1000);
  fillAndShow(0, 0, 255);   // blue
  delay(1000);
  fillAndShow(0, 0, 0);     // off
  delay(1000);
}
