/*
   Magic Band -- Barrel-Jack variant, THERMAL SOAK TEST build (v4, 2026-10-23)
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   Pure thermal stress test -- both strips solid white, constantly, from
   power-up, held forever. No RFID, no RF, no animation, no tap control --
   just the worst-case sustained load for checking temperatures on the
   bench. Power-cycle to restart (nothing to restart, really -- it's
   already doing the one thing it does the moment it boots).

   Brightness is 50% (128/255) -- a 2x factor-of-safety margin over
   production's actual 25% (64/255) setting, specifically to stress-test
   above what the board will ever actually be asked to sustain.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels:
     Outer ring -> D5  |  38px
     Inner ring -> D2  |  34px -- bodged directly to U1's DIP leg, see
       "Build Instructions - Barrel-Jack Variant.md" Step 5a. Both strips'
       VCC wired directly to +5V.
*/

#include <Adafruit_NeoPixel.h>

#define LED_COUNT_OUTER  38
#define LED_COUNT_INNER  34

const uint8_t PIN_LEDS_OUTER = 5;
const uint8_t PIN_LEDS_INNER = 2;  // bodged to U1's D2 DIP leg -- see Build Instructions Step 5a

const uint8_t BRIGHTNESS = 128;  // 50% -- 2x factor of safety over production's 25%

Adafruit_NeoPixel ledsOuter(LED_COUNT_OUTER, PIN_LEDS_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsInner(LED_COUNT_INNER, PIN_LEDS_INNER, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);

  ledsOuter.begin();
  ledsOuter.setBrightness(BRIGHTNESS);
  ledsInner.begin();
  ledsInner.setBrightness(BRIGHTNESS);

  ledsOuter.fill(ledsOuter.Color(255, 255, 255));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(255, 255, 255));
  ledsInner.show();

  Serial.println(F("THERMAL SOAK TEST -- do not ship. Both strips solid white @ 50%, held forever."));
}

void loop() {
  // Nothing to do -- both strips are already set and held in setup().
}
