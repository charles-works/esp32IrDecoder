#include "DisplayManager.h"
#include <Wire.h>

void DisplayManager::begin() {
    // Explicitly initialise I2C with the pins defined in config.h
    Wire.begin(OLED_SDA, OLED_SCL);
    u8g2.setI2CAddress(OLED_ADDR << 1);  // U8g2 expects 8-bit shifted address
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

void DisplayManager::showCode(uint16_t addr, uint8_t cmd) {
    showing = true;
    lastShowTime = millis();
    currentAddr  = addr;
    currentCmd   = cmd;

    char buf[20];
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);

    snprintf(buf, sizeof(buf), "Addr:0x%04X", addr);
    u8g2.setCursor(0, 24);
    u8g2.print(buf);

    snprintf(buf, sizeof(buf), "Cmd: 0x%02X", cmd);
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
