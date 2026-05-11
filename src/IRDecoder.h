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

private:
    static void IRAM_ATTR isrHandler();
    bool decodeNEC(const uint16_t* data, uint8_t count);

    static IRDecoder* instance;

    // --- Hardware ---
    uint8_t irPin = 0;

    // --- ISR edge capture ---
    // Stores per-edge durations (µs between consecutive edges)
    volatile uint32_t lastEdgeUs   = 0;     // micros() of most recent edge
    volatile uint16_t rawData[128];         // edge-duration ring buffer
    volatile uint8_t  rawIndex     = 0;     // next write position
    volatile bool     frameActive  = false; // true once first edge of a frame seen

    // --- Frame-complete detection ---
    // A full NEC frame (leader + 32 data bits + repeat) finishes within ~108 ms.
    // If no new edge arrives within this window the frame is assumed complete.
    static constexpr uint32_t FRAME_TIMEOUT_US = 110000; // 110 ms

    // --- Decoded output ---
    bool     codeReady      = false;
    uint16_t decodedAddress = 0;
    uint8_t  decodedCommand = 0;
    uint32_t decodedRaw     = 0;

    // --- NEC timing constants (microseconds) ---
    static constexpr uint16_t NEC_START_LOW_MIN  = 8000;
    static constexpr uint16_t NEC_START_LOW_MAX  = 10000;
    static constexpr uint16_t NEC_START_HIGH_MIN = 4000;
    static constexpr uint16_t NEC_START_HIGH_MAX = 5000;
    static constexpr uint16_t NEC_BIT_LOW        = 560;
    static constexpr uint16_t NEC_BIT_LOW_TOL    = 200;
    static constexpr uint16_t NEC_BIT_0_HIGH     = 560;
    static constexpr uint16_t NEC_BIT_1_HIGH     = 1690;
    static constexpr uint16_t NEC_BIT_HIGH_TOL   = 300;
};
