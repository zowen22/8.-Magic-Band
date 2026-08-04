/*
   RF TX Test — sends a 433MHz code on demand, to bench-verify the
   transmitter module works before wiring it into MagicBand_Wireless.
   Target: any 5V Uno/Nano/Pro Mini. TX module itself is 3.3V — see
   1. Project Management/RF Module Pinout - D-FLIFE 433MHz Kit.md

   ── Wiring (D-FLIFE TX module) ────────────────────────────────────────────
     Antenna → short spring antenna
     GND     → GND
     PWR IN  → 3.3V (NOT 5V — this board's 3V3 pin, or a Pro Mini's 3V3 pin)
     DATA    → D8
     EN      → leave disconnected (listing: tied internally, no function)

   ── Usage ─────────────────────────────────────────────────────────────────
   CODE_ON/CODE_OFF below are real, captured codes for the outlet the tree
   lights are plugged into (top row on the remote, verified 2026-08-03).
   Auto-repeats CODE_ON every 3s by default; send over Serial instead
   ("o" for on, "f" for off) to test both directions.
*/

#include <RCSwitch.h>

const uint8_t TX_DATA_PIN = 8;

RCSwitch rfSwitch = RCSwitch();

// Real codes captured via RF_Sniffer, verified against the tree-lights outlet (top row)
unsigned long CODE_ON  = 0x885A8F00; // 2287636224
unsigned long CODE_OFF = 0x845A8F00; // 2220527360
const unsigned int BITS      = 32;
const unsigned int PROTOCOL  = 2;
const unsigned int PULSE_LEN = 700;  // measured ~695-704us; protocol 2 library default is 650us
const uint8_t REPEAT_COUNT   = 5;  // no ACK on 433MHz — resend a few times

void sendCode(unsigned long code, const __FlashStringHelper *label) {
  rfSwitch.send(code, BITS);
  Serial.print(F("Sent "));
  Serial.print(label);
  Serial.print(F(": "));
  Serial.println(code);
}

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  rfSwitch.enableTransmit(TX_DATA_PIN);
  rfSwitch.setProtocol(PROTOCOL);
  rfSwitch.setPulseLength(PULSE_LEN);
  rfSwitch.setRepeatTransmit(REPEAT_COUNT);

  Serial.println(F("RF TX Test ready."));
  Serial.println(F("Type 'o' + Enter to send ON, 'f' + Enter to send OFF."));
  Serial.println(F("No input needed — auto-sends a placeholder code every 3s so a"));
  Serial.println(F("second board running RF_Sniffer can confirm the module is alive."));
}

unsigned long lastAutoSend = 0;
const unsigned long AUTO_SEND_INTERVAL_MS = 3000;

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'o') sendCode(CODE_ON, F("ON"));
    if (c == 'f') sendCode(CODE_OFF, F("OFF"));
  }

  if (millis() - lastAutoSend >= AUTO_SEND_INTERVAL_MS) {
    lastAutoSend = millis();
    sendCode(CODE_ON, F("auto ON"));
  }
}
