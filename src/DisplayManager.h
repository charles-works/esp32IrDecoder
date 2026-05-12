#pragma once
#include <U8g2lib.h>
#include "config.h"

class DisplayManager {
public:
    void begin();
    void showWaiting();
    void showCode(uint8_t addr, uint8_t addrInv, uint8_t cmd, uint8_t cmdInv);
    void update();   // must be called in loop; handles 1-minute timeout
    bool isShowingCode();

private:
    U8G2_SSD1315_128X64_NONAME_F_HW_I2C u8g2 = U8G2_SSD1315_128X64_NONAME_F_HW_I2C(
        U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

    bool          showing      = false;
    unsigned long lastShowTime = 0;
};
