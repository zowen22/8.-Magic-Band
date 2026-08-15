/*
   RF Pulse-Length Sweep — bench tool to test whether the outlet responds
   at some pulse length other than our best-measured value, in case our
   700us/PULSE_LEN=690 tuning (matched closely against the real remote's
   directly-measured ~696-702us) is still slightly off from whatever the
   outlet's own receiver actually needs.

   Sweeps PULSE_LEN from -20% to +20% of BASE_PULSE_LEN in 5% steps (9
   values total), sending REPEAT_COUNT repeats of CODE_ON at each one,
   with a pause in between so you can watch the outlet before the next
   value fires. Prints which pulse length is active before each burst.

   ── Wiring (TX only) ───────────────────────────────────────────────────
     Antenna → short spring antenna
     GND     → GND
     PWR IN  → 3.3V (NOT 5V)
     DATA    → D8
     EN      → leave disconnected

   ── Usage ────────────────────────────────────────────────────────────────
   Type 's' + Enter to start the sweep. It runs all 9 values once, then
   stops (type 's' again to re-run). Type 'o'/'f' + Enter at any time for
   a single manual ON/OFF at BASE_PULSE_LEN, same as RF_TXRX_Loopback.
*/

#include <RCSwitch.h>

const uint8_t TX_DATA_PIN = 8;

RCSwitch rfSwitch = RCSwitch();

unsigned long CODE_ON  = 0x885A8F00; // 2287636224
unsigned long CODE_OFF = 0x845A8F00; // 2220527360
const unsigned int BITS      = 32;
const unsigned int PROTOCOL  = 2;
const uint8_t REPEAT_COUNT   = 8;

const unsigned int BASE_PULSE_LEN = 700;
const unsigned long STEP_DELAY_MS = 3000; // pause between values so you can watch the outlet

// -20% to +20% in 5% steps: 560, 595, 630, 665, 700, 735, 770, 805, 840
const int8_t SWEEP_PERCENTS[] = { -20, -15, -10, -5, 0, 5, 10, 15, 20 };
const uint8_t SWEEP_COUNT = sizeof(SWEEP_PERCENTS) / sizeof(SWEEP_PERCENTS[0]);

void sendAt(unsigned int pulseLen, unsigned long code) {
  rfSwitch.setPulseLength(pulseLen);
  for (uint8_t i = 0; i < REPEAT_COUNT; i++) {
    rfSwitch.send(code, BITS);
  }
}

void runSweep() {
  Serial.println(F("Sweep starting."));
  for (uint8_t i = 0; i < SWEEP_COUNT; i++) {
    unsigned int pulseLen = (unsigned int)((long)BASE_PULSE_LEN * (100 + SWEEP_PERCENTS[i]) / 100);

    Serial.print(F("["));
    Serial.print(i + 1);
    Serial.print(F("/"));
    Serial.print(SWEEP_COUNT);
    Serial.print(F("] pulse length "));
    Serial.print(pulseLen);
    Serial.print(F("us ("));
    if (SWEEP_PERCENTS[i] >= 0) Serial.print(F("+"));
    Serial.print(SWEEP_PERCENTS[i]);
    Serial.println(F("%) — sending ON, watch the outlet"));

    sendAt(pulseLen, CODE_ON);
    delay(STEP_DELAY_MS);
  }
  rfSwitch.setPulseLength(BASE_PULSE_LEN); // restore baseline for manual o/f afterward
  Serial.println(F("Sweep complete. If nothing responded, pulse length alone likely isn't the fix."));
}

void setup() {
  Serial.begin(9600);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  rfSwitch.enableTransmit(TX_DATA_PIN);
  rfSwitch.setProtocol(PROTOCOL);
  rfSwitch.setPulseLength(BASE_PULSE_LEN);
  rfSwitch.setRepeatTransmit(1); // send repeats as separate calls, no forced gap

  Serial.println(F("RF Pulse Sweep ready."));
  Serial.println(F("Type 's' + Enter to run the sweep, or 'o'/'f' + Enter for a manual single test at baseline."));
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 's') runSweep();
    if (c == 'o') sendAt(BASE_PULSE_LEN, CODE_ON);
    if (c == 'f') sendAt(BASE_PULSE_LEN, CODE_OFF);
  }
}
