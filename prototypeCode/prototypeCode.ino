#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
/* neopixel library */
#include <Adafruit_NeoPixel.h>
/* libraries for RFID/NFC */
#include <SPI.h>
#include <MFRC522.h>

int loopDelay = 100;

enum STATE {IDLE, SEARCH, QUEST};
STATE compassState = IDLE;

//static BLEUUID targetBeaconUUID("00000000-0000-0000-0000-000000000000");

String targetUUID = "00000000000000000000000000000000";
int RSSI_THRESHOLD = -100;
bool device_found;
int scanTime = 1; //In seconds

int vibratorPIN = 12;

int maxRSSI = -30;
int minRSSI = -100;
int rssi;

#define BUFFER_SIZE 5
int proximityBuffer[BUFFER_SIZE];
int stationIdx = 0;

int rssiBuffer[BUFFER_SIZE];
int rssiIdx = 0;

unsigned long lastTime = 0;
unsigned long currentTime = 0;
unsigned long deltaTime = 0;

// vibrator variables
unsigned long vibratorStartTime = 0;
unsigned long vibratorRunTime = 0;
unsigned long vibratorPowerTime = 0;

/* neopixel setup*/
int ledPin = 2;
#define NUMPIXELS 24
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPin, NEO_GRB + NEO_KHZ800);
int delayCircle = 100;
int delayBlink = 250;
int colorR = 50;
int colorG = 0;
int colorB = 50;
int currentPixel = 0;
int blinkCounter = 0;
bool blinking = false;
bool blinkOn = false;
int blinkTimer = 0;
int idleTimer = 0;

/* NFC setup */
#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 27 // ESP32 pin GPIO27 
#define userLimit 5
String checkedIds[userLimit];
MFRC522 rfid(SS_PIN, RST_PIN);

//Interrupt timer
uint32_t ISR_millis = 0;

//button
struct Button {
  const uint8_t PIN;
  volatile uint32_t numberKeyPresses;
  volatile bool pressed;
};

Button button1 = { 26, 0, false };

// interrupt function
void IRAM_ATTR isr(){
  // use timer to holdoff for 200mS after each count
  if( (millis() - ISR_millis) > 200 ){
    ISR_millis = millis();
    Serial.println("power Button Pressed");

    if (compassState == IDLE){
      compassState = SEARCH;
    }else{
      compassState = IDLE;
      BLEDevice::getScan()->stop();
    }
  }
}

void pushRssi(int value) {
  rssiBuffer[rssiIdx] = value;
  rssiIdx = (rssiIdx + 1) % BUFFER_SIZE;
}

int popRssi() {
  rssiIdx = (rssiIdx - 1 + BUFFER_SIZE) % BUFFER_SIZE;
  return rssiBuffer[rssiIdx];
}

void pushStation(int value) {
  proximityBuffer[stationIdx] = value;
  stationIdx = (stationIdx + 1) % BUFFER_SIZE;
}

int popStation() {
  stationIdx = (stationIdx - 1 + BUFFER_SIZE) % BUFFER_SIZE;
  return proximityBuffer[stationIdx];
}


void setVibratorIntensity(int state, unsigned long delta){

  unsigned long interval;
  unsigned long powerInterval;

  switch(state){
    case 0:
      digitalWrite(vibratorPIN, LOW);
      vibratorStartTime = 0;
      vibratorPowerTime = 0;
      break;
    case 1:
      interval = 100;
      powerInterval = 100;
      vibratorRunTime += delta;

      if (vibratorRunTime >= interval){
        digitalWrite(vibratorPIN, HIGH);
        vibratorPowerTime += delta;

        if(vibratorPowerTime >= powerInterval){
          vibratorRunTime = 0;
          vibratorPowerTime = 0;
        }
      }else {
        digitalWrite(vibratorPIN, LOW);
        vibratorPowerTime = 0;
      }
      break;
    case 2:
      interval = 500;
      powerInterval = 100;
      vibratorRunTime += delta;

      if (vibratorRunTime >= interval){
        digitalWrite(vibratorPIN, HIGH);
        vibratorPowerTime += delta;

        if(vibratorPowerTime >= powerInterval){
          vibratorRunTime = 0;
          vibratorPowerTime = 0;
        }
      }else {
        digitalWrite(vibratorPIN, LOW);
        vibratorPowerTime = 0;
      }
      break;
    case 3:
      interval = 1000;
      powerInterval = 100;
      vibratorRunTime += delta;

      if (vibratorRunTime >= interval){
        digitalWrite(vibratorPIN, HIGH);
        vibratorPowerTime += delta;

        if(vibratorPowerTime >= powerInterval){
          vibratorRunTime = 0;
          vibratorPowerTime = 0;
        }
      }else {
        digitalWrite(vibratorPIN, LOW);
        vibratorPowerTime = 0;
      }
      break;
  }

}

String extractUUID(String input){

  String hexString = "";

  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    char hexChar[3];
    sprintf(hexChar, "%02X", c); // Format as uppercase hex with leading zero
    hexString += hexChar;
  }

  String deviceUUID = hexString.substring(8, 40);
  return deviceUUID;
}

BLEScan* pBLEScan;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (!advertisedDevice.haveManufacturerData()) return;

    //Serial.println(advertisedDevice.getName());
    String deviceUUID = extractUUID(advertisedDevice.getManufacturerData());
    //Serial.println(deviceUUID);
    int drssi = advertisedDevice.getRSSI();

    if(deviceUUID == targetUUID){
      // stop the scan, calculate rssi in loop
      Serial.println("found target Device!");
      BLEDevice::getScan()->stop();
    }
    
    // if (deviceUUID == targetUUID && drssi > RSSI_THRESHOLD) {
    //   BLEDevice::getScan()->stop();
    //   Serial.println(drssi);
    // }
  }

};

void neopixelIdle() {
idleTimer += deltaTime;
if (blinkOn == false) { //only execute when led is not blinking
  if (idleTimer >= delayCircle) {
  idleTimer = 0;
  pixels.clear();
  pixels.setPixelColor(currentPixel, pixels.Color(colorR,colorG, colorB));
  pixels.setPixelColor(currentPixel - 1, pixels.Color(colorR * 0.5, colorG * 0.5, colorB * 0.5));
  pixels.show();
  //Serial.println(currentPixel);
  currentPixel += 1;
  if (currentPixel == NUMPIXELS) {
    currentPixel = 0;
    }
  }
}

}

void neopixelBlink() {
int maxBlink = 2;
blinkTimer += deltaTime;
if (blinking == true) {
  if (blinkOn == false) {
    pixels.clear();
    if (blinkTimer >= delayBlink){
      for(int j=0; j<NUMPIXELS; j++) {
      pixels.setPixelColor(j, pixels.Color(colorR,colorG,colorB));
      }
      pixels.show();
      blinkOn = true;
      blinkTimer = 0;
    }
  }
  else if (blinkOn == true) {
    if (blinkTimer >= delayBlink) {
      pixels.clear();
      pixels.show();
      blinkOn = false;
      blinkCounter += 1;
      blinkTimer = 0;
    }
  }
  if (blinkCounter >= maxBlink) {
    blinking = false;
    blinkTimer = 0;
  }
  }
}


// TEST QUEST

#define COLLECTABLE_ITEMS_COUNT 3
// the items that can be collected
String collectableItems[COLLECTABLE_ITEMS_COUNT] = {"0426AEA37E2681", "0431FEA37E2681", "2D2B3A04"};
// the items the Player has collected
String playerInventory[COLLECTABLE_ITEMS_COUNT];
int inventoryIdx = 0;
int collectedItems = 0;

void checkNFCScanner() {
  bool objectFound = false;
  bool objectCollectable = false;
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    Serial.println("nfc tag is present");
    if (rfid.PICC_ReadCardSerial()) { // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      String uidString;// = extractUUID(String(rfid.uid.uidByte, HEX));
      
      

      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : "");
        Serial.print(rfid.uid.uidByte[i], HEX);
        char buffer[3];
        sprintf(buffer, "%02X", rfid.uid.uidByte[i]);
        uidString += buffer;
      }

      Serial.println(uidString);

      for (int i = 0; i < COLLECTABLE_ITEMS_COUNT; i++) {
        if(collectableItems[i] == uidString) {
          objectCollectable = true;
          break;
        }
      }
      for (int i = 0; i < COLLECTABLE_ITEMS_COUNT -1; i++) {
        if (playerInventory[i] == uidString) {
          objectFound = true;
          Serial.println("Object already found");
          break;
        }
      }

      if (objectFound == false && objectCollectable == true) {
        for (int i = 0; i < COLLECTABLE_ITEMS_COUNT; i++) {

          Serial.println(playerInventory[i]+ String(" : ") + String(i));

          if (playerInventory[i] == "") {
            playerInventory[i] = uidString;
            collectedItems++;
            ledPercentage();
            break;
          }else if(i == COLLECTABLE_ITEMS_COUNT-1){
            Serial.println("All Objects Found");
          }
        }
        for (int i = 0; i < COLLECTABLE_ITEMS_COUNT; i ++){
        Serial.print("collectable Item ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(collectableItems[i]);
        Serial.print("Inventory Item ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(playerInventory[i]);
      }   
      }


      Serial.println();

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

void pushInventory(char value) {
  playerInventory[inventoryIdx] = value;
  inventoryIdx = (inventoryIdx + 1) % COLLECTABLE_ITEMS_COUNT;
}

void ledPercentage() {
  float objectPercent = (float)collectedItems/COLLECTABLE_ITEMS_COUNT;
  int ledPercent = round(NUMPIXELS*objectPercent);
  Serial.println("LED numbers:");
  Serial.println(objectPercent);
  Serial.println(ledPercent);
  Serial.println(collectedItems);
  Serial.println(COLLECTABLE_ITEMS_COUNT);
  pixels.clear();
  for (int i = 0; i < ledPercent; i++) {
    pixels.setPixelColor(i, pixels.Color(colorR, colorG, colorB));
  }
  pixels.show();
}


void setup() {
  // put your setup code here, to run once:
  pinMode(vibratorPIN, OUTPUT);
  Serial.begin(115200); //Enable UART on ESP32
  SPI.begin();
  rfid.PCD_Init();
  pixels.begin();
  pixels.clear();
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1.PIN), isr, FALLING);
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Init Callback Function
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100); // set Scan interval
  pBLEScan->setWindow(99);  // less or equal setInterval value
  //pBLEScan->start(0,true); // continous scan
  //Serial.print(targetBeaconUUID.toString().c_str());
}

void loop() {

  lastTime = currentTime;
  currentTime = millis();
  deltaTime = currentTime - lastTime;

  //Serial.println(compassState);

  // Declare variables outside the switch
  BLEScanResults* foundDevices = nullptr;
  float rssiAverage = 0;
  float average = 0;
  int rssi = 0;
  // quest vars
  int collectedCounter = 0;

  switch (compassState) {
      case IDLE:
        Serial.println("IDLEING");
        pixels.clear();
        delay(loopDelay);
        break;

      case SEARCH:
        //neopixelIdle();
        Serial.println("Searching");
        foundDevices = pBLEScan->start(0, true);
        Serial.print("Devices found: ");
        Serial.println(foundDevices->getCount());

        for (int i = 0; i < foundDevices->getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices->getDevice(i);
            String deviceUUID = extractUUID(device.getManufacturerData());

            if (deviceUUID == targetUUID) {
                rssi = device.getRSSI();
                if (rssi > RSSI_THRESHOLD) {
                    pushStation(1);
                    pushRssi(rssi);
                    Serial.print("Rssi: ");
                    Serial.println(rssi);
                }
            } else {
                pushStation(0);
            }
        }

        // Calculate average rssi from last 5 entries
        rssiAverage = 0;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            rssiAverage += rssiBuffer[i];
        }
        rssiAverage /= BUFFER_SIZE;
        Serial.print("Average rssi: ");
        Serial.println(rssiAverage);

        // Calculate average device presence
        average = 0;
        for(int i = 0; i < BUFFER_SIZE; i++) {
            average += proximityBuffer[i];
        }
        average /= BUFFER_SIZE;

        if(average > 0) {
            if(rssiAverage > -40) {
                setVibratorIntensity(1, deltaTime);
            } else if(rssiAverage > -60) {
                setVibratorIntensity(2, deltaTime);
            } else if(rssiAverage > -80) {
                //setVibratorIntensity(3, deltaTime);
                compassState = QUEST;
                BLEDevice::getScan()->stop();
                Serial.println("Quest Started!!!");
            }
        } else {
            setVibratorIntensity(0, deltaTime);
        }

        pBLEScan->clearResults();
        delay(1); // Wait before next scan
        break;

      case QUEST:
        checkNFCScanner();
          for (int i = 0; i < COLLECTABLE_ITEMS_COUNT; i++){
            for (int j = 0; j < COLLECTABLE_ITEMS_COUNT; j++){
              if(collectableItems[i] == playerInventory[j]){
                collectedCounter++;
              }
            }
          }

          

          if (collectedCounter == COLLECTABLE_ITEMS_COUNT){
            Serial.println("Quest Completed");
            blinking = false;
            blinkOn = false;
            neopixelIdle();
          }
          //Serial.println(collectedCounter);
          delay(1);
          break;
    }
    //delay(25);
}
