/*
   RF Pulse-Length Sweep — bench tool to test whether the outlet responds
   at some pulse length other than our best-measured value, in case our
   700us/PULSE_LEN=690 tuning (matched closely against the real remote's
   directly-measured ~696-702us) is still slightly off from whatever the
   outlet's own receiver actually needs.

   RESULT (2026-08-15): it was. Steps 1-4 (560-665us) triggered the outlet
   consistently; step 5 (700us, our previously "confirmed" value) did not.
   613us (midpoint of the working range) confirmed working directly.
   700us decoded fine on our own bench RX the whole session because
   rc-switch's default 60% receive tolerance is far more forgiving than
   the outlet's actual receiver chip. PULSE_LEN=613 is now the value used
   in RF_TXRX_Loopback.ino, RF_TX_Test.ino, MagicBand_Wireless.ino, and
   MagicBand_BarrelJack.ino. This sketch is kept as-is (still sweeping
   around 700) for historical/reproducibility reasons, not because 700 is
   still believed correct.

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
   Type '1' through '9' + Enter to fire ONE specific step on demand (see
   the table below) so you can test them individually and report back
   which one(s) work, instead of watching a timed auto-sweep. 's' + Enter
   still runs all 9 automatically if you want that instead. 'o'/'f' +
   Enter for a manual single ON/OFF at BASE_PULSE_LEN.

     1 = -20% (560us)   4 = -5%  (665us)   7 = +10% (770us)
     2 = -15% (595us)   5 =  0%  (700us)   8 = +15% (805us)
     3 = -10% (630us)   6 = +5%  (735us)   9 = +20% (840us)
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
  rfSwitch.setPulseLength(BASE_PULSE_LEN); // restore baseline afterward
}

void runStep(uint8_t i) {
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
}

void runSweep() {
  Serial.println(F("Sweep starting."));
  for (uint8_t i = 0; i < SWEEP_COUNT; i++) {
    runStep(i);
    delay(STEP_DELAY_MS);
  }
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
  Serial.println(F("Type '1'-'9' for one step, 's' for the full auto-sweep, or 'o'/'f' for a manual test at baseline."));
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '9') runStep(c - '1');
    if (c == 's') runSweep();
    if (c == 'o') sendAt(BASE_PULSE_LEN, CODE_ON);
    if (c == 'f') sendAt(BASE_PULSE_LEN, CODE_OFF);
  }
}
