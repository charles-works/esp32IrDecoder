#pragma once
#include <Arduino.h>

class IRDecoder {
public:
    void begin(uint8_t pin);
    void update();
    bool available();
    void resume();
    uint16_t getAddress();
    uint8_t getCommand();
    uint32_t getRawCode();

    static void IRAM_ATTR isrHandler();

private:
    bool decodeNEC(uint16_t* data, uint8_t count);   // <-- now takes parameters

    static IRDecoder* instance;

    uint8_t irPin;
    volatile uint32_t lastTime = 0;
    volatile uint16_t rawData[100];
    volatile uint8_t rawIndex = 0;
    volatile bool newData = false;

    bool codeReady = false;
    uint16_t decodedAddress;
    uint8_t decodedCommand;
    uint32_t decodedRaw;

    // NEC timing constants (microseconds)
    const uint16_t NEC_START_LOW_MIN  = 8000;
    const uint16_t NEC_START_LOW_MAX  = 10000;
    const uint16_t NEC_START_HIGH_MIN = 4000;
    const uint16_t NEC_START_HIGH_MAX = 5000;
    const uint16_t NEC_BIT_LOW        = 560;
    const uint16_t NEC_BIT_LOW_TOL    = 200;
    const uint16_t NEC_BIT_0_HIGH     = 560;
    const uint16_t NEC_BIT_1_HIGH     = 1690;
    const uint16_t NEC_BIT_HIGH_TOL   = 300;
};