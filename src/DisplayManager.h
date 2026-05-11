#pragma once
#include <U8g2lib.h>
#include "config.h"

class DisplayManager {
public:
    void begin();
    void showWaiting();
    void showCode(uint16_t addr, uint8_t cmd);
    void update();   // must be called in loop; handles 1-minute timeout
    bool isShowingCode();

private:
    // HW I2C constructor: (rotation, reset, SCL, SDA)
    U8G2_SSD1315_128X64_NONAME_F_HW_I2C u8g2 = U8G2_SSD1315_128X64_NONAME_F_HW_I2C(
        U8G2_R0, /* reset=*/ U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

    bool     showing      = false;
    unsigned long lastShowTime = 0;
    uint16_t currentAddr  = 0;
    uint8_t  currentCmd   = 0;
};
