/*
   Magic Band -- Barrel-Jack (Wall-Powered) variant, THERMAL SOAK TEST build
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   Identical to Arduino/MagicBand_BarrelJack/MagicBand_BarrelJack.ino except
   THERMAL_TEST_MODE is hardcoded to 1 below -- on the first card tap, both
   LED strips hold solid green INDEFINITELY and the sketch deliberately never
   returns (card polling stops too). Intended purely to check LED/driver
   temperatures under sustained load. Power-cycle the board to reset/retest.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production. This file exists only so a thermal test doesn't require
   hand-editing (and remembering to revert) the production sketch.

   -- Pin Map (matches PCB/MagicBand_BarrelJack/MagicBand_BarrelJack.kicad_sch) --
   MFRC522 (SPI):
     RST -> D9  |  SS -> D4  |  MOSI -> D11  |  MISO -> D12  |  SCK -> D13
     VCC -> external 3.3V regulator output (MCP1700-3302)
     IRQ intentionally unconnected -- not used, PICC_IsNewCardPresent() is polled

   NeoPixels (two strips as of 2026-10-13):
     Outer ring -> D5  |  38px, denser/smaller LEDs than the original 16px strip
     Inner ring -> D2  |  35px, NEW -- bodged directly to U1's DIP leg (no
       designed connector exists on rev 1; see "Build Instructions - Barrel-Jack
       Variant.md" Step 5a for the physical solder points). Rev 2 should give
       this its own connector -- see Work Packages WP4.3.
     Both strips' VCC wired directly to +5V, no gating -- always powered

   RF Transmitter (433MHz, D-FLIFE ASK TX):
     DATA -> D8  |  PWR IN -> external 3.3V regulator output (same rail as
     MFRC522 above -- this module is 3.3V-only, NOT the 5V bus)  |
     GND -> GND  |  short antenna
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <RCSwitch.h>
#include <math.h>

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

// -- RF Transmitter ------------------------------------------------------------
const uint8_t RF_PIN = 8;
RCSwitch rfSwitch = RCSwitch();

const unsigned long RF_CODE_ON   = 0x885A8F00;
const unsigned long RF_CODE_OFF  = 0x845A8F00;
const unsigned int  RF_BITS      = 32;
const unsigned int  RF_PROTOCOL  = 2;
const unsigned int  RF_PULSE_LEN = 613;
const uint8_t        RF_REPEAT   = 5;

// -- Thermal test mode ---------------------------------------------------------
// Hardcoded on for this build -- see file header. Do not flip to 0 here; use
// the real MagicBand_BarrelJack.ino for production instead.
#define THERMAL_TEST_MODE 1

const unsigned long HOLD_MS = 15000UL;  // unused in this build

// -- Animation -----------------------------------------------------------------
const uint8_t  SPIN_LAPS = 3;
const float    SPIN_DELAY_START_MS = 60.0;
const float    SPIN_DELAY_END_MS   = 5.0;
const uint8_t  R_SPIN = 255, G_SPIN = 255, B_SPIN = 255;  // white
const uint8_t  R_HOLD = 0,   G_HOLD = 255, B_HOLD = 0;    // green

// -- State -----------------------------------------------------------------------
bool treeOn = false;

//*******************************************************************************//

void allOff() {
  ledsOuter.fill(0);
  ledsOuter.show();
  ledsInner.fill(0);
  ledsInner.show();
}

void spinAnimationOuter() {
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

bool pollCardOnce() {
  mfrc522.PCD_SoftPowerUp();
  delay(2);

  bool sawCard = mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();

  if (sawCard) {
    spinAnimationOuter();

    treeOn = !treeOn;
    fireRfCode(treeOn ? RF_CODE_ON : RF_CODE_OFF);

    holdGreenBoth();

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

#if THERMAL_TEST_MODE
    mfrc522.PCD_SoftPowerDown();
    // Deliberate dead-end -- see file header. Power-cycle to reset/retest.
    while (true) { delay(1000); }
#else
    delay(HOLD_MS);
    allOff();
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
  ledsInner.begin();
  allOff();

  Serial.println(F("THERMAL SOAK TEST BUILD -- do not ship. Polling for the one tap that starts the soak."));
  Serial.flush();
}

void loop() {
  pollCardOnce();
  delay(50);
}
