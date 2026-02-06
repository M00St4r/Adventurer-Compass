/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-rfid-nfc
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 27 // ESP32 pin GPIO27 
#define userLimit 5
String checkedIds[userLimit];

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  for(int i = 0; i < userLimit; i++){
    checkedIds[i] = String("User");
    Serial.println(checkedIds[i]);
  }
}

void loop() {
  bool userFound = false;
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      String uidString = "";
      for (int i = 0; i < rfid.uid.size; i++) {  
        uidString +=  String(rfid.uid.uidByte[i], HEX);
      }

      for (int i = 0; i < userLimit -1; i++) {
        if (checkedIds[i] == uidString) {
          checkedIds[i] = "User";
          userFound = true;
          Serial.println(uidString + " checked out");
          break;
        }
      }

      if (userFound == false) {
        for (int i = 0; i < userLimit; i++) {

          Serial.println(checkedIds[i]+ String(" : ") + String(i));

          if (checkedIds[i] == "User") {
            checkedIds[i] = uidString;
            Serial.println(uidString + "checked in");
            break;
          }else if(i == userLimit-1){
            Serial.println("User Limit reached");
          }
        }      
      }
      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(rfid.)
      }


      Serial.println();

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}