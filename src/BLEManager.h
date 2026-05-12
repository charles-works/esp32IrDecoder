#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"

class BLEManager {
public:
    void begin();
    void sendIRCode(uint8_t addr, uint8_t addrInv, uint8_t cmd, uint8_t cmdInv);
    bool isConnected();

private:
    BLEServer*         pServer         = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    bool               deviceConnected = false;
    bool               initialized     = false;

    class ServerCallbacks : public BLEServerCallbacks {
    public:
        ServerCallbacks(BLEManager* owner) : m_owner(owner) {}
        void onConnect(BLEServer*) override;
        void onDisconnect(BLEServer*) override;
    private:
        BLEManager* m_owner;
    };
};
