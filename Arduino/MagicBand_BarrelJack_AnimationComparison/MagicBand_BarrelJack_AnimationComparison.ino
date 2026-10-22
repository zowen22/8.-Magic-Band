/*
   Magic Band -- Barrel-Jack variant, ANIMATION COMPARISON build (2026-10-22, live-tunable)
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   No RFID, no RF, no card scan needed. Three outer-ring spin variants,
   all parameters live-adjustable over Serial (9600 baud) so you can dial
   things in against a video reference without reflashing:

   Serial commands (type in the Serial Monitor, Enter to send):
     b<0-255>   brightness, both strips                (e.g. "b64" = 25%)
     l<seconds> lap duration, e.g. "l0.25" for a quarter-second lap
     h<seconds> hold duration in cycle mode only, e.g. "h1.5"
     m<-1|0|1|2> which animation runs:
                  -1 = cycle all three (default, original comparison behavior)
                   0 = EXISTING only, looped continuously
                   1 = FILLEMPTY only, looped continuously
                   2 = HALFRING only, looped continuously
     p or ?     print current settings

   In single-animation mode (m0/m1/m2) the pattern just loops forever, no
   hold in between -- meant for staring at one pattern on repeat while you
   tune l/b live against reference footage. Mode -1 restores the original
   cycle-through-all-three behavior.

   Brightness and lap-duration changes apply within a few pixel-steps,
   not just at the next full lap -- stepDelay is recomputed from the live
   globals every step, not cached once per call.

   The three variants:
   1. EXISTING -- single white point, 3 laps, exponential speed-up
      (60ms/step start, 5ms/step end) -- not affected by the `l` command,
      it has its own fixed ramp.
   2. FILLEMPTY -- ring fills (counterclockwise from pixel 0) over one
      lap, then seamlessly continues spinning but emptying over the next
      lap. One lap = `l` seconds.
   3. HALFRING -- half the strip lit as a contiguous arc, spinning as a
      block. One full rotation = `l` seconds.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels:
     Outer ring -> D5  |  38px
     Inner ring -> D2  |  34px -- bodged directly to U1's DIP leg, see
       "Build Instructions - Barrel-Jack Variant.md" Step 5a
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

const uint8_t R_SPIN = 255, G_SPIN = 255, B_SPIN = 255;  // white
const uint8_t R_HOLD = 0,   G_HOLD = 255, B_HOLD = 0;    // green

// -- Live-tunable parameters, changed only via Serial commands --------------
uint8_t brightness = 64;      // 25%
float   lapSeconds  = 1.0 / 3.0;
float   holdSeconds = 0.5;
int8_t  mode = -1;             // -1 = cycle all, 0/1/2 = single animation looped

//*******************************************************************************//

void printSettings() {
  Serial.print(F("brightness=")); Serial.print(brightness);
  Serial.print(F("  lapSeconds=")); Serial.print(lapSeconds, 3);
  Serial.print(F("  holdSeconds=")); Serial.print(holdSeconds, 3);
  Serial.print(F("  mode=")); Serial.println(mode);
}

// Non-blocking: processes at most one pending command line per call. Call
// this frequently (every pixel-step) so changes feel live, not just at the
// next full animation cycle.
void checkSerial() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  char cmd = line.charAt(0);
  String rest = line.substring(1);

  switch (cmd) {
    case 'b': {
      int v = constrain(rest.toInt(), 0, 255);
      brightness = (uint8_t)v;
      ledsOuter.setBrightness(brightness);
      ledsInner.setBrightness(brightness);
      printSettings();
      break;
    }
    case 'l': {
      float v = rest.toFloat();
      if (v > 0.01) lapSeconds = v;
      printSettings();
      break;
    }
    case 'h': {
      float v = rest.toFloat();
      if (v >= 0) holdSeconds = v;
      printSettings();
      break;
    }
    case 'm': {
      int v = rest.toInt();
      if (v >= -1 && v <= 2) mode = (int8_t)v;
      printSettings();
      break;
    }
    case 'p':
    case '?':
      printSettings();
      break;
    default:
      Serial.println(F("unknown command -- use b/l/h/m/p"));
  }
}

void allOff() {
  ledsOuter.fill(0);
  ledsOuter.show();
  ledsInner.fill(0);
  ledsInner.show();
}

void holdBothGreen() {
  ledsOuter.fill(ledsOuter.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsInner.show();
  unsigned long start = millis();
  while (millis() - start < (unsigned long)(holdSeconds * 1000.0)) {
    checkSerial();
    delay(20);
  }
}

// -- EXISTING (production animation, fixed exponential ramp, not `l`-tunable) --
void existingSpin() {
  const uint8_t  SPIN_LAPS = 3;
  const float    SPIN_DELAY_START_MS = 60.0;
  const float    SPIN_DELAY_END_MS   = 5.0;
  const uint16_t totalSteps = (uint16_t)SPIN_LAPS * ledsOuter.numPixels();
  int prevPixel = -1;

  for (uint16_t step = 0; step < totalSteps; step++) {
    checkSerial();
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

// -- FILLEMPTY -- one lap = `lapSeconds`, recomputed live every step --------
void fillEmptySpin(uint8_t numSpins) {
  uint16_t n = ledsOuter.numPixels();
  bool filling = true;

  for (uint8_t s = 0; s < numSpins; s++) {
    for (uint16_t f = 0; f < n; f++) {
      checkSerial();
      ledsOuter.setPixelColor(f, filling ? ledsOuter.Color(R_SPIN, G_SPIN, B_SPIN) : 0);
      ledsOuter.show();
      uint16_t stepDelay = (uint16_t)(lapSeconds * 1000.0 / n);
      delay(stepDelay);
    }
    filling = !filling;
  }
}

// -- HALFRING -- one rotation = `lapSeconds`, recomputed live every step ----
void halfRingSpin(uint8_t numSpins) {
  uint16_t n = ledsOuter.numPixels();
  uint16_t arcLen = n / 2;

  for (uint8_t s = 0; s < numSpins; s++) {
    for (uint16_t shift = 0; shift < n; shift++) {
      checkSerial();
      for (uint16_t i = 0; i < n; i++) {
        uint16_t rel = (i + n - shift) % n;
        ledsOuter.setPixelColor(i, (rel < arcLen) ? ledsOuter.Color(R_SPIN, G_SPIN, B_SPIN) : 0);
      }
      ledsOuter.show();
      uint16_t stepDelay = (uint16_t)(lapSeconds * 1000.0 / n);
      delay(stepDelay);
    }
  }
}

void setup() {
  Serial.begin(9600);
  ledsOuter.begin();
  ledsOuter.setBrightness(brightness);
  ledsInner.begin();
  ledsInner.setBrightness(brightness);
  allOff();
  Serial.println(F("ANIMATION COMPARISON (live-tunable) -- do not ship."));
  Serial.println(F("Commands: b<0-255> l<seconds> h<seconds> m<-1|0|1|2> p"));
  printSettings();
}

void loop() {
  checkSerial();

  switch (mode) {
    case 0:
      existingSpin();
      break;
    case 1:
      fillEmptySpin(2);  // one fill + one empty = one clean repeating unit
      break;
    case 2:
      halfRingSpin(1);   // one full rotation per call, loop() repeats it
      break;
    default:  // -1: cycle all three, original comparison behavior
      existingSpin();
      holdBothGreen();
      allOff();
      fillEmptySpin(3);
      holdBothGreen();
      allOff();
      halfRingSpin(3);
      holdBothGreen();
      allOff();
      break;
  }
}
