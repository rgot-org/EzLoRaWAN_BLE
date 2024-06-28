#include "EzLoRaWAN_BLE.h"
#include "ByteArrayUtils.h"
#include "helper.h"

/********************************************
* BLE callback when client connect/disconnect
* reset the esp32 when disconnecting
********************************************/
class MyServerCallbacks : public BLEServerCallbacks, EzLoRaWAN_BLE
{
    void onConnect(BLEServer* pServer) { 
        ESP_LOGI(TAG,"BLE client connected");
    }

    void onDisconnect(BLEServer* pServer) { 
         ESP_LOGI(TAG, "BLE client disconnected");
      rebootESP32();
     }
};
/********************************
 * BLE callback when BLE client (the phone) sends data to characteristic
 *********************************/
class MyCallbacks : public BLECharacteristicCallbacks, EzLoRaWAN_BLE
{
    void onWrite(BLECharacteristic* pCharacteristic)
    {
        BLEUUID myUUID = pCharacteristic->getUUID();
        EzLoRaWAN* ttn = EzLoRaWAN::getInstance();
        ttn->restoreKeys();

        /*OTAA*/
        if (myUUID.equals(BLEUUID::fromString(CHARACTERISTIC_DEVEUI)))
        {
            ESP_LOGI(TAG,"DevEUI");
            byte* value = pCharacteristic->getData();
            ByteArrayUtils::swapBytes(value, 8);
            ttn->setDevEui(value);
        }
        if (myUUID.equals(BLEUUID::fromString(CHARACTERISTIC_APPEUI)))
        {
            ESP_LOGI(TAG,"AppEUI");
            byte* value = pCharacteristic->getData();
            ByteArrayUtils::swapBytes(value, 8);
            ttn->setAppEui(value);
        }
        if (myUUID.equals(BLEUUID::fromString(CHARACTERISTIC_APPKEY)))
        {
            ESP_LOGI(TAG,"AppKey");
            byte* value = pCharacteristic->getData();
            ttn->setAppKey(value);
        }
        /*ABP*/
        if (myUUID.equals(BLEUUID::fromString(CHARACTERISTIC_DEV_ADDR)))
        {
            ESP_LOGI(TAG,"devADDR");
        }
        if (myUUID.equals(BLEUUID::fromString(CHARACTERISTIC_NWKSKEY)))
        {
            ESP_LOGI(TAG,"NwkSKey");
        }
        if (myUUID.equals(BLEUUID::fromString(CHARACTERISTIC_APP_SKEY)))
        {
            ESP_LOGI(TAG,"AppSKey");
        }

        ttn->saveKeys();
    }
};

bool EzLoRaWAN_BLE::begin(std::string bt_name)
{
    EzLoRaWAN* ttn = EzLoRaWAN::getInstance();
    ttn->restoreKeys();
    if (bt_name == "")
    {
        std::string nameDev(ttn->getDevEui(true).c_str());
        bt_name.append("RGOT_").append(nameDev);
    }

    BLEDevice::init(bt_name.c_str());
    ESP_LOGI(TAG, "BLE Begin server: %s", bt_name.c_str());
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService* pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic* pCharacteristicAppKey = pService->createCharacteristic(
        CHARACTERISTIC_APPKEY, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    BLECharacteristic* pCharacteristicDevEUI = pService->createCharacteristic(
        CHARACTERISTIC_DEVEUI, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    BLECharacteristic* pCharacteristicAppEUI = pService->createCharacteristic(
        CHARACTERISTIC_APPEUI, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    BLECharacteristic* pCharacteristicDevAddr = pService->createCharacteristic(
        CHARACTERISTIC_DEV_ADDR, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    BLECharacteristic* pCharacteristicNwkSKey = pService->createCharacteristic(
        CHARACTERISTIC_NWKSKEY, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    BLECharacteristic* pCharacteristicAppSKey = pService->createCharacteristic(
        CHARACTERISTIC_APP_SKEY, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    pCharacteristicDevEUI->setCallbacks(new MyCallbacks());
    pCharacteristicAppEUI->setCallbacks(new MyCallbacks());
    pCharacteristicAppKey->setCallbacks(new MyCallbacks());
    pCharacteristicDevAddr->setCallbacks(new MyCallbacks());
    pCharacteristicNwkSKey->setCallbacks(new MyCallbacks());
    pCharacteristicAppSKey->setCallbacks(new MyCallbacks());
    byte buf[33];

    int len = ttn->getDevEui(buf);
    pCharacteristicDevEUI->setValue(buf, len);

    len = ttn->getAppEui(buf);
    pCharacteristicAppEUI->setValue(buf, len);

    len = ttn->getPartialAppKey(buf);
    pCharacteristicAppKey->setValue(buf, len);

    pService->start();
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    return true;
}

bool EzLoRaWAN_BLE::stop()
{
    ESP_LOGI(TAG, "stop BLE");
    BLEDevice::deinit();
    return true;
}

void EzLoRaWAN_BLE::rebootESP32() {
    ESP.restart();
}

bool EzLoRaWAN_BLE::getInitialized()
{
    return BLEDevice::getInitialized();
}
EzLoRaWAN_BLE::EzLoRaWAN_BLE() {}
void EzLoRaWAN_BLE::init() {}
