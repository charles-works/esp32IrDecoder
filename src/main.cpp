#include <Arduino.h>
#include "config.h"
#include "IRDecoder.h"
#include "DisplayManager.h"

IRDecoder irDecoder;
DisplayManager displayManager;

void setup() {
    Serial.begin(115200);
    irDecoder.begin(IR_RECEIVER_PIN);
    displayManager.begin();
    displayManager.showWaiting();
    Serial.println("Ready. Waiting for NEC IR signal...");
}

void loop() {
    irDecoder.update();

    if (irDecoder.available()) {
        uint32_t raw = irDecoder.getRawCode();
        uint8_t addr    = raw & 0xFF;
        uint8_t addrInv = (raw >> 8)  & 0xFF;
        uint8_t cmd     = (raw >> 16) & 0xFF;
        uint8_t cmdInv  = (raw >> 24) & 0xFF;

        Serial.printf("NEC: A:%02X %02X  C:%02X %02X\n", addr, addrInv, cmd, cmdInv);
        displayManager.showCode(addr, addrInv, cmd, cmdInv);
        irDecoder.resume();
    }

    displayManager.update();
}
