/*
   Magic Band -- Barrel-Jack variant, TX TOGGLE TEST build
   Target: bare ATmega328P-P, 5V/16MHz -- see PCB/MagicBand_BarrelJack/

   No RFID, no LEDs, no card scan needed -- fires RF_CODE_ON, waits 1
   second, fires RF_CODE_OFF, waits 1 second, repeating forever. For
   bench-verifying the TX module/outlet pairing on its own, independent
   of the RFID/LED side of the board. Power-cycle to restart.

   DO NOT ship this build -- flash the real MagicBand_BarrelJack.ino for
   production.

   RF Transmitter (433MHz, D-FLIFE ASK TX) -- same codes/timing as production:
     DATA -> D8  |  PWR IN -> external 3.3V regulator output (3.3V-only,
     NOT the 5V bus)  |  GND -> GND  |  short antenna
*/

#include <RCSwitch.h>

const uint8_t RF_PIN = 8;
RCSwitch rfSwitch = RCSwitch();

const unsigned long RF_CODE_ON   = 0x885A8F00;
const unsigned long RF_CODE_OFF  = 0x845A8F00;
const unsigned int  RF_BITS      = 32;
const unsigned int  RF_PROTOCOL  = 2;
const unsigned int  RF_PULSE_LEN = 613;
const uint8_t        RF_REPEAT   = 5;

const unsigned long TOGGLE_MS = 1000UL;  // once per second

//*******************************************************************************//

void fireRfCode(unsigned long code) {
  rfSwitch.setProtocol(RF_PROTOCOL);
  rfSwitch.setPulseLength(RF_PULSE_LEN);
  rfSwitch.setRepeatTransmit(RF_REPEAT);
  rfSwitch.send(code, RF_BITS);
}

void setup() {
  Serial.begin(9600);
  rfSwitch.enableTransmit(RF_PIN);
  Serial.println(F("TX TOGGLE TEST -- do not ship. Firing ON/OFF once per second, no scan needed."));
  Serial.flush();
}

void loop() {
  fireRfCode(RF_CODE_ON);
  Serial.println(F("ON"));
  delay(TOGGLE_MS);

  fireRfCode(RF_CODE_OFF);
  Serial.println(F("OFF"));
  delay(TOGGLE_MS);
}
