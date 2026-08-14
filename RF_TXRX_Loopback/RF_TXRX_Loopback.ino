/*
   RF TX/RX Loopback — TX and RX modules on the same board, so a
   received code confirms the TX is actually radiating, independent
   of whether the paired outlet responds.

   ── Wiring ───────────────────────────────────────────────────────────────
     TX: Antenna → short spring antenna
         GND     → GND
         PWR IN  → 3.3V (NOT 5V)
         DATA    → D8
         EN      → leave disconnected
     RX: Antenna → long spring antenna
         GND     → GND
         VIN     → 5V
         DATA    → D2 (interrupt-capable)

   ── Usage ────────────────────────────────────────────────────────────────
   Type 'o'/'f' + Enter to send ON/OFF. Repeats are sent as REPEAT_COUNT
   separate rfSwitch.send() calls (nRepeatTransmit=1 each) with NO added
   delay between them — the gap between repeats is just each call's own
   native trailing sync pulse (~7ms at protocol 2 / 700us pulse length),
   the same gap rc-switch's own internal repeatTransmit would produce.

   Two earlier attempts at manual spacing both failed for different reasons,
   both now understood:
   1. A ~228ms delay-loop gap (with Serial.print inside it) got zero
      decodes because the print itself could block for real time once the
      UART buffer filled, and that time landed inside the window rc-switch
      uses to self-calibrate its pulse-length unit.
   2. Removing the print but keeping a manually-sized ~133ms gap *still*
      got zero decodes — because 133ms is ~19x longer than protocol 2's
      real ~7ms sync period. receiveProtocol() derives its assumed pulse
      unit as `timings[0] / syncLengthInPulses` — feeding it a 133ms gap
      instead of ~7ms makes it calibrate against completely the wrong
      unit, so every subsequent data-bit comparison fails. This isn't a
      timing-precision problem, it's a wrong-scale-gap problem — no amount
      of software timing precision fixes an artificial gap that doesn't
      match the protocol's own sync period.

   This version sends each repeat separately (so our own polling isn't
   blocked inside one big blocking send() call the way setRepeatTransmit(5)
   would) but doesn't add any extra delay, so the gap stays at the correct
   native scale. Every TX send and RX decode is recorded to memory with a
   timestamp and printed only after all repeats are done, so Serial I/O
   never lands inside a timing-sensitive window.
*/

#include <RCSwitch.h>

const uint8_t TX_DATA_PIN = 8;
const uint8_t RX_DATA_PIN = 2;

RCSwitch rfSwitch   = RCSwitch();
RCSwitch rfReceiver = RCSwitch();

unsigned long CODE_ON  = 0x885A8F00; // 2287636224
unsigned long CODE_OFF = 0x845A8F00; // 2220527360
const unsigned int BITS      = 32;
const unsigned int PROTOCOL  = 2;
const unsigned int PULSE_LEN = 700;
const uint8_t REPEAT_COUNT   = 5;

const uint8_t MAX_EVENTS = 20;
unsigned long evTs[MAX_EVENTS];
unsigned long evVal[MAX_EVENTS];
uint8_t evBits[MAX_EVENTS];
uint8_t evProto[MAX_EVENTS];
bool evIsTx[MAX_EVENTS];

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  rfSwitch.enableTransmit(TX_DATA_PIN);
  rfSwitch.setProtocol(PROTOCOL);
  rfSwitch.setPulseLength(PULSE_LEN);
  rfSwitch.setRepeatTransmit(1); // repeats sent manually below, spaced out

  rfReceiver.enableReceive(digitalPinToInterrupt(RX_DATA_PIN));

  Serial.println(F("RF TX/RX Loopback ready."));
  Serial.println(F("Type 'o'/'f' + Enter to send ON/OFF on demand."));
}

void printEvent(uint8_t i, const __FlashStringHelper *label) {
  Serial.print(evTs[i]);
  if (evIsTx[i]) {
    Serial.print(F("ms TX ("));
    Serial.print(label);
    Serial.print(F("): "));
  } else {
    Serial.print(F("ms   RX: "));
  }
  Serial.print(evVal[i]);
  Serial.print(F(" (0x"));
  Serial.print(evVal[i], HEX);
  Serial.print(F(") / "));
  Serial.print(evBits[i]);
  Serial.print(F(" bit / protocol "));
  Serial.println(evProto[i]);
}

void sendRepeated(unsigned long code, const __FlashStringHelper *label) {
  uint8_t n = 0;

  for (uint8_t i = 0; i < REPEAT_COUNT && n < MAX_EVENTS; i++) {
    rfSwitch.send(code, BITS); // nRepeatTransmit=1 — each call includes its own native ~7ms trailing sync

    evTs[n]    = millis();
    evVal[n]   = code;
    evBits[n]  = BITS;
    evProto[n] = PROTOCOL;
    evIsTx[n]  = true;
    n++;

    // Single quick check, no added delay — the protocol's own native sync
    // gap (not an artificial one) is what separates repeats correctly.
    if (rfReceiver.available() && n < MAX_EVENTS) {
      evTs[n]    = millis();
      evVal[n]   = rfReceiver.getReceivedValue();
      evBits[n]  = rfReceiver.getReceivedBitlength();
      evProto[n] = rfReceiver.getReceivedProtocol();
      evIsTx[n]  = false;
      n++;
      rfReceiver.resetAvailable();
    }
  }

  // All timing-sensitive work is done — safe to print now.
  for (uint8_t i = 0; i < n; i++) {
    printEvent(i, label);
  }
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'o') sendRepeated(CODE_ON, F("ON"));
    if (c == 'f') sendRepeated(CODE_OFF, F("OFF"));
  }

  // Idle-time receive (e.g. the real remote), unrelated to our own send timing.
  if (rfReceiver.available()) {
    unsigned long value   = rfReceiver.getReceivedValue();
    unsigned int bitlen   = rfReceiver.getReceivedBitlength();
    unsigned int protocol = rfReceiver.getReceivedProtocol();
    unsigned long now     = millis();
    rfReceiver.resetAvailable();

    Serial.print(now);
    Serial.print(F("ms   RX: "));
    Serial.print(value);
    Serial.print(F(" (0x"));
    Serial.print(value, HEX);
    Serial.print(F(") / "));
    Serial.print(bitlen);
    Serial.print(F(" bit / protocol "));
    Serial.println(protocol);
  }
}
