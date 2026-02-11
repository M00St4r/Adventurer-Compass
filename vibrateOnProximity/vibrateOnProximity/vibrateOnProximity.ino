#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


/*TODO:
  Rework smoothing, smooth with RSSI too.

*/
// String knownBLEAddresses[] = {"6E:bc:55:18:cf:7b", "53:3c:cb:56:36:02", "40:99:4b:75:7d:2f", "5c:5b:68:6f:34:96"};
static BLEUUID targetBeaconUUID("00000000-0000-0000-0000-000000000000");

String targetUUID = "00000000000000000000000000000000";
int RSSI_THRESHOLD = -100;
bool device_found;
int scanTime = 1; //In seconds

int vibrator = 12;

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
      digitalWrite(vibrator, LOW);
      vibratorStartTime = 0;
      break;
    case 1:
      interval = 100;
      powerInterval = 100;
      vibratorRunTime += delta;

      if (vibratorRunTime >= interval){
        digitalWrite(vibrator, HIGH);
        vibratorPowerTime += delta;

        if(vibratorPowerTime >= powerInterval){
          vibratorRunTime = 0;
        }
      }else {
        digitalWrite(vibrator, LOW);
        vibratorPowerTime = 0;
      }
      break;
    case 2:
      interval = 500;
      powerInterval = 100;
      vibratorRunTime += delta;

      if (vibratorRunTime >= interval){
        digitalWrite(vibrator, HIGH);
        vibratorPowerTime += delta;

        if(vibratorPowerTime >= powerInterval){
          vibratorRunTime = 0;
        }
      }else {
        digitalWrite(vibrator, LOW);
        vibratorPowerTime = 0;
      }
      break;
    case 3:
      interval = 1000;
      powerInterval = 100;
      vibratorRunTime += delta;

      if (vibratorRunTime >= interval){
        digitalWrite(vibrator, HIGH);
        vibratorPowerTime += delta;

        if(vibratorPowerTime >= powerInterval){
          vibratorRunTime = 0;
        }
      }else {
        digitalWrite(vibrator, LOW);
        vibratorPowerTime = 0;
      }
      break;
  }

}

// void setVibratorIntensity(bool powered, unsigned long delta, unsigned long interval, unsigned long powerInterval){

//   if(powered){
//     vibratorRunTime += delta;

//       if (vibratorRunTime >= interval){
//         digitalWrite(vibrator, HIGH);
//         vibratorPowerTime += delta;

//         if(vibratorPowerTime >= powerInterval){
//           vibratorRunTime = 0;
//         }
//       }else {
//         digitalWrite(vibrator, LOW);
//         vibratorPowerTime = 0;
//       }
//   }else{
//     digitalWrite(vibrator, LOW);
//     vibratorStartTime = 0;
//   }

// }

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
void setup() {
  Serial.begin(115200); //Enable UART on ESP32
  Serial.println("Scanning..."); // Print Scanning
  pinMode(vibrator, OUTPUT); //make BUILTIN_LED pin as output
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Init Callback Function
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100); // set Scan interval
  pBLEScan->setWindow(99);  // less or equal setInterval value
  //pBLEScan->start(0,true); // continous scan
  Serial.print(targetBeaconUUID.toString().c_str());
}

void loop() {
  lastTime = currentTime;
  currentTime = millis();
  deltaTime = currentTime - lastTime;

  // Serial.print("Delta Time: ");
  // Serial.println(deltaTime);
  // Serial.print("millis: ");
  // Serial.println(currentTime);

  //ledOFF();

  BLEScanResults* foundDevices = pBLEScan->start(0, true);
  Serial.print("Devices found: ");
  Serial.println(foundDevices->getCount());

  for (int i = 0; i < foundDevices->getCount(); i++)
  {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);

    String deviceUUID = extractUUID(device.getManufacturerData());

    //Serial.println("Device UUID: " + deviceUUID);

    if (deviceUUID == targetUUID) {
      rssi = device.getRSSI();
      
      if (rssi > RSSI_THRESHOLD){
        pushStation(1);
        pushRssi(rssi);
        Serial.print("Rssi: ");
        Serial.println(rssi);
      }
    }else{
      pushStation(0);
      //pushRssi(-100);
    }
  }

  //calculate average rssi from last 5 entries
  float rssiAverage = 0;
  for (int i = 0; i < BUFFER_SIZE; i++){
    rssiAverage += rssiBuffer[i];
  }
  rssiAverage /= BUFFER_SIZE;
  Serial.print("Average rssi: ");
  Serial.println(rssiAverage);

  // calculate average device presence
  float average = 0;
  for(int i = 0; i < BUFFER_SIZE; i++){
    average += proximityBuffer[i];
  }
  average /= BUFFER_SIZE;
  // Serial.print("Device Average");
  // Serial.println(average);

  if(average > 0){
    if(rssiAverage > -40){
      setVibratorIntensity(1, deltaTime);
    }else if(rssiAverage > -60){
      setVibratorIntensity(2, deltaTime);
    }else if(rssiAverage > -80){
      setVibratorIntensity(3, deltaTime);
    }
  }else{
    setVibratorIntensity(0, deltaTime);
  }

  pBLEScan->clearResults();
  delay(1); // Wait before next scan
}