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

   -- Animation sequence, as of 2026-10-13, timing fixed 2026-10-18 --
   On card tap: outer ring only spins a single white point 3 laps, speeding
   up exponentially over the sequence. Right after the spin, the MFRC522 is
   fully powered down BEFORE the RF outlet toggle fires (fixed 2026-10-18 --
   firing RF while the reader was still powered, sharing the same 3.3V
   rail, was the leading explanation for RF not reliably triggering the
   outlet in production despite a standalone TX-only test sketch working
   fine on the same board). Same alternating on/off RF logic as always.
   Both strips then hold solid green for HOLD_MS -- but card-scanning
   reopens just 1 second after RF fires, not after the full hold (fixed
   2026-10-18, see scanLockoutUntil/holdActive). THERMAL_TEST_MODE below
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

// -- Animation -----------------------------------------------------------------
const uint8_t  SPIN_LAPS = 3;
const float    SPIN_DELAY_START_MS = 60.0;  // delay per step at the start of the spin
const float    SPIN_DELAY_END_MS   = 5.0;   // delay per step at the end -- exponential ramp between the two
const uint8_t  R_SPIN = 255, G_SPIN = 255, B_SPIN = 255;  // white
const uint8_t  R_HOLD = 0,   G_HOLD = 255, B_HOLD = 0;    // green

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

// Single white point, outer ring only, SPIN_LAPS full laps, speeding up
// exponentially from SPIN_DELAY_START_MS down to SPIN_DELAY_END_MS across the
// whole sequence (not reset per lap).
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
    spinAnimationOuter();  // outer ring only, white, 3 laps, exponential speed-up

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

    // RF toggle fires exactly as the solid hold begins -- same alternating
    // on/off logic as always, just re-timed to this new sequence.
    treeOn = !treeOn;
    fireRfCode(treeOn ? RF_CODE_ON : RF_CODE_OFF);

    holdGreenBoth();  // both strips, solid green

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
