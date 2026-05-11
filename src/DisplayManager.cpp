#include "DisplayManager.h"
#include <Wire.h>

void DisplayManager::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    u8g2.setI2CAddress(OLED_ADDR << 1);
    u8g2.begin();
    showWaiting();
}

void DisplayManager::showWaiting() {
    showing = false;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(0, 24);
    u8g2.print(" Waiting");
    u8g2.setCursor(0, 48);
    u8g2.print("  for IR...");
    u8g2.sendBuffer();
}

void DisplayManager::showCode(uint8_t addr, uint8_t addrInv, uint8_t cmd, uint8_t cmdInv) {
    showing = true;
    lastShowTime = millis();

    char buf[20];
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);

    // Line 1: address original + inverse
    snprintf(buf, sizeof(buf), "A:%02X %02X", addr, addrInv);
    u8g2.setCursor(0, 24);
    u8g2.print(buf);

    // Line 2: command original + inverse
    snprintf(buf, sizeof(buf), "C:%02X %02X", cmd, cmdInv);
    u8g2.setCursor(0, 48);
    u8g2.print(buf);

    u8g2.sendBuffer();
}

void DisplayManager::update() {
    if (showing && (millis() - lastShowTime > DISPLAY_HOLD_TIME)) {
        showWaiting();
    }
}

bool DisplayManager::isShowingCode() {
    return showing;
}
