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
   native trailing sync pulse (~7ms at protocol 2 / ~690us pulse length),
   the same gap rc-switch's own internal repeatTransmit would produce.
   No artificial gap is inserted — see project history/Technical Reference
   for why that breaks rc-switch's decode entirely (it self-calibrates its
   pulse-length unit from the gap duration).

   REPEAT_COUNT is 8, matched directly against the real remote: a
   temporarily-patched local rc-switch (decoding every repeat instead of
   requiring 2 matching gaps to confirm one) let us capture the remote's
   true raw repeat count and timing instead of the receive-side-collapsed
   count the stock library shows. Real remote, two separate presses:
   tight cluster of 6-7 repeats at a consistent 66-68ms spacing, plus one
   later straggler (98-100ms after the cluster). The straggler isn't
   reproduced here deliberately — it showed up identically in our own TX's
   raw capture despite our code doing nothing special for the last repeat,
   and its timing was wildly inconsistent (98ms/100ms for the remote,
   392ms for us) versus the tight cluster's precise, repeatable spacing —
   that's the signature of a receive-side artifact (the last repeat's
   trailing gap sits open until literally anything next disturbs the RX
   pin), not a deliberately-timed closing transmission. 8 was chosen as
   the higher of the two observed real counts, erring toward more
   reliability margin rather than less.

   No Serial I/O happens during the repeat loop itself, not even for TX
   (only RX decodes are ever printed, and only after the whole burst
   finishes) — recording to memory is cheap enough not to matter, printing
   is not, so it stays out entirely until every repeat is done.
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
const unsigned int PULSE_LEN = 690;
const uint8_t REPEAT_COUNT   = 8;

const uint8_t MAX_EVENTS = 10;
unsigned long evTs[MAX_EVENTS];
unsigned long evVal[MAX_EVENTS];
uint8_t evBits[MAX_EVENTS];
uint8_t evProto[MAX_EVENTS];
unsigned int evDelay[MAX_EVENTS];

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  rfSwitch.enableTransmit(TX_DATA_PIN);
  rfSwitch.setProtocol(PROTOCOL);
  rfSwitch.setPulseLength(PULSE_LEN);
  rfSwitch.setRepeatTransmit(1); // repeats sent manually below

  rfReceiver.enableReceive(digitalPinToInterrupt(RX_DATA_PIN));

  Serial.println(F("RF TX/RX Loopback ready."));
  Serial.println(F("Type 'o'/'f' + Enter to send ON/OFF on demand."));
}

void printEvent(uint8_t i) {
  Serial.print(evTs[i]);
  Serial.print(F("ms   RX: "));
  Serial.print(evVal[i]);
  Serial.print(F(" (0x"));
  Serial.print(evVal[i], HEX);
  Serial.print(F(") / "));
  Serial.print(evBits[i]);
  Serial.print(F(" bit / protocol "));
  Serial.print(evProto[i]);
  Serial.print(F(" / delay "));
  Serial.print(evDelay[i]);
  Serial.println(F("us"));
}

void sendRepeated(unsigned long code) {
  uint8_t n = 0;

  for (uint8_t i = 0; i < REPEAT_COUNT; i++) {
    rfSwitch.send(code, BITS); // nRepeatTransmit=1 — each call includes its own native ~7ms trailing sync

    // Single quick check, no added delay, no printing — the protocol's own
    // native sync gap (not an artificial one) is what separates repeats
    // correctly, and nothing here should cost real wall-clock time.
    if (rfReceiver.available() && n < MAX_EVENTS) {
      evTs[n]    = millis();
      evVal[n]   = rfReceiver.getReceivedValue();
      evBits[n]  = rfReceiver.getReceivedBitlength();
      evProto[n] = rfReceiver.getReceivedProtocol();
      evDelay[n] = rfReceiver.getReceivedDelay();
      n++;
      rfReceiver.resetAvailable();
    }
  }

  // All timing-sensitive work is done — safe to print now.
  for (uint8_t i = 0; i < n; i++) {
    printEvent(i);
  }
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'o') sendRepeated(CODE_ON);
    if (c == 'f') sendRepeated(CODE_OFF);
  }

  // Idle-time receive (e.g. the real remote), unrelated to our own send timing.
  if (rfReceiver.available()) {
    unsigned long value   = rfReceiver.getReceivedValue();
    unsigned int bitlen   = rfReceiver.getReceivedBitlength();
    unsigned int protocol = rfReceiver.getReceivedProtocol();
    unsigned int delay    = rfReceiver.getReceivedDelay();
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
    Serial.print(protocol);
    Serial.print(F(" / delay "));
    Serial.print(delay);
    Serial.println(F("us"));
  }
}
