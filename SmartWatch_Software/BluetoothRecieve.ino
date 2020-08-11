//BLE related variables
//UUID's for the services used by the android app (change as you please if you're building this yourself, just match them in the android app)
static BLEUUID serviceUUID("d3bde760-c538-11ea-8b6e-0800200c9a66");
static BLEUUID    charUUID("d3bde760-c538-11ea-8b6e-0800200c9a67");

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEClient*  pClient;
static boolean connected = false;


/* Callbacks */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      printDebug("BLE Advertised Device found: " + String(advertisedDevice.toString().c_str()));
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        printDebug("%%%%%%%%%%%% Device Found %%%%%%%%%%%%%%%%%");
      }
    }
};

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
      connected = true;
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      printDebug("%%%%%%%%%% Device has Disconnected %%%%%%%%");
    }
};

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


/* BLE interfacing functions
*/
// Start by scanning for the device we want to communicate with a FreeRTOS task
void initBLE() {
  if (!connected) {
    xTaskCreatePinnedToCore( xFindDevice, "FIND_DEVICE", 4096, (void *) 1 , tskIDLE_PRIORITY, &xConnect, 0 );
  }
}

void xFindDevice(void * pvParameters ) {
  BLEDevice::init("");
  printDebug("%%% Find Device Task Launched %%%");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(40);           //I've found these interval and window values to work the best with android, but others may be better.
  pBLEScan->setWindow(39);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(8);

#ifdef DEBUG
  //if myDevice is not null then we have found the device
  if (myDevice) {
    Serial.println("%%% Device Found %%%");
    xTaskCreatePinnedToCore( formConnection, "FIND_DEVICE", 4096, (void *) 1 , tskIDLE_PRIORITY, &xConnect, 0 );
  } else {
    Serial.println("%%%% Device Not Found %%%");
  }
#endif
  vTaskDelete(NULL);
}

/* Opens connection to the server and allows us to access the characteristic that data
    is transmitted to, this should be called after xFindDevice completes, however it
    cannot be run in a seperate thread due to I2C
*/
void formConnection(void * pvParameters) {
  //if for some this function is called before we find the device then we need to
  //do the scan again and make sure that we have a device
  if (!myDevice) {
    xFindDevice((void*) 1);
  }

  //create a client to communicate to the server through
  pClient = BLEDevice::createClient();

  //set callbacks for client, if it disconnects we need to know,
  //also we don't consider the device to be connected unless the client is connected
  pClient->setClientCallbacks(new MyClientCallback());

  //check that device is found again
  //attempting to connect to a null device will cause the device to crash
  if (myDevice) {
    pClient->connect(myDevice);
  } else {
    vTaskDelete(NULL);
  }

  //obtain a reference to the desired service
  //and check for null reference, this indicates failure
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (!pRemoteService) {
    pClient->disconnect();
    printDebug("%%%% Could not obtain remote service");
    vTaskDelete(NULL);
  }

  //second verse same as the first, but for the characteristic
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (!pRemoteCharacteristic) {
    pClient->disconnect();
    printDebug("%%%% Could not obtain remote characteristic");
    vTaskDelete(NULL);
  }

  //register to be notified of changes in the characteristic
  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic -> registerForNotify(notifyCallback);
  }
  vTaskDelete(NULL);
}
