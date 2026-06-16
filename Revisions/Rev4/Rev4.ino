/*
 * MagicBand Tree Controller — Rev4
 * Hardware: Arduino Nano (ATmega328P)
 *
 * Tap an RFID card/band to toggle Christmas tree lights via 433MHz RF outlet.
 * No mains wiring — Arduino sends RF signal to a pre-paired outlet.
 *
 * Power: 5.5mm/2.1mm barrel jack → Nano VIN + GND pins (5V 3A supply)
 *
 * Pin map (Nano pin numbers = Uno pin numbers, different physical layout):
 *   2  — DFPlayer Mini RX  (Arduino TX → player)
 *   3  — DFPlayer Mini TX  (player TX → Arduino)
 *   4  — MFRC522 SS
 *   5  — NeoPixel outer ring
 *   6  — NeoPixel inner ring
 *   8  — RF transmitter DATA
 *   9  — MFRC522 RST
 *  11  — SPI MOSI
 *  12  — SPI MISO
 *  13  — SPI SCK
 *  3V3 — MFRC522 VCC
 *  GND — common ground
 *
 * Before flashing:
 *   1. Run Sniffer.ino with your receiver to capture remote codes
 *   2. Fill in RF_CODE_ON and RF_CODE_OFF below
 *   3. Put outlet in learn mode, flash this sketch, scan any RFID card once
 *      to confirm the outlet responds, then it's ready to ship
 *
 * Libraries required:
 *   rc-switch, Adafruit NeoPixel, MFRC522, DFRobotDFPlayerMini
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <RCSwitch.h>

// ── UPDATE THESE ──────────────────────────────────────────────────────────────
#define NUMPIXELS_OUTER  16         // outer ring pixel count
#define NUMPIXELS_INNER  11         // inner ring pixel count
#define RF_CODE_ON       0x000000   // replace with value from Sniffer (e.g. 0x44B13C)
#define RF_CODE_OFF      0x000000   // replace with value from Sniffer
#define RF_BITS          24         // almost always 24 — confirm from Sniffer output
// ─────────────────────────────────────────────────────────────────────────────

// Pins
#define PIN_OUTER    5
#define PIN_INNER    6
#define PIN_RF       8
#define RST_PIN      9
#define SS_PIN       4
#define PIN_MP3_TX   2
#define PIN_MP3_RX   3

// Animation colors
#define COL_CHASE_R  255
#define COL_CHASE_G  220
#define COL_CHASE_B  0
#define COL_ACCESS_R 0
#define COL_ACCESS_G 255
#define COL_ACCESS_B 0

#define CHASE_SPEED_MS   50
#define SCAN_DEBOUNCE_MS 2500
#define RF_REPEAT        5    // send RF code N times to ensure reception

// ── Objects ───────────────────────────────────────────────────────────────────
Adafruit_NeoPixel outer(NUMPIXELS_OUTER, PIN_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel inner(NUMPIXELS_INNER, PIN_INNER, NEO_GRB + NEO_KHZ800);
MFRC522 rfid(SS_PIN, RST_PIN);
SoftwareSerial mp3Serial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;
RCSwitch rf = RCSwitch();

bool treeOn = false;

// ── Helpers ───────────────────────────────────────────────────────────────────

void clearAll() {
  outer.fill(0);
  inner.fill(0);
  outer.show();
  inner.show();
}

void showAll(uint8_t r, uint8_t g, uint8_t b) {
  outer.fill(outer.Color(r, g, b));
  inner.fill(inner.Color(r, g, b));
  outer.show();
  inner.show();
}

void toggleTree() {
  treeOn = !treeOn;
  // Send multiple times — 433MHz is fire-and-forget, no ACK
  for (int i = 0; i < RF_REPEAT; i++) {
    rf.send(treeOn ? RF_CODE_ON : RF_CODE_OFF, RF_BITS);
  }
  Serial.println(treeOn ? "Tree ON" : "Tree OFF");
}

void chaseAnimation(int passes) {
  for (int p = 0; p < passes; p++) {
    for (int i = NUMPIXELS_INNER - 1; i >= 0; i--) {
      inner.fill(0);
      inner.setPixelColor(i, inner.Color(COL_CHASE_R, COL_CHASE_G, COL_CHASE_B));
      inner.show();
      delay(CHASE_SPEED_MS);
    }
  }
  inner.fill(0);
  inner.show();
}

void accessFlash() {
  showAll(COL_ACCESS_R, COL_ACCESS_G, COL_ACCESS_B);
  delay(300);
  clearAll();
  delay(100);
  showAll(COL_ACCESS_R, COL_ACCESS_G, COL_ACCESS_B);
  delay(300);
  clearAll();
}

void rainbowCycle(int waitMs, int cycles) {
  for (int c = 0; c < cycles; c++) {
    for (long hue = 0; hue < 65536; hue += 512) {
      for (int i = 0; i < NUMPIXELS_OUTER; i++) {
        long pixelHue = hue + (i * 65536L / NUMPIXELS_OUTER);
        outer.setPixelColor(i, outer.gamma32(outer.ColorHSV(pixelHue)));
      }
      for (int i = 0; i < NUMPIXELS_INNER; i++) {
        long pixelHue = hue + (i * 65536L / NUMPIXELS_INNER);
        inner.setPixelColor(i, inner.gamma32(inner.ColorHSV(pixelHue)));
      }
      outer.show();
      inner.show();
      delay(waitMs);
    }
  }
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  rf.enableTransmit(PIN_RF);
  rf.setRepeatTransmit(1); // we handle repeats manually in toggleTree()

  outer.begin();
  inner.begin();
  clearAll();

  if (player.begin(mp3Serial)) {
    player.volume(20);
    Serial.println("DFPlayer ready");
  } else {
    Serial.println("DFPlayer not found — continuing without audio");
  }

  Serial.println("Ready. Waiting for band...");
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Print UID
  Serial.print("UID:");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Animate, play audio, toggle tree
  player.play(1);
  chaseAnimation(3);
  accessFlash();
  toggleTree();

  if (treeOn) {
    rainbowCycle(8, 1);
  } else {
    clearAll();
  }

  delay(SCAN_DEBOUNCE_MS);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
