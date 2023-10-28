#include <Arduino.h>
#include "BLEDevice.h"
#include "esp32-hal-adc.h"
#include "esp32-hal-gpio.h"

const int button1Pin = 4;
const int button2Pin = 5;
const int button3Pin = 14;
const int button4Pin = 27;
const int button5Pin = 26;
const int ledPin = 25;

const int yAxis = A1;
const int xAxis = A0;
const int buttoon = 5;

int forward = 0;
int backward = 0;
int left = 0;
int right = 0;

int turboTmp = 0;
int turbo = 0;

int lamp = 0;

uint8_t IDLE_COMMAND[] =                 {0x02, 0x5e, 0x69, 0x5a, 0x48, 0xff, 0x2a, 0x43, 0x8c, 0xa6, 0x80, 0xf8, 0x3e, 0x04, 0xe4, 0x5d};
uint8_t FORWARD_COMMAND[] =              {0x29, 0x60, 0x9c, 0x66, 0x48, 0x52, 0xcf, 0xf1, 0xb0, 0xf0, 0xcb, 0xb9, 0x80, 0x14, 0xbd, 0x2c};
uint8_t FORWARD_TURBO_COMMAND[] =        {0xe6, 0x55, 0x67, 0xda, 0x8e, 0x6c, 0x56, 0x0d, 0x09, 0xd3, 0x73, 0x3a, 0x7f, 0x47, 0xff, 0x06};
uint8_t BACKWARD_COMMAND[] =             {0x03, 0x20, 0x99, 0x09, 0xba, 0x9d, 0xa1, 0xc8, 0xb9, 0x86, 0x16, 0x3c, 0x6d, 0x48, 0x46, 0x55};
uint8_t BACKWARD_TURBO_COMMAND[] =       {0xce, 0xc2, 0xff, 0x1d, 0x7a, 0xcc, 0x16, 0x3c, 0xd1, 0x3b, 0x7e, 0x61, 0x53, 0xad, 0x5c, 0x45};
uint8_t FORWARD_LEFT_COMMAND[] =         {0x99, 0x28, 0xe5, 0x90, 0xdf, 0xe8, 0x21, 0x48, 0x5f, 0x41, 0x4f, 0xbb, 0x63, 0x3d, 0x5c, 0x4e};
uint8_t LEFT_COMMAND[] =                 {0x51, 0x38, 0x21, 0x12, 0x13, 0x5c, 0xcc, 0xdb, 0x46, 0xcf, 0x89, 0x21, 0xb7, 0x05, 0x49, 0x9a};
uint8_t FORWARD_RIGHT_COMMAND[] =        {0x0f, 0x2c, 0xe5, 0x66, 0x62, 0xd4, 0xfd, 0x9d, 0x32, 0xa4, 0x4f, 0x10, 0x2b, 0xf2, 0x0a, 0xa7};
uint8_t BACKWARD_LEFT_COMMAND[] =        {0x98, 0xce, 0x98, 0x1d, 0x58, 0xd1, 0x15, 0xaf, 0xe1, 0x19, 0x60, 0xbf, 0x46, 0x13, 0x92, 0x5c};
uint8_t RIGHT_COMMAND[] =                {0x1b, 0x57, 0x69, 0xcd, 0xf1, 0x3e, 0x8a, 0xb6, 0x27, 0x08, 0x0f, 0xf3, 0xce, 0xfc, 0x3b, 0xc0};
uint8_t BACKWARD_RIGHT_COMMAND[] =       {0xf2, 0x52, 0x0f, 0xba, 0x31, 0x44, 0xfb, 0x11, 0x46, 0x8f, 0xe0, 0x80, 0xc6, 0xc2, 0xc2, 0x3c};
uint8_t FORWARD_TURBO_LEFT_COMMAND[] =   {0x59, 0x23, 0x81, 0xc9, 0x43, 0xa4, 0x17, 0xca, 0x1b, 0xc3, 0xb5, 0x94, 0x00, 0xe0, 0xfc, 0x12};
uint8_t FORWARD_TURBO_RIGHT_COMMAND[] =  {0xfb, 0x97, 0x6f, 0xba, 0x04, 0xaf, 0x87, 0x02, 0x22, 0x26, 0xec, 0x50, 0xae, 0x82, 0xf8, 0xc4};
uint8_t BACKWARD_TURBO_LEFT_COMMAND[] =  {0xd5, 0x4a, 0xd5, 0x58, 0x57, 0xd3, 0x27, 0x74, 0x5f, 0x14, 0x1d, 0xd0, 0x0d, 0x67, 0x15, 0x95};
uint8_t BACKWARD_TURBO_RIGHT_COMMAND[] = {0x80, 0xdf, 0xb2, 0x16, 0x5f, 0x32, 0x60, 0xf1, 0xd9, 0x83, 0x77, 0x50, 0xf4, 0x3a, 0x43, 0xda};

static BLEUUID serviceUUID_BBurago("0000fff0-0000-1000-8000-00805f9b34fb");
static BLEUUID    charUUID_BBurago("0000fff1-0000-1000-8000-00805f9b34fb");

uint8_t BBurago_command[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


boolean switchedToBBurago = false;
static boolean doConnect = true;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;


int scanTime = 5; //In seconds
BLEScan* pBLEScan;




static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }
    void onDisconnect(BLEClient* pclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  Serial.print("Start scan");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  Serial.println(" - Connected to server");

  BLERemoteService* pRemoteService;
  pRemoteService = pClient->getService(serviceUUID_BBurago);

  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID_BBurago.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID_BBurago);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_BBurago.toString().c_str());
    pClient->disconnect();
    return false;
  }

  
  Serial.println(" - Found our characteristic");
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      if (advertisedDevice.haveServiceUUID()) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      }
      Serial.println("This One Done");


    }
};

uint8_t* getBrandbaseCommand(uint8_t turboOn, uint8_t forwardOn, uint8_t backwardOn, uint8_t leftOn, uint8_t rightOn) {
  if (!leftOn && !rightOn && !forwardOn && !backwardOn) {
    return IDLE_COMMAND;
  }

  if (turboOn) {
    if (forwardOn && !leftOn && !rightOn) {
      return FORWARD_TURBO_COMMAND;
    } else if (backwardOn && !leftOn && !rightOn) {
      return BACKWARD_TURBO_COMMAND;
    } else if (forwardOn && leftOn && !rightOn) {
      return FORWARD_TURBO_LEFT_COMMAND;
    } else if (forwardOn && !leftOn && rightOn) {
      return FORWARD_TURBO_RIGHT_COMMAND;
    } else if (backwardOn && leftOn && !rightOn) {
      return BACKWARD_TURBO_LEFT_COMMAND;
    } else if (backwardOn && !leftOn && rightOn) {
      return BACKWARD_TURBO_RIGHT_COMMAND;
    }
  } else {
    if (leftOn && !forwardOn && !backwardOn) {
      return LEFT_COMMAND;
    } else if (rightOn && !forwardOn && !backwardOn) {
      return RIGHT_COMMAND;
    } else if (forwardOn && !leftOn && !rightOn) {
      return FORWARD_COMMAND;
    } else if (backwardOn && !leftOn && !rightOn) {
      return BACKWARD_COMMAND;
    } else if (forwardOn && leftOn && !rightOn) {
      return FORWARD_LEFT_COMMAND;
    } else if (forwardOn && !leftOn && rightOn) {
      return FORWARD_RIGHT_COMMAND;
    } else if (backwardOn && leftOn && !rightOn) {
      return BACKWARD_LEFT_COMMAND;
    } else if (backwardOn && !leftOn && rightOn) {
      return BACKWARD_RIGHT_COMMAND;
    }
  }
}


uint8_t* getBBuragoCommand(uint8_t turboOn, uint8_t forwardOn, uint8_t backwardOn, uint8_t leftOn, uint8_t rightOn, uint8_t lampOn) {
  BBurago_command[1] = 0;
  BBurago_command[2] = 0;
  BBurago_command[3] = 0;
  BBurago_command[4] = 0;
  BBurago_command[5] = 0;
  BBurago_command[6] = 0;


  if (turboOn)
    BBurago_command[6] = 1;

  if (lampOn)
    BBurago_command[5] = 1;

  if (forwardOn)
    BBurago_command[1] = 1;
  else if (backwardOn)
    BBurago_command[2] = 1;

  if (rightOn)
    BBurago_command[4] = 1;
  else if (leftOn)
    BBurago_command[3] = 1;

  return BBurago_command;
}

void setup() {
  Serial.begin(115200);
  pinMode(buttoon, INPUT_PULLUP);

  BLEDevice::init("");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);

  delay(2000);
}


int a = 0;
void loop() {
  long valueY = analogRead(yAxis);
  long valueX = analogRead(xAxis);
  int buttonValue = digitalRead(buttoon);

  if(valueX < 3000){
    left = 1;
    right = 0;
  } else if(valueX > 4000){
    right = 1;
    left = 0;
  }else{
    right = 0;
    left = 0;
  }

  if(valueY < 3000){
    backward = 1;
    forward = 0;
  } else if(valueY > 4000){
    backward = 0;
    forward = 1;
  }else{
    backward = 0;
    forward = 0;
  }
  Serial.println(valueX);
  Serial.println(valueY);
  Serial.println(left);
  Serial.println(right);
  Serial.println(forward);
  Serial.println(backward);
  Serial.println(buttonValue);
  Serial.println(turbo);

  turbo = 0;

  if (turboTmp == 0 && 0 == 1) {
    if (turbo == 0)
      turbo = 1;
    else
      turbo = 0;
  }

  if(1-buttonValue){
    backward = 1;
    forward = 0;
    turbo = 0;
  }

  turboTmp = 0;

  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  if (connected) {
    pRemoteCharacteristic->writeValue(getBBuragoCommand(turbo, forward, backward, left, right, 1-buttonValue), 8);
  } else if (doScan) {
    Serial.println("REconnect +===");
    doConnect = true;
  }

  delay(5);
}
