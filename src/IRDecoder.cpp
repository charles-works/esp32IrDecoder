#include "IRDecoder.h"

IRDecoder* IRDecoder::instance = nullptr;

void IRDecoder::begin(uint8_t pin) {
    irPin = pin;
    instance = this;
    pinMode(irPin, INPUT);
    lastTime = micros();
    attachInterrupt(digitalPinToInterrupt(irPin), isrHandler, CHANGE);
}

void IRAM_ATTR IRDecoder::isrHandler() {
    uint32_t now = micros();
    uint16_t duration = now - instance->lastTime;
    // filter very short glitches
    if (duration > 100) {
        instance->rawData[instance->rawIndex] = duration;
        instance->rawIndex = (instance->rawIndex + 1) % 100;
    }
    instance->lastTime = now;
    instance->newData = true;
}

void IRDecoder::update() {
    if (newData) {
        noInterrupts();
        newData = false;
        // copy the circular buffer locally
        uint16_t data[100];
        uint8_t count = rawIndex;
        memcpy(data, (const void*)rawData, count * sizeof(uint16_t));
        rawIndex = 0;
        interrupts();

        if (decodeNEC(data, count)) {
            codeReady = true;
        }
    }
}

bool IRDecoder::decodeNEC(uint16_t* data, uint8_t count) {
    if (count < 3) return false;

    // search for the start pulse pair
    int startIdx = -1;
    for (int i = 0; i < count - 1; i++) {
        if (data[i] >= NEC_START_LOW_MIN && data[i] <= NEC_START_LOW_MAX &&
            data[i+1] >= NEC_START_HIGH_MIN && data[i+1] <= NEC_START_HIGH_MAX) {
            startIdx = i;
            break;
        }
    }
    if (startIdx == -1) return false;

    int bitIdx = startIdx + 2;
    if (bitIdx + 64 > count) return false;   // need 32*2 durations

    uint32_t raw = 0;
    for (int b = 0; b < 32; b++) {
        uint16_t lowDur  = data[bitIdx++];
        uint16_t highDur = data[bitIdx++];

        // low pulse must be close to 560 µs
        if (lowDur < NEC_BIT_LOW - NEC_BIT_LOW_TOL ||
            lowDur > NEC_BIT_LOW + NEC_BIT_LOW_TOL) {
            return false;
        }

        // decode bit based on high duration
        if (highDur >= NEC_BIT_1_HIGH - NEC_BIT_HIGH_TOL &&
            highDur <= NEC_BIT_1_HIGH + NEC_BIT_HIGH_TOL) {
            raw |= (1UL << b);             // bit = 1
        } else if (highDur >= NEC_BIT_0_HIGH - NEC_BIT_HIGH_TOL &&
                   highDur <= NEC_BIT_0_HIGH + NEC_BIT_HIGH_TOL) {
            // bit = 0, nothing to set
        } else {
            return false;                   // invalid timing
        }
    }

    // extract bytes (LSB first transmission)
    uint8_t addr     = raw & 0xFF;
    uint8_t addr_inv = (raw >> 8) & 0xFF;
    uint8_t cmd      = (raw >> 16) & 0xFF;
    uint8_t cmd_inv  = (raw >> 24) & 0xFF;

    // check standard NEC (complement check)
    if ((addr ^ addr_inv) == 0xFF && (cmd ^ cmd_inv) == 0xFF) {
        decodedAddress = addr;                  // 8-bit address
    } else {
        decodedAddress = (addr << 8) | addr_inv; // extended 16-bit address
    }
    decodedCommand = cmd;
    decodedRaw     = raw;

    return true;
}

bool IRDecoder::available() {
    return codeReady;
}

uint16_t IRDecoder::getAddress() {
    return decodedAddress;
}

uint8_t IRDecoder::getCommand() {
    return decodedCommand;
}

uint32_t IRDecoder::getRawCode() {
    return decodedRaw;
}

void IRDecoder::resume() {
    codeReady = false;
}