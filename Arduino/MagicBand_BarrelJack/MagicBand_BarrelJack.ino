/*
   Magic Band -- Barrel-Jack (Wall-Powered) variant
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/
   Derived from MagicBand_Wireless.ino, with the battery-only complexity
   removed: no sleep/wake watchdog loop, no MOSFET NeoPixel power gating.
   Continuous wall power via barrel jack -- nothing here to conserve.

   -- Pin Map (matches PCB/MagicBand_BarrelJack/MagicBand_BarrelJack.kicad_sch) --
   MFRC522 (SPI):
     RST -> D9  |  SS -> D4  |  MOSI -> D11  |  MISO -> D12  |  SCK -> D13
     VCC -> external 3.3V regulator output (MCP1700-3302)
     IRQ intentionally unconnected -- not used, PICC_IsNewCardPresent() is polled

   NeoPixels (two strips as of 2026-10-13):
     Outer ring -> D5  |  38px, denser/smaller LEDs than the original 16px strip
       |  driven at 50% brightness (setBrightness(128)) -- also eases current
       draw on the +5V rail, see the smoke-incident findings in WP4.3
     Inner ring -> D2  |  34px (corrected 2026-10-18, was 35px), NEW -- bodged directly to U1's DIP leg (no
       designed connector exists on rev 1; see "Build Instructions - Barrel-Jack
       Variant.md" Step 5a for the physical solder points). Rev 2 should give
       this its own connector -- see Work Packages WP4.3.  |  driven at 100%
       brightness
     Both strips' VCC wired directly to +5V, no gating -- always powered

   RF Transmitter (433MHz, D-FLIFE ASK TX):
     DATA -> D8  |  PWR IN -> external 3.3V regulator output (same rail as
     MFRC522 above -- this module is 3.3V-only, NOT the 5V bus)  |
     GND -> GND  |  short antenna

   3.3V Regulator (MCP1700-3302):
     IN -> board +5V (from barrel jack, through reverse-polarity protection)
     OUT -> MFRC522 VCC + TX module PWR IN, above  |  GND -> common ground

   Barrel jack power input has a P-channel MOSFET (Q1) reverse-polarity
   protection circuit ahead of the regulator and MCU -- added after a
   preliminary design review flagged bare wire leads with no mechanical
   keying as a real risk of a reversed/wrong adapter. See PCB Design Plan.md.

   RF codes below were captured via RF_Sniffer against the paired outlet
   remote's top-row ON/OFF buttons -- verified against the physical outlet
   the tree lights are plugged into (2026-08-03). Confirmed repeatable
   across multiple presses each, not a rolling code.

   -- Animation sequence, as of 2026-10-13, timing fixed 2026-10-18,
   animation replaced 2026-10-25, hold-override/transition fixed 2026-10-25 --
   On card tap: any hold still showing is cleared immediately (a new tap
   always overrides) -- outer ring then shows a half-ring arc that builds up
   pixel-by-pixel and rotates through 8 accelerating laps (HALFRINGACCEL,
   ported from the AnimationComparison sketch's video-reference-tuned
   version -- see that sketch's header for the real-footage timestamps this
   was built from). Both strips go straight to the solid-green hold the
   instant the spin ends -- no gap -- same as the comparison sketch's own
   animation -> hold transition. The MFRC522 is then fully powered down
   BEFORE the RF outlet toggle fires (fixed 2026-10-18 -- firing RF while the
   reader was still powered, sharing the same 3.3V rail, was the leading
   explanation for RF not reliably triggering the outlet in production
   despite a standalone TX-only test sketch working fine on the same board).
   Same alternating on/off RF logic as always. The green hold lasts HOLD_MS
   -- but card-scanning reopens just 1 second after RF fires, not after the
   full hold (fixed 2026-10-18, see scanLockoutUntil/holdActive). THERMAL_TEST_MODE below
   controls how long that hold lasts -- see its comment before flashing.
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <RCSwitch.h>
#include <math.h>

// -- LED Strip Configuration ------------------------------------------------
// Outer ring: existing strip position, now denser/smaller LEDs -- 38px (was 16).
// Inner ring: new strip, bodged to U1's D2 leg -- 35px. See header comment above.
#define LED_COUNT_OUTER  38
#define LED_COUNT_INNER  34

const uint8_t PIN_LEDS_OUTER = 5;
const uint8_t PIN_LEDS_INNER = 2;  // bodged to U1's D2 DIP leg -- carries no signal
                                    // on rev 1 boards until this firmware is flashed

Adafruit_NeoPixel ledsOuter(LED_COUNT_OUTER, PIN_LEDS_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsInner(LED_COUNT_INNER, PIN_LEDS_INNER, NEO_GRB + NEO_KHZ800);

// -- RFID --------------------------------------------------------------------
const uint8_t RST_PIN = 9;
const uint8_t SS_PIN  = 4;
MFRC522 mfrc522(SS_PIN, RST_PIN);

// -- RF Transmitter ------------------------------------------------------------
const uint8_t RF_PIN = 8;
RCSwitch rfSwitch = RCSwitch();

const unsigned long RF_CODE_ON   = 0x885A8F00;
const unsigned long RF_CODE_OFF  = 0x845A8F00;
const unsigned int  RF_BITS      = 32;
const unsigned int  RF_PROTOCOL  = 2;
const unsigned int  RF_PULSE_LEN = 613;  // confirmed against the real outlet 2026-08-15 (RF_PulseSweep bench tool) --
                                          // 700us decoded fine on our own bench RX but did NOT trigger the outlet;
                                          // the outlet's receiver needed a notably shorter pulse (560-665us worked)
const uint8_t        RF_REPEAT   = 5;  // 433MHz is fire-and-forget, no ACK -- resend

// -- Thermal test mode ---------------------------------------------------------
// 1 = hold the solid-green state indefinitely (thermal soak test, to check LED/
//     driver temperatures under sustained load) -- deliberately halts here and
//     never returns, so card polling stops too. Power-cycle to reset.
// 0 = normal production behavior: hold green for HOLD_MS, then off, then resume
//     polling for the next tap.
// Compile-time only -- flip this and reflash, not a runtime toggle.
#define THERMAL_TEST_MODE 0

const unsigned long HOLD_MS = 15000UL;  // production hold duration; unused in THERMAL_TEST_MODE

// -- Animation (HALFRINGACCEL, ported from AnimationComparison 2026-10-25) -----
// Whole animation's duration, INCLUDING the initial build-up -- change this
// alone, everything else rescales automatically.
const float   HALFRING_ACCEL_TOTAL_SECONDS = 1.75;
const uint8_t HALFRING_ACCEL_NUM_SPINS     = 8;    // rotations after the build-up
const float   HALFRING_ACCEL_RATIO         = 2.5;  // first rotation takes this many times longer than the last (shape, independent of total time)

// Tapers the lit arc's own leading/trailing edges to a dim fade instead of a
// hard on/off cutoff.
const uint8_t HALFRING_TAPER_PIXELS = 4;   // how many pixels at each edge of the lit half fade
const uint8_t HALFRING_TAPER_MIN    = 60;  // brightness (0-255) at the very edge pixel

const uint8_t  R_HOLD = 0, G_HOLD = 255, B_HOLD = 0;  // green

// -- State -----------------------------------------------------------------------
bool treeOn = false;
bool holdActive = false;
unsigned long holdEndAt = 0;
unsigned long scanLockoutUntil = 0;  // RFID polling is skipped entirely until this time

//*******************************************************************************//

void allOff() {
  ledsOuter.fill(0);
  ledsOuter.show();
  ledsInner.fill(0);
  ledsInner.show();
}

// Returns 0-255 brightness for a pixel at position `rel` inside a lit arc of
// length `arcLen`, fading the pixel toward the edge over
// HALFRING_TAPER_PIXELS instead of a hard cutoff.
uint8_t litArcBrightness(uint16_t rel, uint16_t arcLen) {
  uint16_t distFromEdge = min(rel, (uint16_t)(arcLen - 1 - rel));
  if (distFromEdge >= HALFRING_TAPER_PIXELS) return 255;
  return (uint8_t)(HALFRING_TAPER_MIN + (255 - HALFRING_TAPER_MIN) * (float)distFromEdge / (float)HALFRING_TAPER_PIXELS);
}

// Half-ring arc, outer ring only, white. Starts with a sequential build-up
// (the arc reveals one pixel at a time from pixel 0 upward, no rotation yet)
// then HALFRING_ACCEL_NUM_SPINS full rotations follow, accelerating from the
// first lap down to the last (HALFRING_ACCEL_RATIO:1). The build-up's speed
// is tied directly to lap 1's speed so there's no jump at the handoff: arcLen
// is exactly half of n, so giving the build-up half of lap 1's weight makes
// its per-pixel-step delay mathematically identical to lap 1's per-step
// delay. Everything sums to exactly HALFRING_ACCEL_TOTAL_SECONDS.
void spinAnimationOuter() {
  uint16_t n = ledsOuter.numPixels();
  uint16_t arcLen = n / 2;

  float lapWeights[16];  // fixed upper bound, comfortably above any realistic NUM_SPINS
  float lapWeightSum = 0;
  for (uint8_t i = 0; i < HALFRING_ACCEL_NUM_SPINS; i++) {
    float frac = (HALFRING_ACCEL_NUM_SPINS > 1) ? (float)i / (float)(HALFRING_ACCEL_NUM_SPINS - 1) : 1.0;
    lapWeights[i] = pow(HALFRING_ACCEL_RATIO, 1.0 - frac);  // starts at ratio, ends at 1
    lapWeightSum += lapWeights[i];
  }

  float buildupWeight = lapWeights[0] * 0.5;  // half of lap 1's weight, since arcLen = n/2
  float totalWeight = buildupWeight + lapWeightSum;

  // Build-up: arc fills in pixel-by-pixel, no rotation.
  float buildupSeconds = HALFRING_ACCEL_TOTAL_SECONDS * (buildupWeight / totalWeight);
  uint16_t buildupStepDelay = (uint16_t)(buildupSeconds * 1000.0 / arcLen);
  for (uint16_t f = 0; f < arcLen; f++) {
    for (uint16_t i = 0; i < n; i++) {
      uint8_t level = (i <= f) ? litArcBrightness(i, arcLen) : 0;
      ledsOuter.setPixelColor(i, ledsOuter.Color(level, level, level));
    }
    ledsOuter.show();
    delay(buildupStepDelay);
  }

  // The accelerating rotating laps, starting from the fully-revealed arc's
  // resting position (shift=0).
  for (uint8_t s = 0; s < HALFRING_ACCEL_NUM_SPINS; s++) {
    float lapSeconds = HALFRING_ACCEL_TOTAL_SECONDS * (lapWeights[s] / totalWeight);
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

void holdGreenBoth() {
  ledsOuter.fill(ledsOuter.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsInner.show();
}

void fireRfCode(unsigned long code) {
  rfSwitch.setProtocol(RF_PROTOCOL);
  rfSwitch.setPulseLength(RF_PULSE_LEN);
  rfSwitch.setRepeatTransmit(RF_REPEAT);
  rfSwitch.send(code, RF_BITS);
}

// One duty cycle: RFID powered up, briefly polled, powered back down.
// Returns true if a card was seen and handled.
bool pollCardOnce() {
  // Skip the RFID entirely during the post-fire lockout -- also means we
  // don't re-power it needlessly while a card might still be sitting on
  // the reader right after a tap.
  if (millis() < scanLockoutUntil) return false;

  mfrc522.PCD_SoftPowerUp();
  delay(2);  // datasheet-recommended settle time after power-up

  bool sawCard = mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();

  if (sawCard) {
    // A new tap always wins -- clear any hold still showing (spinAnimationOuter()
    // below only ever touches the outer ring, so without this the inner ring's
    // green from a still-active previous hold would keep showing right through
    // the new spin instead of being overridden).
    allOff();

    spinAnimationOuter();  // outer ring only, white, half-ring build-up + accel laps

    // Go straight to the green hold the instant the spin ends, same as the
    // AnimationComparison sketch's animation -> holdBothGreen() transition --
    // no gap. The MFRC522 halt/power-down + RF fire below take real time; with
    // the old blank-ending spin that gap was invisible, but this animation
    // ends on a lit arc, so doing those steps first left it frozen on screen
    // before green appeared. LEDs and MFRC522/RF are independent peripherals,
    // so there's no ordering requirement forcing this hold after RF.
    holdGreenBoth();  // both strips, solid green

    // Power the MFRC522 all the way down BEFORE firing RF, not after (fixed
    // 2026-10-18). Both modules share the 3.3V regulator rail, and the RF
    // TX module's transmission is timing-sensitive -- leaving the reader
    // still actively powered (its own 13.56MHz field possibly still
    // settling) right when RF needs to fire is the leading explanation for
    // why a standalone TX-only test sketch (no MFRC522 at all) transmitted
    // reliably on this same board while production didn't.
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    mfrc522.PCD_SoftPowerDown();

    // RF toggle fires right after the hold is already showing -- same
    // alternating on/off logic as always.
    treeOn = !treeOn;
    fireRfCode(treeOn ? RF_CODE_ON : RF_CODE_OFF);

#if THERMAL_TEST_MODE
    // Deliberate dead-end: thermal soak test holds the green fill indefinitely
    // to check LED/driver temperatures under sustained load. Never returns, so
    // card polling stops here too -- power-cycle to reset. Flip
    // THERMAL_TEST_MODE to 0 and reflash for production.
    while (true) { delay(1000); }
#else
    // Card-scanning reopens just 1s after RF fires (fixed 2026-10-18) --
    // decoupled from how long the LEDs stay lit. The green hold itself
    // still runs the full HOLD_MS, timed out separately in loop().
    scanLockoutUntil = millis() + 1000UL;
    holdActive = true;
    holdEndAt = millis() + HOLD_MS;
    return true;  // MFRC522 already powered down above -- skip the redundant call below
#endif
  }

  mfrc522.PCD_SoftPowerDown();
  return sawCard;
}

//*******************************************************************************//

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  mfrc522.PCD_SoftPowerDown();

  rfSwitch.enableTransmit(RF_PIN);

  ledsOuter.begin();
  ledsOuter.setBrightness(128);  // outer ring at 50% -- also eases current draw on the +5V rail
  ledsInner.begin();
  ledsInner.setBrightness(255);  // inner ring at 100%, explicit for clarity
  allOff();

  Serial.println(F("Ready -- polling continuously, no sleep/wake."));
  Serial.flush();
}

void loop() {
  // Turns the green hold off once HOLD_MS elapses, independent of scanning
  // (which reopens much sooner -- see scanLockoutUntil in pollCardOnce()).
  if (holdActive && millis() >= holdEndAt) {
    allOff();
    holdActive = false;
  }

  pollCardOnce();
  delay(50);  // small idle margin between soft-power-up/down cycles -- MFRC522 wasn't
              // characterized for zero-gap continuous power-cycling, so don't hammer it
}
