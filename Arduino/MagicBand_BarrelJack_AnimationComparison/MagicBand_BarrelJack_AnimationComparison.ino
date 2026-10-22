/*
   Magic Band -- Barrel-Jack variant, ANIMATION COMPARISON build (2026-10-20)
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   No RFID, no RF, no card scan needed -- continuously cycles through FOUR
   outer-ring spin variants back to back, for side-by-side comparison before
   picking one for production. Each variant does 3 spins, then both strips
   hold solid green for 0.5s, then the next variant starts. Repeats forever.
   Outer ring only animates; inner ring stays off except during the green
   holds (matches "obviously this is just the outer ring and the inner only
   is on for the holds").

   The four variants, in cycle order:
   1. EXISTING -- the current production animation unchanged: single white
      point, 3 laps, exponential speed-up (60ms/step start, 5ms/step end).
   2. FILLEMPTY_A -- ring fills (counterclockwise from pixel 0) over one
      lap, then seamlessly continues spinning but emptying over the next
      lap, alternating. Each individual spin (one fill lap OR one empty
      lap) takes 1/3 second. 3 spins = fill, empty, fill -- ends fully lit.
   3. FILLEMPTY_B -- identical to #2, but each individual spin takes 1/4
      second instead of 1/3.
   4. HALFRING -- half the strip (19 of 38px) lit as a contiguous arc,
      spinning as a block. One full rotation takes 1/3 second. 3 spins =
      3 full rotations.

   Assumptions, correct and reflash if any are wrong:
   - "Counterclockwise from the top" is read as increasing pixel index
     from 0 -- same convention the existing animation already uses. Actual
     physical direction depends on how the strip is mounted/wired; this
     sketch can't verify that from code alone.
   - FillEmpty's "1/4 second cycle" (variant 3) is treated as the SAME kind
     of per-spin duration as variant 2's "1/3 second individual spin," not
     a full fill+empty pair taking 1/4s (which would mean 1/8s per spin
     instead) -- picked for a cleaner apples-to-apples speed comparison
     between the two. Flag me if you meant the full-cycle reading instead.
   - Continuous auto-cycling, no card needed -- the request mentioned "on
     scan" for the first FillEmpty description, then pivoted ("Actually
     let's make a comparison sketch...") to a cycling comparison reel,
     which is treated as the final, overriding scope.
   - Outer ring kept at the same 50% brightness as production (so the
     comparison reflects what will actually ship); inner ring at 100%,
     matching production.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels:
     Outer ring -> D5  |  38px  |  50% brightness
     Inner ring -> D2  |  34px -- bodged directly to U1's DIP leg, see
       "Build Instructions - Barrel-Jack Variant.md" Step 5a  |  100% brightness
*/

#include <Adafruit_NeoPixel.h>
#include <math.h>

// -- LED Strip Configuration ------------------------------------------------
#define LED_COUNT_OUTER  38
#define LED_COUNT_INNER  34

const uint8_t PIN_LEDS_OUTER = 5;
const uint8_t PIN_LEDS_INNER = 2;  // bodged to U1's D2 DIP leg -- see Build Instructions Step 5a

Adafruit_NeoPixel ledsOuter(LED_COUNT_OUTER, PIN_LEDS_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsInner(LED_COUNT_INNER, PIN_LEDS_INNER, NEO_GRB + NEO_KHZ800);

// -- Shared timing/color -----------------------------------------------------
const unsigned long HOLD_MS = 500UL;
const uint8_t R_SPIN = 255, G_SPIN = 255, B_SPIN = 255;  // white
const uint8_t R_HOLD = 0,   G_HOLD = 255, B_HOLD = 0;    // green

//*******************************************************************************//

void allOff() {
  ledsOuter.fill(0);
  ledsOuter.show();
  ledsInner.fill(0);
  ledsInner.show();
}

void holdBothGreen(unsigned long ms) {
  ledsOuter.fill(ledsOuter.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsInner.show();
  delay(ms);
}

// -- Variant 1: EXISTING (production animation, unchanged) -------------------
void existingSpin() {
  const uint8_t  SPIN_LAPS = 3;
  const float    SPIN_DELAY_START_MS = 60.0;
  const float    SPIN_DELAY_END_MS   = 5.0;
  const uint16_t totalSteps = (uint16_t)SPIN_LAPS * ledsOuter.numPixels();
  int prevPixel = -1;

  for (uint16_t step = 0; step < totalSteps; step++) {
    int i = step % ledsOuter.numPixels();
    if (prevPixel >= 0) ledsOuter.setPixelColor(prevPixel, 0);
    ledsOuter.setPixelColor(i, ledsOuter.Color(R_SPIN, G_SPIN, B_SPIN));
    ledsOuter.show();

    float t = (totalSteps > 1) ? (float)step / (float)(totalSteps - 1) : 1.0;
    float stepDelay = SPIN_DELAY_START_MS * pow(SPIN_DELAY_END_MS / SPIN_DELAY_START_MS, t);
    delay((unsigned long)stepDelay);

    prevPixel = i;
  }
  ledsOuter.setPixelColor(prevPixel, 0);
  ledsOuter.show();
}

// -- Variants 2/3: FILLEMPTY --------------------------------------------------
// One "spin" = one full lap of either filling (pixel 0 upward lighting up)
// or emptying (pixel 0 upward turning off) -- alternating, starting with
// fill. numSpins=3 -> fill, empty, fill (ends fully lit).
void fillEmptySpin(float lapSeconds, uint8_t numSpins) {
  uint16_t n = ledsOuter.numPixels();
  uint16_t stepDelay = (uint16_t)(lapSeconds * 1000.0 / n);
  bool filling = true;

  for (uint8_t s = 0; s < numSpins; s++) {
    for (uint16_t f = 0; f < n; f++) {
      ledsOuter.setPixelColor(f, filling ? ledsOuter.Color(R_SPIN, G_SPIN, B_SPIN) : 0);
      ledsOuter.show();
      delay(stepDelay);
    }
    filling = !filling;
  }
}

// -- Variant 4: HALFRING -------------------------------------------------------
// A contiguous arc of half the strip lit, spinning as a block. One full
// rotation (arc returns to its starting position) = lapSeconds.
void halfRingSpin(float lapSeconds, uint8_t numSpins) {
  uint16_t n = ledsOuter.numPixels();
  uint16_t arcLen = n / 2;
  uint16_t stepDelay = (uint16_t)(lapSeconds * 1000.0 / n);

  for (uint8_t s = 0; s < numSpins; s++) {
    for (uint16_t shift = 0; shift < n; shift++) {
      for (uint16_t i = 0; i < n; i++) {
        uint16_t rel = (i + n - shift) % n;
        ledsOuter.setPixelColor(i, (rel < arcLen) ? ledsOuter.Color(R_SPIN, G_SPIN, B_SPIN) : 0);
      }
      ledsOuter.show();
      delay(stepDelay);
    }
  }
}

void setup() {
  Serial.begin(9600);
  ledsOuter.begin();
  ledsOuter.setBrightness(64);  // 25%
  ledsInner.begin();
  ledsInner.setBrightness(64);  // 25%
  allOff();
  Serial.println(F("ANIMATION COMPARISON -- do not ship. Cycling: existing / fillEmpty-1/3s / fillEmpty-1/4s / halfRing-1/3s."));
  Serial.flush();
}

void loop() {
  existingSpin();
  holdBothGreen(HOLD_MS);
  allOff();

  fillEmptySpin(1.0 / 3.0, 3);
  holdBothGreen(HOLD_MS);
  allOff();

  fillEmptySpin(1.0 / 4.0, 3);
  holdBothGreen(HOLD_MS);
  allOff();

  halfRingSpin(1.0 / 3.0, 3);
  holdBothGreen(HOLD_MS);
  allOff();
}
