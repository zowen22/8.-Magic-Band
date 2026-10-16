/*
   Magic Band -- Barrel-Jack variant, THERMAL SOAK TEST build (v3, 2026-10-13)
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   Tap a card to turn the test ON -- a sharp white strobe (4 quick flashes,
   both strips) fires first to clearly signify "test mode engaged," not
   mistakable for the production spin or the smooth rainbow animations that
   follow. Once on, both strips continuously cycle through a variety of
   COMPLEMENTARY (not identical/mirrored) animations:
   - Rainbow: both cycle the color wheel together, inner ring 128 steps
     (half the wheel) ahead of outer -- opposite hues at every instant.
   - Solid-color pairs: red/cyan, blue/yellow, green/magenta, held in turn.
   - Chase: a white point chases clockwise on the outer ring while one
     chases counter-clockwise on the inner ring, at the same time.
   - Both strips full solid white together -- worst case for current draw,
     watch/feel for heat here -- before the cycle repeats.
   Tap again at any point to turn it off immediately (checked every ~150ms,
   not just between full cycles) -- both strips go dark, no strobe on the
   way out. Tap again to resume (strobe, then animations from the top).

   Assumption: no RF/outlet toggle here, unlike the production sketch --
   this is LED-only test mode. Say so if you want RF added back in.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels:
     Outer ring -> D5  |  38px
     Inner ring -> D2  |  35px -- bodged directly to U1's DIP leg, see
       "Build Instructions - Barrel-Jack Variant.md" Step 5a. Both strips'
       VCC wired directly to +5V.

   MFRC522 (SPI) -- same pins as production:
     RST -> D9  |  SS -> D4  |  MOSI -> D11  |  MISO -> D12  |  SCK -> D13
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>

// -- LED Strip Configuration ------------------------------------------------
#define LED_COUNT_OUTER  38
#define LED_COUNT_INNER  35

const uint8_t PIN_LEDS_OUTER = 5;
const uint8_t PIN_LEDS_INNER = 2;  // bodged to U1's D2 DIP leg -- see Build Instructions Step 5a

Adafruit_NeoPixel ledsOuter(LED_COUNT_OUTER, PIN_LEDS_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsInner(LED_COUNT_INNER, PIN_LEDS_INNER, NEO_GRB + NEO_KHZ800);

// -- RFID --------------------------------------------------------------------
const uint8_t RST_PIN = 9;
const uint8_t SS_PIN  = 4;
MFRC522 mfrc522(SS_PIN, RST_PIN);

// -- Timing -----------------------------------------------------------------
const uint16_t RAINBOW_STEP_MS = 20;
const uint16_t CHASE_STEP_MS   = 40;
const unsigned long SOLID_HOLD_MS = 3000UL;
const unsigned long WHITE_HOLD_MS = 5000UL;  // worst case for current draw
const unsigned long TAP_CHECK_INTERVAL_MS = 150;  // don't hammer the MFRC522 -- see Technical Reference

// -- State -----------------------------------------------------------------
bool testOn = false;
unsigned long lastTapCheck = 0;

//*******************************************************************************//

void allOff() {
  ledsOuter.fill(0);
  ledsOuter.show();
  ledsInner.fill(0);
  ledsInner.show();
}

// Sharp, unmistakable "test mode engaged" signal -- deliberately different
// from the production spin and from the smooth rainbow that follows.
void flashTestModeSignal() {
  for (uint8_t i = 0; i < 4; i++) {
    ledsOuter.fill(ledsOuter.Color(255, 255, 255));
    ledsOuter.show();
    ledsInner.fill(ledsInner.Color(255, 255, 255));
    ledsInner.show();
    delay(120);
    allOff();
    delay(120);
  }
}

// Checks for a tap at most once every TAP_CHECK_INTERVAL_MS (cheap no-op
// otherwise). On a real tap, toggles testOn, plays the strobe if turning on
// or clears the strips if turning off, and returns true.
bool maybeCheckTap() {
  unsigned long now = millis();
  if (now - lastTapCheck < TAP_CHECK_INTERVAL_MS) return false;
  lastTapCheck = now;

  mfrc522.PCD_SoftPowerUp();
  delay(2);
  bool sawCard = mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
  if (sawCard) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  mfrc522.PCD_SoftPowerDown();

  if (!sawCard) return false;

  testOn = !testOn;
  if (testOn) {
    flashTestModeSignal();
  } else {
    allOff();
  }
  return true;
}

// Delays up to `ms`, checking for a tap periodically. Returns false (and
// stops early) the moment a tap toggles state during the hold.
bool holdWithTapCheck(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (maybeCheckTap()) return false;
    delay(20);
  }
  return true;
}

uint32_t wheelOuter(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85)  return ledsOuter.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) { pos -= 85; return ledsOuter.Color(0, pos * 3, 255 - pos * 3); }
  pos -= 170;    return ledsOuter.Color(pos * 3, 255 - pos * 3, 0);
}

uint32_t wheelInner(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85)  return ledsInner.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) { pos -= 85; return ledsInner.Color(0, pos * 3, 255 - pos * 3); }
  pos -= 170;    return ledsInner.Color(pos * 3, 255 - pos * 3, 0);
}

// Both strips cycle the rainbow wheel together, inner ring 128 steps
// (half the wheel -- roughly complementary hue) ahead of outer.
void rainbowComplementary() {
  for (uint16_t j = 0; j < 256; j++) {
    if (maybeCheckTap()) return;

    for (uint16_t i = 0; i < ledsOuter.numPixels(); i++) {
      ledsOuter.setPixelColor(i, wheelOuter((i * 256 / ledsOuter.numPixels() + j) & 255));
    }
    for (uint16_t i = 0; i < ledsInner.numPixels(); i++) {
      ledsInner.setPixelColor(i, wheelInner((i * 256 / ledsInner.numPixels() + j + 128) & 255));
    }
    ledsOuter.show();
    ledsInner.show();
    delay(RAINBOW_STEP_MS);
  }
}

void solidPairHold(uint8_t ro, uint8_t go, uint8_t bo, uint8_t ri, uint8_t gi, uint8_t bi) {
  if (!testOn) return;
  ledsOuter.fill(ledsOuter.Color(ro, go, bo));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(ri, gi, bi));
  ledsInner.show();
  holdWithTapCheck(SOLID_HOLD_MS);
}

// Outer chases clockwise, inner chases counter-clockwise, same shared
// color -- the two rings move against each other, not mirrored.
void chaseComplementary(uint8_t r, uint8_t g, uint8_t b) {
  if (!testOn) return;
  uint16_t outerCount = ledsOuter.numPixels();
  uint16_t innerCount = ledsInner.numPixels();
  uint16_t steps = ((outerCount > innerCount) ? outerCount : innerCount) * 2;  // a couple of full laps
  int prevOuter = -1, prevInner = -1;

  for (uint16_t step = 0; step < steps; step++) {
    if (maybeCheckTap()) return;

    int iOuter = step % outerCount;
    int iInner = (innerCount - 1) - (step % innerCount);  // opposite direction

    if (prevOuter >= 0) ledsOuter.setPixelColor(prevOuter, 0);
    ledsOuter.setPixelColor(iOuter, ledsOuter.Color(r, g, b));
    if (prevInner >= 0) ledsInner.setPixelColor(prevInner, 0);
    ledsInner.setPixelColor(iInner, ledsInner.Color(r, g, b));

    ledsOuter.show();
    ledsInner.show();
    delay(CHASE_STEP_MS);

    prevOuter = iOuter;
    prevInner = iInner;
  }
  if (testOn) allOff();
}

void bothWhiteHold() {
  if (!testOn) return;
  ledsOuter.fill(ledsOuter.Color(255, 255, 255));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(255, 255, 255));
  ledsInner.show();
  holdWithTapCheck(WHITE_HOLD_MS);
}

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  mfrc522.PCD_SoftPowerDown();

  ledsOuter.begin();
  ledsInner.begin();
  allOff();

  Serial.println(F("THERMAL TEST v3 -- do not ship. Tap to start (strobe signal), tap to stop, LED-only."));
  Serial.flush();
}

void loop() {
  if (!testOn) {
    maybeCheckTap();
    delay(30);
    return;
  }

  rainbowComplementary();
  solidPairHold(255, 0, 0,   0, 255, 255);  // outer red / inner cyan
  solidPairHold(0, 0, 255,   255, 255, 0);  // outer blue / inner yellow
  solidPairHold(0, 255, 0,   255, 0, 255);  // outer green / inner magenta
  chaseComplementary(255, 255, 255);        // white chase, opposite directions
  bothWhiteHold();                           // worst case for current draw
  if (testOn) { allOff(); delay(500); }
}
