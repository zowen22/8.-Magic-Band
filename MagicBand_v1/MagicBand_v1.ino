/*
   Magic Band — RFID-triggered LED ring and relay controller
   Target: Arduino Nano

   MFRC522 wiring:
     RST → D9  |  SS → D10  |  MOSI → D11  |  MISO → D12  |  SCK → D13
*/

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <SPI.h>
#include <MFRC522.h>

// ── LED Strip Configuration ────────────────────────────────────────────────
// Update these two values when cutting strips to a different length
#define OUTER_LEDS  16
#define INNER_LEDS  11
#define PIN_OUTER    5
#define PIN_INNER    6

Adafruit_NeoPixel outerRing(OUTER_LEDS, PIN_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel innerRing(INNER_LEDS, PIN_INNER, NEO_GRB + NEO_KHZ800);

// ── RFID ──────────────────────────────────────────────────────────────────
#define RST_PIN  9
#define SS_PIN   10
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ── DFPlayer Mini ─────────────────────────────────────────────────────────
// Note: PIN_MP3_RX shares D10 with SS_PIN — confirm wiring before enabling
static const uint8_t PIN_MP3_TX = 11;
static const uint8_t PIN_MP3_RX = 10;
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;

// ── State ─────────────────────────────────────────────────────────────────
const int RELAY_PIN   = 7;
const int SPIN_SPEED  = 50;
int scanCount         = 2;  // even → lights on branch fires first

// Idle color (yellow) and access color (green)
const uint8_t r = 255, g = 255, b = 0;
const uint8_t rA = 0,  gA = 255, bA = 0;

//*******************************************************************************//

void setup() {
  delay(100);
  Serial.begin(9600);
  delay(100);
  softwareSerial.begin(9600);
  delay(100);
  while (!Serial);
  SPI.begin();
  delay(500);
  mfrc522.PCD_Init();
  delay(500);
  mfrc522.PCD_DumpVersionToSerial();
  delay(500);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  if (player.begin(softwareSerial)) {
    Serial.println("Speaker on");
    player.volume(20);
  }

  outerRing.begin();
  innerRing.begin();
  outerRing.show();
  innerRing.show();

  Serial.println(F("Ready."));
}

//*******************************************************************************//

void ledsOff() {
  outerRing.fill(0);
  outerRing.show();
  innerRing.fill(0);
  innerRing.show();
}

void ledsAccess() {
  for (int i = 0; i < outerRing.numPixels(); i++)
    outerRing.setPixelColor(i, outerRing.Color(rA, gA, bA));
  for (int i = 0; i < innerRing.numPixels(); i++)
    innerRing.setPixelColor(i, innerRing.Color(rA, gA, bA));
  outerRing.show();
  innerRing.show();
}

// Chase animation on inner ring, high index → low
void spinAnimation() {
  for (int i = innerRing.numPixels() - 1; i >= 0; i--) {
    if (i < innerRing.numPixels() - 1)
      innerRing.setPixelColor(i + 1, innerRing.Color(0, 0, 0));
    innerRing.setPixelColor(i, innerRing.Color(r, g, b));
    innerRing.show();
    delay(SPIN_SPEED);
  }
  innerRing.setPixelColor(0, innerRing.Color(0, 0, 0));
  innerRing.show();
  delay(SPIN_SPEED);
}

void toggleRelay() {
  if (digitalRead(RELAY_PIN) == HIGH) {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Relay off");
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Relay on");
  }
}

uint32_t Wheel(byte pos) {
  if (pos < 85)
    return outerRing.Color(pos * 3, 255 - pos * 3, 0);
  else if (pos < 170) {
    pos -= 85;
    return outerRing.Color(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return outerRing.Color(0, pos * 3, 255 - pos * 3);
  }
}

void rainbow(uint8_t wait) {
  Serial.println("start rainbow");
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < outerRing.numPixels(); i++) {
      outerRing.setPixelColor(i, Wheel((i + j) & 255));
      if (mfrc522.PICC_IsNewCardPresent()) {
        Serial.println("Card detected in rainbow");
        delay(1000);
      }
    }
    for (int i = 0; i < innerRing.numPixels(); i++)
      innerRing.setPixelColor(i, Wheel((i + j) & 255));
    outerRing.show();
    innerRing.show();
    delay(wait);
  }
}

void rainbow2(int wait) {
  for (long hue = 0; hue < 5 * 65536; hue += 256) {
    for (int i = 0; i < outerRing.numPixels(); i++) {
      long pixelHue = hue + (i * 65536L / outerRing.numPixels());
      outerRing.setPixelColor(i, outerRing.gamma32(outerRing.ColorHSV(pixelHue)));
    }
    for (int i = 0; i < innerRing.numPixels(); i++) {
      long pixelHue = hue + (i * 65536L / innerRing.numPixels());
      innerRing.setPixelColor(i, innerRing.gamma32(innerRing.ColorHSV(pixelHue)));
    }
    outerRing.show();
    innerRing.show();
    delay(wait);
  }
}

void(* resetFunc)(void) = 0;

//*******************************************************************************//

void loop() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte len;
  MFRC522::StatusCode status;

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial())   return;

  Serial.println(F("**Band/Card Detected:**"));
  Serial.print("UID: ");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();

  Serial.println("Authorized access");

  spinAnimation();
  spinAnimation();
  spinAnimation();
  toggleRelay();

  if ((scanCount % 2) == 0) {
    rainbow2(10);
  } else {
    ledsOff();
  }
  scanCount++;

  player.play(1);
  Serial.println();
  delay(2500);

  // Read name stored on card
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));
  Serial.print(F("Name: "));

  byte buffer1[18];
  byte block = 4;
  len = 18;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Auth failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Read failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer1[i] != 32) Serial.write(buffer1[i]);
  }
  Serial.print(" ");

  byte buffer2[18];
  block = 1;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Auth failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Read failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  for (uint8_t i = 0; i < 16; i++) Serial.write(buffer2[i]);

  Serial.println(F("\n**End Reading**\n"));
  delay(1000);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

//*******************************************************************************//
