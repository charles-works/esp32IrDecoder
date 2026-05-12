#include "BLEManager.h"

// ── ServerCallbacks (defined outside the class to avoid ODR issues) ──

void BLEManager::ServerCallbacks::onConnect(BLEServer*) {
    m_owner->deviceConnected = true;
    Serial.println("[BLE] Client connected");
}

void BLEManager::ServerCallbacks::onDisconnect(BLEServer*) {
    m_owner->deviceConnected = false;
    Serial.println("[BLE] Client disconnected");
    // Restart advertising so another client can connect
    BLEDevice::startAdvertising();
}

// ── BLEManager public API ──

void BLEManager::begin() {
    BLEDevice::init(BLE_DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));

    BLEService* pService = pServer->createService(BLE_SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        BLE_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // Helps iPhone connection stability
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] Advertising started as \"" BLE_DEVICE_NAME "\"");
    initialized = true;
}

void BLEManager::sendIRCode(uint8_t addr, uint8_t addrInv,
                            uint8_t cmd,  uint8_t cmdInv) {
    if (!initialized || !deviceConnected) return;

    // Compact human-readable string: "A:XX YY C:XX YY"
    char buf[24];
    snprintf(buf, sizeof(buf), "A:%02X %02X C:%02X %02X", addr, addrInv, cmd, cmdInv);
    pCharacteristic->setValue((uint8_t*)buf, strlen(buf));
    pCharacteristic->notify();
}

bool BLEManager::isConnected() {
    return deviceConnected;
}
