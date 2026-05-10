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
        uint16_t addr = irDecoder.getAddress();
        uint8_t  cmd  = irDecoder.getCommand();

        Serial.printf("New code -> Addr: 0x%04X, Cmd: 0x%02X\n", addr, cmd);
        displayManager.showCode(addr, cmd);
        irDecoder.resume();
    }

    displayManager.update();
}