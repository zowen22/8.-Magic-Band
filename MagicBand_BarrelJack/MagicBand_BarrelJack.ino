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

   NeoPixels:
     Outer ring -> D5  |  Inner ring -> D6
     VCC wired directly to +5V, no gating -- rings are always powered

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
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <RCSwitch.h>

// -- LED Strip Configuration ------------------------------------------------
#define OUTER_LEDS  16
#define INNER_LEDS  11

const uint8_t PIN_OUTER = 5;
const uint8_t PIN_INNER = 6;

Adafruit_NeoPixel outerRing(OUTER_LEDS, PIN_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel innerRing(INNER_LEDS, PIN_INNER, NEO_GRB + NEO_KHZ800);

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
const unsigned int  RF_PULSE_LEN = 700;  // measured ~695-704us; library default for protocol 2 is 650us
const uint8_t        RF_REPEAT   = 5;  // 433MHz is fire-and-forget, no ACK -- resend

// -- Animation -----------------------------------------------------------------
const int SPIN_SPEED = 50;  // ms per step in chase animation
const uint8_t R_IDLE = 255, G_IDLE = 255, B_IDLE = 0;  // yellow

// -- State -----------------------------------------------------------------------
bool treeOn = false;

//*******************************************************************************//

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

    delay(2500);
    ledsOff();

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

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  mfrc522.PCD_SoftPowerDown();

  rfSwitch.enableTransmit(RF_PIN);

  outerRing.begin();
  innerRing.begin();
  ledsOff();

  Serial.println(F("Ready -- polling continuously, no sleep/wake."));
  Serial.flush();
}

void loop() {
  pollCardOnce();
}
