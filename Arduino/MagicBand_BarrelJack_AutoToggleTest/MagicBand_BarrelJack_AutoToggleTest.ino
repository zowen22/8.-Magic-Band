/*
   Magic Band -- Barrel-Jack variant, AUTOMATIC ON/OFF CYCLE TEST build
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   No RFID, no card scan needed -- just cycles both LED strips solid
   green ON for 10s (RF_CODE_ON fires as it starts), then OFF for 10s
   (RF_CODE_OFF fires as it starts), repeating forever. For unattended
   thermal/endurance cycling of both LEDs and the RF outlet without
   needing a card tap each time. Power-cycle the board to stop/restart.

   Assumption baked in here, correct and reflash if wrong:
   - "every 10 seconds" read as: each state (on, then off) lasts 10s, so
     a full on-off cycle is 20s -- not a 10s full cycle.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   NeoPixels (two strips, same wiring as production):
     Outer ring -> D5  |  38px
     Inner ring -> D2  |  35px -- bodged directly to U1's DIP leg, see
       "Build Instructions - Barrel-Jack Variant.md" Step 5a. Both
       strips' VCC wired directly to +5V.

   RF Transmitter (433MHz, D-FLIFE ASK TX) -- same codes/timing as production:
     DATA -> D8  |  PWR IN -> external 3.3V regulator output  |  GND -> GND
*/

#include <Adafruit_NeoPixel.h>
#include <RCSwitch.h>

// -- LED Strip Configuration ------------------------------------------------
#define LED_COUNT_OUTER  38
#define LED_COUNT_INNER  35

const uint8_t PIN_LEDS_OUTER = 5;
const uint8_t PIN_LEDS_INNER = 2;  // bodged to U1's D2 DIP leg -- see Build Instructions Step 5a

Adafruit_NeoPixel ledsOuter(LED_COUNT_OUTER, PIN_LEDS_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ledsInner(LED_COUNT_INNER, PIN_LEDS_INNER, NEO_GRB + NEO_KHZ800);

// -- RF Transmitter ------------------------------------------------------------
const uint8_t RF_PIN = 8;
RCSwitch rfSwitch = RCSwitch();

const unsigned long RF_CODE_ON   = 0x885A8F00;
const unsigned long RF_CODE_OFF  = 0x845A8F00;
const unsigned int  RF_BITS      = 32;
const unsigned int  RF_PROTOCOL  = 2;
const unsigned int  RF_PULSE_LEN = 613;
const uint8_t        RF_REPEAT   = 5;

// -- Cycle timing ----------------------------------------------------------
const unsigned long CYCLE_MS = 10000UL;  // duration of EACH state (on, then off) -- not the full cycle
const uint8_t R_HOLD = 0, G_HOLD = 255, B_HOLD = 0;  // green

//*******************************************************************************//

void fireRfCode(unsigned long code) {
  rfSwitch.setProtocol(RF_PROTOCOL);
  rfSwitch.setPulseLength(RF_PULSE_LEN);
  rfSwitch.setRepeatTransmit(RF_REPEAT);
  rfSwitch.send(code, RF_BITS);
}

void allOff() {
  ledsOuter.fill(0);
  ledsOuter.show();
  ledsInner.fill(0);
  ledsInner.show();
}

void allGreen() {
  ledsOuter.fill(ledsOuter.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsOuter.show();
  ledsInner.fill(ledsInner.Color(R_HOLD, G_HOLD, B_HOLD));
  ledsInner.show();
}

void setup() {
  Serial.begin(9600);
  rfSwitch.enableTransmit(RF_PIN);
  ledsOuter.begin();
  ledsInner.begin();
  allOff();
  Serial.println(F("AUTO ON/OFF CYCLE TEST -- do not ship. 10s green+RF-on / 10s off+RF-off, repeating, no scan needed."));
  Serial.flush();
}

void loop() {
  fireRfCode(RF_CODE_ON);
  allGreen();
  delay(CYCLE_MS);
  fireRfCode(RF_CODE_OFF);
  allOff();
  delay(CYCLE_MS);
}
