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

BLEScan* pBLEScan;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {


  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (!advertisedDevice.haveManufacturerData()) return;
    // Serial.print("Service Data UUID: ");
    // Serial.println(advertisedDevice.getServiceUUID());
    // if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID("00000000-0000-0000-0000-000000000000"))) {
    //   // Found the device with the target UUID
    //   BLEDevice::getScan()->stop();
    //   Serial.println("we found the target device");
    //   device_found = true;
    //   // Proceed with connection
    // }

    String data = advertisedDevice.getManufacturerData();

    if (data.length() < 25) return;

    const uint8_t* payload = (const uint8_t*)data.c_str();
    int len = data.length();

    // Apple Company ID (0x004C)
    if (payload[0] != 0x4C || payload[1] != 0x00) return;

    // iBeacon prefix
    if (payload[2] != 0x02 || payload[3] != 0x15) return;

    // Extract UUID
    char uuidStr[37];
    sprintf(uuidStr,
            "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            payload[4], payload[5], payload[6], payload[7],
            payload[8], payload[9],
            payload[10], payload[11],
            payload[12], payload[13],
            payload[14], payload[15], payload[16], payload[17],
            payload[18], payload[19]);

    BLEUUID beaconUUID(uuidStr);

    //String ManufData = advertisedDevice.toString().c_str();

    // Serial.print("ManufData: ");
    // Serial.println(ManufData);
    // Serial.print("UUID: ");

    // Serial.println(ManufData.substring(69, 101));

    if (beaconUUID.equals(targetBeaconUUID)) {
      device_found = true;
      Serial.println("🎯 iBeacon UUID MATCH");
      BLEDevice::getScan()->stop();

    }

    // // We have found a device, let us now see if it contains the service we are looking for.
    // if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

    //   // BLEDevice::getScan()->stop();
    //   // myDevice = new BLEAdvertisedDevice(advertisedDevice);
    //   // doConnect = true;
    //   // doScan = true;

    //   device_found = true;
    //   Serial.println("Device Found");

    // }  // Found our server
    // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
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

  Serial.print(targetBeaconUUID.toString().c_str());
}
void loop() {
  device_found = false;
  // put your main code here, to run repeatedly:
  BLEScanResults* foundDevices = pBLEScan->start(scanTime, false);
  for (int i = 0; i < foundDevices->getCount(); i++)
  {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);

    Serial.print("Device Name: ");
    Serial.println(device.getName());
    Serial.print("Manufacturer Data: ");
    Serial.println(device.getManufacturerData());

    String data = device.getManufacturerData();

    String input = data;
    String hexString = "";

    for (int i = 0; i < input.length(); i++) {
      char c = input.charAt(i);
      char hexChar[3];
      sprintf(hexChar, "%02X", c); // Format as uppercase hex with leading zero
      hexString += hexChar;
    }

    String deviceUUID = hexString.substring(8, 40);

    Serial.println("Device UUID: " + deviceUUID);

    if (deviceUUID == targetUUID) {
      int rssi = device.getRSSI();
      Serial.println(rssi);
      if (rssi > RSSI_THRESHOLD){
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("write LED HIGH");
      }
      else{
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("write LED LOW");
      }
    }
  }
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}