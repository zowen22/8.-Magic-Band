/*
   Magic Band — Wireless (battery) variant
   Target: Arduino Pro Mini, 5V/16MHz (ATmega328P)
   See: 1. Project Management/BOM - Wireless Ornament Variant.md

   Power architecture ("properly engineered" scenario in the BOM):
     - Pro Mini spends nearly all its time in SLEEP_MODE_PWR_DOWN
     - Watchdog timer wakes the CPU ~1x/second
     - MFRC522 is soft-powered-up, polled once, soft-powered-down again
     - NeoPixel VCC is cut via a GPIO-gated MOSFET (Adafruit #5648) whenever
       not animating — NeoPixels draw ~0.6mA/pixel even "off" if VCC stays
       connected, which otherwise dominates the sleep-current budget
     - 433MHz TX only draws current while actively sending

   ── Pin Map ─────────────────────────────────────────────────────────────
   MFRC522 (SPI):
     RST → D9  |  SS → D4  |  MOSI → D11  |  MISO → D12  |  SCK → D13
     VCC → external 3.3V regulator output (see below — NOT a Pro Mini pin)
     (SS on D4 per Technical Reference's documented pin map, not D10 —
      see Session Log 2026-07-12 for the doc/code mismatch this resolves)

   NeoPixels:
     Outer ring → D5  |  Inner ring → D6
     VCC gate (MOSFET driver input) → D7 — HIGH powers the rings, LOW cuts them

   RF Transmitter (433MHz, D-FLIFE ASK TX — see 1. Project Management/
   RF Module Pinout - D-FLIFE 433MHz Kit.md):
     DATA → D8  |  PWR IN → external 3.3V regulator output (same rail as
     MFRC522 above — this module is 3.3V-only, NOT the 5V bus)  |
     GND → GND  |  short antenna

   3.3V Regulator (MCP1700-3302, NOT AMS1117 — its ~mA-range quiescent
     current would dominate sleep draw; MCP1700 is 1.6uA. See BOM):
     Pro Mini has only ONE onboard regulator (5V) — unlike the Nano, it
     has no separate 3.3V pin. IN → Pro Mini VCC (5V)  |
     OUT → MFRC522 VCC + TX module PWR IN, above  |  GND → common ground

   DFPlayer Mini (optional — NOT part of the baseline wireless BOM/power
   budget; compiled out unless ENABLE_AUDIO is defined):
     D2 → DFPlayer RX  |  D3 ← DFPlayer TX

   RF codes below were captured via RF_Sniffer against the paired outlet
   remote's top-row ON/OFF buttons — verified against the physical outlet
   the tree lights are plugged into (2026-08-03). Confirmed repeatable
   across multiple presses each, not a rolling code.
*/

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <RCSwitch.h>

#ifdef ENABLE_AUDIO
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#endif

// ── LED Strip Configuration ────────────────────────────────────────────────
#define OUTER_LEDS  16
#define INNER_LEDS  11

const uint8_t PIN_OUTER = 5;
const uint8_t PIN_INNER = 6;
const uint8_t PIN_PIXEL_GATE = 7;   // MOSFET driver input — HIGH = rings powered

Adafruit_NeoPixel outerRing(OUTER_LEDS, PIN_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel innerRing(INNER_LEDS, PIN_INNER, NEO_GRB + NEO_KHZ800);

// ── RFID ──────────────────────────────────────────────────────────────────
const uint8_t RST_PIN = 9;
const uint8_t SS_PIN  = 4;
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ── RF Transmitter ────────────────────────────────────────────────────────
const uint8_t RF_PIN = 8;
RCSwitch rfSwitch = RCSwitch();

const unsigned long RF_CODE_ON   = 0x885A8F00;
const unsigned long RF_CODE_OFF  = 0x845A8F00;
const unsigned int  RF_BITS      = 32;
const unsigned int  RF_PROTOCOL  = 2;
const unsigned int  RF_PULSE_LEN = 613;  // confirmed against the real outlet 2026-08-15 (RF_PulseSweep bench tool) —
                                          // 700us decoded fine on our own bench RX but did NOT trigger the outlet;
                                          // the outlet's receiver needed a notably shorter pulse (560-665us worked)
const uint8_t        RF_REPEAT   = 5;  // 433MHz is fire-and-forget, no ACK — resend

// ── DFPlayer Mini (optional) ─────────────────────────────────────────────
#ifdef ENABLE_AUDIO
const uint8_t PIN_MP3_TX = 2;
const uint8_t PIN_MP3_RX = 3;
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;
bool playerReady = false;
#endif

// ── Animation ─────────────────────────────────────────────────────────────
const int SPIN_SPEED = 50;  // ms per step in chase animation
const uint8_t R_IDLE = 255, G_IDLE = 255, B_IDLE = 0;  // yellow

// ── State ─────────────────────────────────────────────────────────────────
bool treeOn = false;  // survives sleep — SRAM is retained in power-down sleep

// ── Watchdog / sleep ──────────────────────────────────────────────────────
volatile bool wdtWake = false;

ISR(WDT_vect) {
  wdtWake = true;
}

// WDIE-only (no WDE) — fires an interrupt, not a reset. ~1s timeout (WDP2|WDP1).
void setupWatchdog1Hz() {
  cli();
  MCUSR &= ~(1 << WDRF);
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDIE) | (1 << WDP2) | (1 << WDP1);
  sei();
}

void sleepUntilNextPoll() {
  wdtWake = false;
  setupWatchdog1Hz();  // WDIE self-clears after firing; rearm every cycle
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  sleep_enable();
  sleep_bod_disable();  // saves ~20-25uA; must be set with interrupts off, right before sleep
  interrupts();
  sleep_cpu();
  sleep_disable();
}

//*******************************************************************************//

void setPixelPower(bool on) {
  digitalWrite(PIN_PIXEL_GATE, on ? HIGH : LOW);
  if (on) delay(2);  // let rail settle before driving data
}

void ledsOff() {
  outerRing.fill(0);
  outerRing.show();
  innerRing.fill(0);
  innerRing.show();
}

void spinAnimation() {
  for (int i = innerRing.numPixels() - 1; i >= 0; i--) {
    if (i < innerRing.numPixels() - 1)
      innerRing.setPixelColor(i + 1, 0);
    innerRing.setPixelColor(i, innerRing.Color(R_IDLE, G_IDLE, B_IDLE));
    innerRing.show();
    delay(SPIN_SPEED);
  }
  innerRing.setPixelColor(0, 0);
  innerRing.show();
  delay(SPIN_SPEED);
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
  mfrc522.PCD_SoftPowerUp();
  delay(2);  // datasheet-recommended settle time after power-up

  bool sawCard = mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();

  if (sawCard) {
    setPixelPower(true);
    outerRing.begin();
    innerRing.begin();

    spinAnimation();
    spinAnimation();
    spinAnimation();

    treeOn = !treeOn;
    fireRfCode(treeOn ? RF_CODE_ON : RF_CODE_OFF);

    if (treeOn) {
      for (int i = 0; i < outerRing.numPixels(); i++)
        outerRing.setPixelColor(i, outerRing.Color(R_IDLE, G_IDLE, B_IDLE));
      outerRing.show();
    } else {
      ledsOff();
    }

#ifdef ENABLE_AUDIO
    if (playerReady) player.play(1);
    delay(2500);
#endif

    ledsOff();
    setPixelPower(false);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  mfrc522.PCD_SoftPowerDown();
  return sawCard;
}

//*******************************************************************************//

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  power_adc_disable();  // ADC unused — saves current in both active and sleep

  pinMode(PIN_PIXEL_GATE, OUTPUT);
  digitalWrite(PIN_PIXEL_GATE, LOW);  // rings off until first animation

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  mfrc522.PCD_SoftPowerDown();

  rfSwitch.enableTransmit(RF_PIN);

#ifdef ENABLE_AUDIO
  softwareSerial.begin(9600);
  if (player.begin(softwareSerial)) {
    player.volume(20);
    playerReady = true;
    Serial.println(F("DFPlayer ready"));
  } else {
    Serial.println(F("DFPlayer not found — check wiring on D2/D3"));
  }
#endif

  Serial.println(F("Ready — entering sleep/wake poll loop."));
  Serial.flush();
}

void loop() {
  sleepUntilNextPoll();
  if (!wdtWake) return;  // spurious wake guard

  pollCardOnce();
}
