/*
   Magic Band — RFID-triggered LED ring and relay controller
   Target: Arduino Nano

   ── Pin Map ─────────────────────────────────────────────────────────────
   MFRC522 (SPI, fixed):
     RST → D9  |  SS → D10  |  MOSI → D11  |  MISO → D12  |  SCK → D13

   DFPlayer Mini (SoftwareSerial — D2/D3 avoid SPI pin conflicts):
     D2 → DFPlayer RX  |  D3 ← DFPlayer TX

   Relay      → D7
   Outer LEDs → D5
   Inner LEDs → D6
*/

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <SPI.h>
#include <MFRC522.h>

// ── LED Strip Configuration ────────────────────────────────────────────────
// Change these when cutting strips to a different length — nothing else needs updating
#define OUTER_LEDS  16
#define INNER_LEDS  11

const uint8_t PIN_OUTER = 5;
const uint8_t PIN_INNER = 6;

Adafruit_NeoPixel outerRing(OUTER_LEDS, PIN_OUTER, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel innerRing(INNER_LEDS, PIN_INNER, NEO_GRB + NEO_KHZ800);

// ── RFID ──────────────────────────────────────────────────────────────────
const uint8_t RST_PIN = 9;
const uint8_t SS_PIN  = 10;
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ── DFPlayer Mini ─────────────────────────────────────────────────────────
const uint8_t PIN_MP3_TX = 2;   // D2 → DFPlayer RX
const uint8_t PIN_MP3_RX = 3;   // D3 ← DFPlayer TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;
bool playerReady = false;

// ── Hardware ──────────────────────────────────────────────────────────────
const uint8_t RELAY_PIN  = 7;
const int     SPIN_SPEED = 50;  // ms per step in chase animation

// Colors — adjust to taste
const uint8_t R_IDLE   = 255, G_IDLE   = 255, B_IDLE   = 0;    // yellow
const uint8_t R_ACCESS =   0, G_ACCESS = 255, B_ACCESS  = 0;   // green (reserved for UID-gated use)

// ── State ─────────────────────────────────────────────────────────────────
int scanCount = 0;  // even → ON (rainbow); odd → OFF

//*******************************************************************************//

void setup() {
  Serial.begin(9600);
  // Allow up to 2 s for IDE serial monitor; skips automatically in standalone
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000);

  softwareSerial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  if (player.begin(softwareSerial)) {
    player.volume(20);
    playerReady = true;
    Serial.println(F("DFPlayer ready"));
  } else {
    Serial.println(F("DFPlayer not found — check wiring on D2/D3"));
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

// All LEDs to access color — wire to a specific UID check when needed
void ledsAccess() {
  for (int i = 0; i < outerRing.numPixels(); i++)
    outerRing.setPixelColor(i, outerRing.Color(R_ACCESS, G_ACCESS, B_ACCESS));
  for (int i = 0; i < innerRing.numPixels(); i++)
    innerRing.setPixelColor(i, innerRing.Color(R_ACCESS, G_ACCESS, B_ACCESS));
  outerRing.show();
  innerRing.show();
}

// Chase animation down the inner ring; wraps to strip length automatically
void spinAnimation() {
  for (int i = innerRing.numPixels() - 1; i >= 0; i--) {
    if (i < innerRing.numPixels() - 1)
      innerRing.setPixelColor(i + 1, 0);
    innerRing.setPixelColor(i, innerRing.Color(R_IDLE, G_IDLE, B_IDLE));
    innerRing.show();
    delay(SPIN_SPEED);
  }
  innerRing.setPixelColor(0, 0);
  innerRing.show();
  delay(SPIN_SPEED);
}

void toggleRelay() {
  bool on = digitalRead(RELAY_PIN) == HIGH;
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  Serial.println(on ? F("Relay off") : F("Relay on"));
}

static uint32_t colorWheel(byte pos) {
  if (pos < 85)
    return outerRing.Color(pos * 3, 255 - pos * 3, 0);
  if (pos < 170) {
    pos -= 85;
    return outerRing.Color(255 - pos * 3, 0, pos * 3);
  }
  pos -= 170;
  return outerRing.Color(0, pos * 3, 255 - pos * 3);
}

// Wheel-based rainbow — available for future triggers
void rainbow(uint8_t wait) {
  for (uint8_t j = 0; j < 255; j++) {
    for (int i = 0; i < outerRing.numPixels(); i++)
      outerRing.setPixelColor(i, colorWheel((i + j) & 255));
    for (int i = 0; i < innerRing.numPixels(); i++)
      innerRing.setPixelColor(i, colorWheel((i + j) & 255));
    outerRing.show();
    innerRing.show();
    delay(wait);
  }
}

// Smooth HSV rainbow, gamma-corrected
void rainbow2(int wait) {
  for (long hue = 0; hue < 5 * 65536L; hue += 256) {
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

// Read first/last name stored on card and print to serial
// Soft failures print a blank field; RFID halt is handled by the caller
void readGuestName() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte buffer[18];
  byte len;
  MFRC522::StatusCode status;

  Serial.print(F("Name: "));

  // First name — block 4
  len = 18;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
  if (status == MFRC522::STATUS_OK) {
    status = mfrc522.MIFARE_Read(4, buffer, &len);
    if (status == MFRC522::STATUS_OK)
      for (uint8_t i = 0; i < 16; i++)
        if (buffer[i] != 32) Serial.write(buffer[i]);
  }
  Serial.print(' ');

  // Last name — block 1
  len = 18;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid));
  if (status == MFRC522::STATUS_OK) {
    status = mfrc522.MIFARE_Read(1, buffer, &len);
    if (status == MFRC522::STATUS_OK)
      for (uint8_t i = 0; i < 16; i++)
        if (buffer[i] != 32) Serial.write(buffer[i]);
  }
  Serial.println();
}

//*******************************************************************************//

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial())   return;

  Serial.print(F("Card detected — UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(' ');
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Animate, toggle relay, set light state for this scan
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

  if (playerReady) player.play(1);
  delay(2500);

  readGuestName();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

//*******************************************************************************//
