/*
   Magic Band -- Barrel-Jack variant, ANIMATION COMPARISON build (2026-10-22, tag-scan version)
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   Loops the CURRENT animation continuously from power-up -- no tap
   needed to start. A tap just advances which animation is selected,
   cycling EXISTING -> FILLEMPTY -> HALFRINGACCEL -> HALFRINGDIM ->
   EXISTING -> ..., and the newly-selected one takes over on the next
   play-through. ANIMATION_GAP_MS (below) is the pause between each
   loop/repetition of whichever animation is currently playing, not a
   tap debounce.

   HALFRINGACCEL (2026-10-23) replaces the old plain HALFRING -- built
   from a real video reference of actual Disney park entrances. Rough
   per-rotation timestamps read off that footage: 1.75s, 2.2s, 2.6s,
   2.95s, 3.2s, 3.5s, 3.75s, 4.0s, then fill -- i.e. roughly 8 rotations
   accelerating from ~0.45s/lap down to ~0.25s/lap, not perfectly
   monotonic (video-timing read noise), used here as a rough shape
   reference rather than replicated exactly. Implemented as an
   exponential accel curve (HALFRING_ACCEL_RATIO controls how much
   faster the last lap is than the first) auto-scaled so the total
   always sums to exactly HALFRING_ACCEL_TOTAL_SECONDS regardless of
   the other parameters -- change the total, nothing else needs
   retuning.

   Both half-fill animations (HALFRINGACCEL and HALFRINGDIM) now taper
   the lit arc's own leading/trailing edges to a dim fade instead of a
   hard on/off cutoff (HALFRING_TAPER_PIXELS/_MIN below) -- a small,
   shared change applied to both, independent of whether the "off" half
   is fully dark (HALFRINGACCEL) or dimly glowing (HALFRINGDIM).

   The RFID reader stays powered continuously -- no power-up/power-down
   cycling (this is a wall-powered bench/comparison build, not the
   battery-conscious production sketch, so there's no reason to power-
   cycle it). It's checked both right after each spin and throughout the
   gap, so a tap registers quickly regardless of when it happens.

   All the parameters you'd want to change are grouped at the top, below
   -- edit a value, reflash, no need to hunt through the animation code
   itself.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels:
     Outer ring -> D5  |  38px -- all three animations are outer-ring-only
     Inner ring -> D2  |  34px -- bodged directly to U1's DIP leg, see
       "Build Instructions - Barrel-Jack Variant.md" Step 5a -- stays off
       except during the green hold after each animation

   MFRC522 (SPI):
     RST -> D9  |  SS -> D4  |  MOSI -> D11  |  MISO -> D12  |  SCK -> D13
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// ============================================================================
// EASY-TO-EDIT PARAMETERS -- change these, reflash, nothing else to hunt for
// ============================================================================

#define LED_COUNT_OUTER  38
#define LED_COUNT_INNER  34

const uint8_t PIN_LEDS_OUTER = 5;
const uint8_t PIN_LEDS_INNER = 2;   // bodged to U1's D2 DIP leg -- see Build Instructions Step 5a

const uint8_t BRIGHTNESS_OUTER = 64;  // 0-255 (64 = 25%)
const uint8_t BRIGHTNESS_INNER = 64;  // 0-255 (64 = 25%)

const float   FILLEMPTY_LAP_SECONDS = 1.0 / 3.0;  // one fill lap or one empty lap
const uint8_t FILLEMPTY_NUM_SPINS   = 3;           // fill, empty, fill (odd count ends fully lit)

const float   HALFRING_ACCEL_TOTAL_SECONDS = 2.5;  // whole animation's duration -- change this alone, the per-lap timing rescales automatically
const uint8_t HALFRING_ACCEL_NUM_SPINS     = 8;     // number of rotations within that total
const float   HALFRING_ACCEL_RATIO         = 1.8;   // first rotation takes this many times longer than the last (shape of the acceleration, independent of total time)

const float   HALFRINGDIM_LAP_SECONDS = 1.0 / 3.0;  // same idea as HALFRINGACCEL, but the "off" half glows dim instead of going dark, and at a constant speed
const uint8_t HALFRINGDIM_NUM_SPINS   = 3;
const uint8_t HALFRINGDIM_DIM_LEVEL   = 20;          // 0-255 brightness of the dim half (20 = faint glow, not off)

// Shared by both half-fill animations: tapers the lit arc's own edges to a
// dim fade instead of a hard cutoff.
const uint8_t HALFRING_TAPER_PIXELS = 4;   // how many pixels at each edge of the lit half fade
const uint8_t HALFRING_TAPER_MIN    = 60;  // brightness (0-255) at the very edge pixel, before it reaches the off-arc level

const unsigned long HOLD_MS         = 500UL;   // both strips green, after each animation finishes
const unsigned long ANIMATION_GAP_MS = 1000UL; // flat delay after each run before the next tap is acted on -- reader stays powered/scanning through this, just not acted on

const uint8_t R_SPIN = 255, G_SPIN = 255, B_SPIN = 255;  // white, all three animations
const uint8_t R_HOLD = 0,   G_HOLD = 255, B_HOLD = 0;    // green, the post-animation hold

// ============================================================================
// End of editable parameters
// ============================================================================

Adafruit_NeoPixel ledsOuter(LED_COUNT_OUTER, PIN_LEDS_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsInner(LED_COUNT_INNER, PIN_LEDS_INNER, NEO_GRB + NEO_KHZ800);

const uint8_t RST_PIN = 9;
const uint8_t SS_PIN  = 4;
MFRC522 mfrc522(SS_PIN, RST_PIN);

uint8_t currentAnimation = 0;  // 0=existing, 1=fillEmpty, 2=halfRing -- advances on every tap

//*******************************************************************************//

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
  delay(HOLD_MS);
}

// -- Animation 0: EXISTING (production animation, unchanged) ----------------
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

// -- Animation 1: FILLEMPTY ---------------------------------------------------
// One "spin" = one full lap of either filling (pixel 0 upward lighting up)
// or emptying (pixel 0 upward turning off) -- alternating, starting with
// fill. FILLEMPTY_NUM_SPINS=3 -> fill, empty, fill (ends fully lit).
void fillEmptySpin() {
  uint16_t n = ledsOuter.numPixels();
  uint16_t stepDelay = (uint16_t)(FILLEMPTY_LAP_SECONDS * 1000.0 / n);
  bool filling = true;

  for (uint8_t s = 0; s < FILLEMPTY_NUM_SPINS; s++) {
    for (uint16_t f = 0; f < n; f++) {
      ledsOuter.setPixelColor(f, filling ? ledsOuter.Color(R_SPIN, G_SPIN, B_SPIN) : 0);
      ledsOuter.show();
      delay(stepDelay);
    }
    filling = !filling;
  }
}

// Shared by both half-fill animations below: returns 0-255 brightness for a
// pixel at position `rel` inside a lit arc of length `arcLen`, fading the
// pixel toward the edge over HALFRING_TAPER_PIXELS instead of a hard cutoff.
uint8_t litArcBrightness(uint16_t rel, uint16_t arcLen) {
  uint16_t distFromEdge = min(rel, (uint16_t)(arcLen - 1 - rel));
  if (distFromEdge >= HALFRING_TAPER_PIXELS) return 255;
  return (uint8_t)(HALFRING_TAPER_MIN + (255 - HALFRING_TAPER_MIN) * (float)distFromEdge / (float)HALFRING_TAPER_PIXELS);
}

// -- Animation 2: HALFRINGACCEL -----------------------------------------------
// A contiguous arc of half the strip lit (edges tapered, see litArcBrightness),
// spinning as a block, accelerating over HALFRING_ACCEL_NUM_SPINS rotations.
// Per-lap durations are an exponential curve from HALFRING_ACCEL_RATIO:1 down
// to 1:1, rescaled so they always sum to exactly HALFRING_ACCEL_TOTAL_SECONDS.
void halfRingAccelSpin() {
  uint16_t n = ledsOuter.numPixels();
  uint16_t arcLen = n / 2;

  float weights[HALFRING_ACCEL_NUM_SPINS];
  float weightSum = 0;
  for (uint8_t i = 0; i < HALFRING_ACCEL_NUM_SPINS; i++) {
    float frac = (HALFRING_ACCEL_NUM_SPINS > 1) ? (float)i / (float)(HALFRING_ACCEL_NUM_SPINS - 1) : 1.0;
    weights[i] = pow(HALFRING_ACCEL_RATIO, 1.0 - frac);  // starts at RATIO, ends at 1
    weightSum += weights[i];
  }

  for (uint8_t s = 0; s < HALFRING_ACCEL_NUM_SPINS; s++) {
    float lapSeconds = HALFRING_ACCEL_TOTAL_SECONDS * (weights[s] / weightSum);
    uint16_t stepDelay = (uint16_t)(lapSeconds * 1000.0 / n);

    for (uint16_t shift = 0; shift < n; shift++) {
      for (uint16_t i = 0; i < n; i++) {
        uint16_t rel = (i + n - shift) % n;
        uint8_t level = (rel < arcLen) ? litArcBrightness(rel, arcLen) : 0;
        ledsOuter.setPixelColor(i, ledsOuter.Color(level, level, level));
      }
      ledsOuter.show();
      delay(stepDelay);
    }
  }
}

// -- Animation 3: HALFRINGDIM -------------------------------------------------
// Same lit-arc edge taper as HALFRINGACCEL, constant speed, but the "off"
// half glows at HALFRINGDIM_DIM_LEVEL instead of going fully dark.
void halfRingDimSpin() {
  uint16_t n = ledsOuter.numPixels();
  uint16_t arcLen = n / 2;
  uint16_t stepDelay = (uint16_t)(HALFRINGDIM_LAP_SECONDS * 1000.0 / n);

  for (uint8_t s = 0; s < HALFRINGDIM_NUM_SPINS; s++) {
    for (uint16_t shift = 0; shift < n; shift++) {
      for (uint16_t i = 0; i < n; i++) {
        uint16_t rel = (i + n - shift) % n;
        uint8_t level = (rel < arcLen) ? litArcBrightness(rel, arcLen) : HALFRINGDIM_DIM_LEVEL;
        ledsOuter.setPixelColor(i, ledsOuter.Color(level, level, level));
      }
      ledsOuter.show();
      delay(stepDelay);
    }
  }
}

void runCurrentAnimation() {
  switch (currentAnimation) {
    case 0: Serial.println(F("Playing: EXISTING"));     existingSpin();     break;
    case 1: Serial.println(F("Playing: FILLEMPTY"));    fillEmptySpin();    break;
    case 2: Serial.println(F("Playing: HALFRINGACCEL")); halfRingAccelSpin(); break;
    case 3: Serial.println(F("Playing: HALFRINGDIM"));  halfRingDimSpin();  break;
  }
}

// Checks for a tap once. If seen, advances currentAnimation to the next
// one in the cycle and returns true. Call this frequently (between spins,
// and throughout the gap) so the reader is effectively always scanning.
bool checkForTapAndAdvance() {
  bool sawCard = mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
  if (sawCard) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    currentAnimation = (currentAnimation + 1) % 4;
    Serial.print(F("Tap -- switching to animation "));
    Serial.println(currentAnimation);
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  SPI.begin();
  mfrc522.PCD_Init();
  // Deliberately no PCD_SoftPowerDown() here -- reader stays powered/
  // scanning continuously for this comparison build, unlike production's
  // battery-conscious power-cycling.

  ledsOuter.begin();
  ledsOuter.setBrightness(BRIGHTNESS_OUTER);
  ledsInner.begin();
  ledsInner.setBrightness(BRIGHTNESS_INNER);
  allOff();

  Serial.println(F("ANIMATION COMPARISON (tag-scan) -- do not ship. Loops the current animation continuously from power-up; tap advances to the next one."));
}

void loop() {
  // The current animation always plays, tap or no tap -- a tap just
  // advances which one is selected for the NEXT play-through.
  runCurrentAnimation();
  holdBothGreen();
  allOff();

  checkForTapAndAdvance();

  // Gap between loops -- reader stays active/checked throughout, so a tap
  // during the gap (not just right after a spin) still registers.
  unsigned long start = millis();
  while (millis() - start < ANIMATION_GAP_MS) {
    checkForTapAndAdvance();
    delay(20);
  }
}
