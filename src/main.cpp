/*********
  Testing BLE server for BAK device sensor

*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Arduino.h>


//BLE server name
#define bleServerName "BLE BAK Test(ESP32)"

BLEServer *pServer = NULL;//added

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

bool deviceConnected = false;
bool oldDeviceConnected = false;//added

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "3d7b0044-fbc5-4284-b0f5-710049ec62c9"

// Temperature Characteristic and Descriptor
  BLECharacteristic bmeDataSetCharacteristics("6518ea87-cc4e-469e-8a7d-c7d99d822fbe", BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  BLEDescriptor bmeDataSetDescriptor(BLEUUID((uint16_t)0x2902));

// Messparameter Characteristic and Descriptor
BLECharacteristic bmeMessParamCharacteristics("afd77861-8032-43a2-8369-a4f991913238", BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor bmeMessaParamDescriptor(BLEUUID((uint16_t)0x2903));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void checkToReconnect() //added
{
  // disconnected so advertise
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Disconnected: start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connected so reset boolean control
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    Serial.println("Reconnected");
    oldDeviceConnected = deviceConnected;
  }
}

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  
  // Messparameter
  bmeService->addCharacteristic(&bmeMessParamCharacteristics);
  //bmeMessaParamDescriptor.setValue("BAK Param Characteristic");
  //bmeMessParamCharacteristics.addDescriptor(new BLE2902());



  // DataSet
    bmeService->addCharacteristic(&bmeDataSetCharacteristics);
    //bmeDataSetDescriptor.setValue("Testing values from ESP32");
    //bmeDataSetCharacteristics.addDescriptor(&bmeDataSetDescriptor);

  
  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

  checkToReconnect();
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {

      //Notify Messparameter reading from BME sensor
      uint8_t array[26] = {0x42,0x41,0x4b,0x2d,0x32,0x33,0x30,0x31,0x00,0x01,0x13,0x01,0x17,0x0a,0x19,0x0b,0x2d,0x01,0x01,0x01,0x5e,0x00,0x26,0x02,0x58,0x00};
      bmeMessParamCharacteristics.setValue(array,26);
      bmeMessParamCharacteristics.notify();   
      Serial.print("Sending values for Messparameter:");
      String strr = (char*)array;
      Serial.print(strr);
      Serial.println("\n");

      //Notify dataSet reading from BME sensor
        uint8_t array2[6] = {0x01,0x40,0x00,0x20,0x02,0x12};
        bmeDataSetCharacteristics.setValue(array2, 6);
        bmeDataSetCharacteristics.notify();
        Serial.print("Sending values of dataSet ");
        String str = (char*)array2;
        Serial.print(str);
        Serial.print("\n");
      
      
      lastTime = millis();
    }
  } else {
    //no connected
  }
}