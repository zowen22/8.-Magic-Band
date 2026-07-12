/*
   RF Sniffer — captures 433MHz codes from a paired outlet remote
   Bench tool only, per Technical Reference "RF Workflow" step 1.
   Target: any 5V Uno/Nano/Pro Mini — RX module needs 5V, not 3.3V
   (see 1. Project Management/RF Module Pinout - D-FLIFE 433MHz Kit.md)

   ── Wiring (D-FLIFE RX module) ────────────────────────────────────────────
     Antenna → long spring antenna
     GND     → GND
     VIN     → 5V
     DATA    → D2 (must be an interrupt-capable pin; D2/D3 on this MCU)
     (module has two DATA pins, both carry the same signal — wire either one)

   ── Usage ─────────────────────────────────────────────────────────────────
   1. Flash this sketch, open Serial Monitor at 9600 baud
   2. Press each button on the remote (ON, OFF, etc.) one at a time
   3. Record the printed value/bitlength/protocol/pulse-length for each
   4. Transfer RF_CODE_ON / RF_CODE_OFF / RF_BITS / RF_PROTOCOL into
      MagicBand_Wireless.ino (and set pulse length there via
      rfSwitch.setPulseLength() if it's non-default)
*/

#include <RCSwitch.h>

const uint8_t RX_DATA_PIN = 2;

RCSwitch rfReceiver = RCSwitch();

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  rfReceiver.enableReceive(digitalPinToInterrupt(RX_DATA_PIN));

  Serial.println(F("RF Sniffer ready — press buttons on the remote."));
}

void loop() {
  if (!rfReceiver.available()) return;

  unsigned long value   = rfReceiver.getReceivedValue();
  unsigned int bitlen   = rfReceiver.getReceivedBitlength();
  unsigned int protocol = rfReceiver.getReceivedProtocol();
  unsigned int pulseLen = rfReceiver.getReceivedDelay();

  if (value == 0) {
    Serial.println(F("Unknown encoding — received data but couldn't decode a protocol."));
  } else {
    Serial.print(F("Code: "));
    Serial.print(value);
    Serial.print(F(" (0x"));
    Serial.print(value, HEX);
    Serial.print(F(") / "));
    Serial.print(bitlen);
    Serial.print(F(" bit / protocol "));
    Serial.print(protocol);
    Serial.print(F(" / pulse "));
    Serial.print(pulseLen);
    Serial.println(F("us"));
  }

  rfReceiver.resetAvailable();
}
