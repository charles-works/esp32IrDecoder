#include <Arduino.h>
#include "config.h"
#include "IRDecoder.h"
#include "DisplayManager.h"
#include "BLEManager.h"

IRDecoder    irDecoder;
DisplayManager displayManager;
BLEManager   bleManager;

// NEC wire format is LSB-first per byte; reverse each byte to get
// the conventional MSB-first representation used by most IR tools.
static uint8_t bitrev8(uint8_t b) {
    b = (uint8_t)((b & 0xF0) >> 4 | (b & 0x0F) << 4);
    b = (uint8_t)((b & 0xCC) >> 2 | (b & 0x33) << 2);
    b = (uint8_t)((b & 0xAA) >> 1 | (b & 0x55) << 1);
    return b;
}

void setup() {
    Serial.begin(115200);
    irDecoder.begin(IR_RECEIVER_PIN);
    displayManager.begin();
    bleManager.begin();
    displayManager.showWaiting();
    Serial.println("Ready. Waiting for NEC IR signal...");
}

void loop() {
    irDecoder.update();

    if (irDecoder.available()) {
        uint32_t raw = irDecoder.getRawCode();
        uint8_t addr    = bitrev8((uint8_t)(raw & 0xFF));
        uint8_t addrInv = bitrev8((uint8_t)((raw >> 8)  & 0xFF));
        uint8_t cmd     = bitrev8((uint8_t)((raw >> 16) & 0xFF));
        uint8_t cmdInv  = bitrev8((uint8_t)((raw >> 24) & 0xFF));

        Serial.printf("NEC: A:%02X %02X  C:%02X %02X\n", addr, addrInv, cmd, cmdInv);
        displayManager.showCode(addr, addrInv, cmd, cmdInv);
        bleManager.sendIRCode(addr, addrInv, cmd, cmdInv);
        irDecoder.resume();
    }

    displayManager.update();
}
