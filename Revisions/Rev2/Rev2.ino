/*
   INITIAL AUTHOR: Dominick Civitano

   See: https://github.com/miguelbalboa/rfid/tree/master/examples/rfid_write_personal_data

   Uses MIFARE RFID card using RFID-RC522 reader
   Uses MFRC522 - Library
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

   Schematic
   Adruino

*/
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 11; // Connects to module's RX
static const uint8_t PIN_MP3_RX = 10; // Connects to module's TX
int counter = 2;

#define PIN 5
#define PINin 6
#define NUMPIXELS1      16
#define NUMPIXELS2      11
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS1, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsIn = Adafruit_NeoPixel(NUMPIXELS2, PINin, NEO_GRB + NEO_KHZ800);


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

int spinspeed = 50;
int red = 255;
int green = 255;
int blue = 0;
int redAccess = 0;
int greenAccess = 255;
int blueAccess = 0;
boolean TreeOn = false;


const int relay = 7; //relay control pin

//*****************************************************************************************//

void setup() { 
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  //while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();    // Init MFRC522 card
  delay(1000);
  //mfrc522.PCD_DumpVersionToSerial();
  pinMode(relay, OUTPUT);



  digitalWrite(relay, LOW);
  pixels.begin();
  pixelsIn.begin();
  pixelsIn.show();
  pixels.show(); // Initialize all pixels to 'off'
  // Init SPI bus

  Serial.println(F("Goodnight Moon!"));    //shows in serial that it is ready to read
}

//*****************************************************************************************//

void test()
{
  pixels.setPixelColor(18, pixels.Color(red, green, blue));
  pixels.show();
  pixelsIn.setPixelColor(5, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(1000);
  pixels.setPixelColor(18, pixels.Color(0, 0, 0));
  pixels.show();
  pixelsIn.setPixelColor(5, pixelsIn.Color(0, 0, 0));
  pixelsIn.show();
  delay(1000);
}
void disney() {
  /*pixels.setPixelColor(0, pixels.Color(0, 0, 0));//
    pixels.setPixelColor(23, pixels.Color(red, green, blue));
    pixels.show();
    delay(spinspeed);
    pixels.setPixelColor(23, pixels.Color(0, 0, 0));
    pixels.setPixelColor(22, pixels.Color(red, green, blue));
    pixels.show();
    delay(spinspeed);
    pixels.setPixelColor(22, pixels.Color(0, 0, 0));
    pixels.setPixelColor(21, pixels.Color(red, green, blue));
    pixels.show();
    delay(spinspeed);
    pixels.setPixelColor(21, pixels.Color(0, 0, 0));
    pixels.setPixelColor(20, pixels.Color(red, green, blue));
    pixels.show();
    delay(spinspeed);
    pixels.setPixelColor(20, pixels.Color(0, 0, 0));*/
  pixelsIn.setPixelColor(0, pixelsIn.Color(0, 0, 0));//
  pixelsIn.setPixelColor(19, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(19, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(18, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(18, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(17, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(17, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(16, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(16, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(15, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(15, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(14, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(14, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(13, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(13, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(12, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(12, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(11, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(11, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(10, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(10, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(9, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(9, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(8, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(8, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(7, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(7, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(6, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(6, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(5, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(5, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(4, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(4, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(3, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(3, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(2, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(2, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(1, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  pixelsIn.setPixelColor(1, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(0, pixelsIn.Color(red, green, blue));
  pixelsIn.show();
  delay(spinspeed);
  //pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  //pixels.show();
  //delay(spinspeed);
}
void(* resetFunc) (void) = 0;

void disneyaccess()
{
  //  player.play(1);

  pixels.setPixelColor(0, pixels.Color(redAccess-20, greenAccess, blueAccess));
  pixels.setPixelColor(1, pixels.Color(redAccess-60, greenAccess, blueAccess));
  pixels.setPixelColor(2, pixels.Color(redAccess-80, greenAccess, blueAccess));
  pixels.setPixelColor(3, pixels.Color(redAccess-100, greenAccess, blueAccess));
  pixels.setPixelColor(4, pixels.Color(redAccess-120, greenAccess, blueAccess));
  pixels.setPixelColor(5, pixels.Color(redAccess-140, greenAccess, blueAccess));
  pixels.setPixelColor(6, pixels.Color(redAccess-160, greenAccess, blueAccess));
  pixels.setPixelColor(7, pixels.Color(redAccess-180, greenAccess, blueAccess));
  pixels.setPixelColor(8, pixels.Color(redAccess-200, greenAccess, blueAccess));
  pixels.setPixelColor(9, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(10, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(11, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(12, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(13, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(14, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(15, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(16, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(17, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(18, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(19, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(20, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(21, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(22, pixels.Color(redAccess, greenAccess, blueAccess));
  pixels.setPixelColor(23, pixels.Color(redAccess, greenAccess, blueAccess));

  pixelsIn.setPixelColor(0, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(1, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(2, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(3, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(4, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(5, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(6, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(7, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(8, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(9, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(10, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(11, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(12, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(13, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(14, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(15, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(16, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(17, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(18, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(19, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(20, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(21, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(22, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixelsIn.setPixelColor(23, pixelsIn.Color(redAccess, greenAccess, blueAccess));
  pixels.show();
  pixelsIn.show();

  if (digitalRead(relay) == HIGH)
  {
    digitalWrite(relay, LOW);
    Serial.println("Relay off");
  }
  else
  {
    digitalWrite(relay, HIGH);
    Serial.println("Relay on");
  }


}
void rainbow2(int wait) {
  Serial.println("Starting Rainbow");
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; /*i<pixels.numPixels()*/ 10; i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      pixelsIn.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show();
    pixelsIn.show();
    delay(wait);
  }
  Serial.println("Ending Rainbow");
}
void disneyoff()
{
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.setPixelColor(1, pixels.Color(0, 0, 0));
  pixels.setPixelColor(2, pixels.Color(0, 0, 0));
  pixels.setPixelColor(3, pixels.Color(0, 0, 0));
  pixels.setPixelColor(4, pixels.Color(0, 0, 0));
  pixels.setPixelColor(5, pixels.Color(0, 0, 0));
  pixels.setPixelColor(6, pixels.Color(0, 0, 0));
  pixels.setPixelColor(7, pixels.Color(0, 0, 0));
  pixels.setPixelColor(8, pixels.Color(0, 0, 0));
  pixels.setPixelColor(9, pixels.Color(0, 0, 0));
  pixels.setPixelColor(10, pixels.Color(0, 0, 0));
  pixels.setPixelColor(11, pixels.Color(0, 0, 0));
  pixels.setPixelColor(12, pixels.Color(0, 0, 0));
  pixels.setPixelColor(13, pixels.Color(0, 0, 0));
  pixels.setPixelColor(14, pixels.Color(0, 0, 0));
  pixels.setPixelColor(15, pixels.Color(0, 0, 0));
  pixels.setPixelColor(16, pixels.Color(0, 0, 0));
  pixels.setPixelColor(17, pixels.Color(0, 0, 0));
  pixels.setPixelColor(18, pixels.Color(0, 0, 0));
  pixels.setPixelColor(19, pixels.Color(0, 0, 0));
  pixels.setPixelColor(20, pixels.Color(0, 0, 0));
  pixels.setPixelColor(21, pixels.Color(0, 0, 0));
  pixels.setPixelColor(22, pixels.Color(0, 0, 0));
  pixels.setPixelColor(23, pixels.Color(0, 0, 0));
  pixels.show();

  pixelsIn.setPixelColor(0, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(1, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(2, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(3, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(4, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(5, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(6, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(7, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(8, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(9, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(10, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(11, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(12, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(13, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(14, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(15, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(16, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(17, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(18, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(19, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(20, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(21, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(22, pixelsIn.Color(0, 0, 0));
  pixelsIn.setPixelColor(23, pixelsIn.Color(0, 0, 0));
  pixelsIn.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
Serial.println("start rainbow");
  for (j = 0; j < 256; j++) {
    for (i = 0; i < pixels.numPixels(); i++) {
      
      pixels.setPixelColor(i, Wheel((i * 1 + j) & 255));
      pixelsIn.setPixelColor(i, Wheel((i * 1 + j) & 255));
         if (mfrc522.PICC_IsNewCardPresent()) {
          Serial.println("read card in rainbow, turning off");
        //  turnoff();
          delay(1000);
    }
    pixels.show();
    pixelsIn.show();
   delay(wait);
 
    }
  }
 
}

void rainbowstill()
{
int i;
int j;
for (i = 0; i < pixels.numPixels(); i++) {
      
      pixels.setPixelColor(i, Wheel((i * 1 + j) & 255));
      pixelsIn.setPixelColor(i, Wheel((i * 1 + j) & 255));
    pixels.show();
    pixelsIn.show();
      
}
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }

  if (WheelPos < 85) {
    return pixelsIn.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return pixelsIn.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return pixelsIn.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}



void loop()
{
 //rainbow(10);
 //test();
 
 // delay(10);
//
//disneyaccess();
 

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    Serial.println(F("**Band/Card Detected:**"));

    //-------------------------------------------
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    //if (content.substring(1) == "04 91 28 8A D5 2C 80") //change here the UID of the card/cards that you want to give access
    //{
    Serial.println("Authorized access. Light up the tree!");
    if ( (counter % 2) == 0) { 
    disney();
    disney();
    disney();
    if (digitalRead(relay) == HIGH)
  {
    digitalWrite(relay, LOW);
    Serial.println("Relay off");
  }
  else
  {
    digitalWrite(relay, HIGH);
    Serial.println("Relay on");
  }
    //rainbow2(10);
    rainbowstill();
    counter++; 
      
    }
    
    else
    {    disney();
    disney();
    disney();
    disneyoff();
    counter++;

       if (digitalRead(relay) == HIGH)
  {
    digitalWrite(relay, LOW);
    Serial.println("Relay off");
  }
  else
  {
    digitalWrite(relay, HIGH);
    Serial.println("Relay on");
  }
    }
    //player.play(1);
    //delay(2000);


    Serial.println("Done w loop");

    delay(2500);



    //else   {
    // Serial.println(" Access denied");

    //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

    //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

    //-------------------------------------------
/*
    Serial.print(F("Name: "));

    byte buffer1[18];

    block = 4;
    len = 18;

    //------------------------------------------- GET FIRST NAME
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
    if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
    }

    status = mfrc522.MIFARE_Read(block, buffer1, &len);
    if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
    }

    //PRINT FIRST NAME
    for (uint8_t i = 0; i < 16; i++)
    {
    if (buffer1[i] != 32)
    {
      Serial.write(buffer1[i]);
    }
    }
    Serial.print(" ");

    //---------------------------------------- GET LAST NAME

    byte buffer2[18];
    block = 1;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
    if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
    }

    status = mfrc522.MIFARE_Read(block, buffer2, &len);
    if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
    }

    //PRINT LAST NAME
    for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
    }


    //----------------------------------------
*/
    Serial.println(F("\n**End Reading**\n"));

    delay(1000); //change value if you want to read cards faster

    //mfrc522.PICC_HaltA();
   // mfrc522.PCD_StopCrypto1();
  
}
//*****************************************************************************************//
