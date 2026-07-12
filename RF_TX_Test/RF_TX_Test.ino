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
   Default: sends a placeholder test code every 3s so you can confirm with
   a second board running RF_Sniffer that data is actually going out over
   the air (range, correct pin, module not dead, etc.) — this is NOT the
   real outlet code.

   To test against the actual paired outlet: capture the real ON/OFF codes
   with RF_Sniffer first, fill them in below, then either send over Serial
   ("o" for on, "f" for off) or let it auto-repeat.
*/

#include <RCSwitch.h>

const uint8_t TX_DATA_PIN = 8;

RCSwitch rfSwitch = RCSwitch();

// Placeholder — replace with real captured codes from RF_Sniffer once known
unsigned long CODE_ON  = 5393; // 0x1511, RCSwitch example default — NOT a real outlet code
unsigned long CODE_OFF = 5396; // 0x1514, RCSwitch example default — NOT a real outlet code
const unsigned int BITS     = 24;
const unsigned int PROTOCOL = 1;
const uint8_t REPEAT_COUNT  = 5;  // no ACK on 433MHz — resend a few times

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
    sendCode(CODE_ON, F("placeholder"));
  }
}
