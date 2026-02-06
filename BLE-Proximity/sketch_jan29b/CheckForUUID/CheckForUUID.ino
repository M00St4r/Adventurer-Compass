#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// String knownBLEAddresses[] = {"6E:bc:55:18:cf:7b", "53:3c:cb:56:36:02", "40:99:4b:75:7d:2f", "5c:5b:68:6f:34:96"};
static BLEUUID targetBeaconUUID("00000000-0000-0000-0000-000000000000");

String targetUUID = "00000000000000000000000000000000";
int RSSI_THRESHOLD = -60;
bool device_found;
int scanTime = 1; //In seconds

int LED_BUILTIN = 12;

int maxRSSI = -30;
int minRSSI = -60;
int rssi;

#define BUFFER_SIZE 5
int ledBuffer[BUFFER_SIZE];
int idx = 0;

void push(int value) {
  ledBuffer[idx] = value;
  idx = (idx + 1) % BUFFER_SIZE;
}

int pop() {
  idx = (idx - 1 + BUFFER_SIZE) % BUFFER_SIZE;
  return ledBuffer[idx];
}

void ledON(){
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("write LED HIGH");
}

void ledOFF(){
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("write LED LOW");
}

void ledValue(int value){
  analogWrite(LED_BUILTIN, value);
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
    
    if (deviceUUID == targetUUID && drssi > RSSI_THRESHOLD) {
      BLEDevice::getScan()->stop();
      Serial.println(drssi);
    }
  }

};
void setup() {
  Serial.begin(115200); //Enable UART on ESP32
  Serial.println("Scanning..."); // Print Scanning
  pinMode(LED_BUILTIN, OUTPUT); //make BUILTIN_LED pin as output
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

  //ledOFF();

  BLEScanResults* foundDevices = pBLEScan->start(scanTime, false);
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
        push(1);
      }
    }else{
      push(0);
    }
  }
  float median = 0;
  for(int i = 0; i < BUFFER_SIZE; i++){
    median += ledBuffer[i];
  }
  median /= BUFFER_SIZE;

  Serial.println(median);

  if(median > 0){
    ledValue(map(rssi, -60, -30, 0, 255));
  }else{
    ledValue(0);
  }

  pBLEScan->clearResults();
  delay(10); // Wait before next scan
}