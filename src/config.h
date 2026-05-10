#pragma once

// Pin definitions
#define IR_RECEIVER_PIN   4
#define OLED_SDA          8
#define OLED_SCL          9

// OLED settings (SSD1315)
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT    64
#define OLED_ADDR       0x3C      // 0x3C or 0x3D depending on SA0 pin

// Display hold time (milliseconds)
#define DISPLAY_HOLD_TIME  60000   // 1 minute