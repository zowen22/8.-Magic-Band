/*
 * 433MHz RF Code Sniffer
 *
 * Listens for RF signals and prints the code, bit length, and protocol
 * to Serial Monitor. Use this once to capture your outlet remote's codes.
 *
 * Wiring:
 *   Receiver DATA pin  →  Arduino pin 2
 *   Receiver VCC       →  Arduino 5V
 *   Receiver GND       →  Arduino GND
 *
 * Steps:
 *   1. Upload this sketch
 *   2. Open Serial Monitor at 9600 baud
 *   3. Press each button on your outlet remote (A-ON, A-OFF, B-ON, etc.)
 *   4. Record the "Value" for each button — paste into Rev4
 *
 * Library required: rc-switch (install via Arduino Library Manager)
 */

#include <RCSwitch.h>

#define PIN_RECEIVER 2

RCSwitch rf = RCSwitch();

void setup() {
  Serial.begin(9600);
  rf.enableReceive(digitalPinToInterrupt(PIN_RECEIVER));
  Serial.println("Sniffer ready. Press buttons on your remote...");
  Serial.println("------------------------------------------------");
}

void loop() {
  if (rf.available()) {
    Serial.print("Value:    ");
    Serial.println(rf.getReceivedValue());
    Serial.print("Bits:     ");
    Serial.println(rf.getReceivedBitlength());
    Serial.print("Protocol: ");
    Serial.println(rf.getReceivedProtocol());
    Serial.println("------------------------------------------------");
    rf.resetAvailable();
  }
}
