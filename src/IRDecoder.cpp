#include "IRDecoder.h"

IRDecoder* IRDecoder::instance = nullptr;

// ──────────────────────────── public API ────────────────────────────

void IRDecoder::begin(uint8_t pin) {
    irPin = pin;
    instance = this;
    pinMode(irPin, INPUT_PULLUP);          // 1838 modules typically need pull-up on idle-high
    lastEdgeUs = micros();
    attachInterrupt(digitalPinToInterrupt(irPin), isrHandler, CHANGE);
}

void IRDecoder::update() {
    // 1. Check whether the frame has timed out (no new edges for >= FRAME_TIMEOUT_US)
    noInterrupts();
    bool     active    = frameActive;
    uint32_t lastUs    = lastEdgeUs;
    uint8_t  count     = rawIndex;
    bool     timedOut  = active && ((micros() - lastUs) >= FRAME_TIMEOUT_US);
    if (!timedOut) {
        interrupts();
        return;                           // frame still in progress or nothing started
    }

    // 2. Snapshot the edge buffer and reset for next frame
    uint16_t snapshot[128];
    memcpy(snapshot, (const void*)rawData, count * sizeof(uint16_t));
    rawIndex    = 0;
    frameActive = false;
    interrupts();

    // 3. Attempt NEC decode on the collected edges
    Serial.printf("[IR] frame captured: %u edges\n", count);
    if (count >= 4 && decodeNEC(snapshot, count)) {
        codeReady = true;
        Serial.printf("[IR] NEC decoded -> Addr: 0x%04X  Cmd: 0x%02X  Raw: 0x%08lX\n",
                      decodedAddress, decodedCommand, (unsigned long)decodedRaw);
    } else if (count > 0) {
        Serial.printf("[IR] decode failed (%u edges)\n", count);
    }
}

bool IRDecoder::available()  { return codeReady; }
void IRDecoder::resume()     { codeReady = false; }
uint16_t IRDecoder::getAddress()  { return decodedAddress; }
uint8_t  IRDecoder::getCommand()  { return decodedCommand; }
uint32_t IRDecoder::getRawCode()  { return decodedRaw; }

// ──────────────────────────── ISR ───────────────────────────────────

void IRAM_ATTR IRDecoder::isrHandler() {
    uint32_t now = micros();
    uint16_t duration = now - instance->lastEdgeUs;

    // Filter sub-100 µs glitches (e.g. EMI / bounce)
    if (duration > 100) {
        uint8_t idx = instance->rawIndex;
        if (idx < sizeof(instance->rawData) / sizeof(instance->rawData[0])) {
            instance->rawData[idx] = duration;
            instance->rawIndex = idx + 1;
        }
        // else: buffer overflow — silently drop; frame will likely fail decode
    }
    instance->lastEdgeUs  = now;
    instance->frameActive = true;
}

// ──────────────────────────── NEC decode ────────────────────────────

bool IRDecoder::decodeNEC(const uint16_t* data, uint8_t count) {
    if (count < 66) return false;         // need leader (2) + 32 bits × 2 = 66 minimum

    // 1. Find the leader: long LOW followed by long HIGH
    int startIdx = -1;
    for (int i = 0; i < count - 1; i++) {
        if (data[i]  >= NEC_START_LOW_MIN  && data[i]  <= NEC_START_LOW_MAX &&
            data[i+1] >= NEC_START_HIGH_MIN && data[i+1] <= NEC_START_HIGH_MAX) {
            startIdx = i;
            break;
        }
    }
    if (startIdx < 0) return false;

    // 2. Decode 32 bits (each bit = LOW ~560µs + variable HIGH)
    int bitIdx = startIdx + 2;
    if (bitIdx + 64 > count) return false; // not enough edges for 32 bits

    uint32_t raw = 0;
    for (int b = 0; b < 32; b++) {
        uint16_t lowDur  = data[bitIdx++];
        uint16_t highDur = data[bitIdx++];

        if (lowDur < (NEC_BIT_LOW - NEC_BIT_LOW_TOL) ||
            lowDur > (NEC_BIT_LOW + NEC_BIT_LOW_TOL)) {
            return false;                  // bad LOW pulse width
        }

        if (highDur >= (NEC_BIT_1_HIGH - NEC_BIT_HIGH_TOL) &&
            highDur <= (NEC_BIT_1_HIGH + NEC_BIT_HIGH_TOL)) {
            raw |= (1UL << b);            // bit = 1
        } else if (highDur >= (NEC_BIT_0_HIGH - NEC_BIT_HIGH_TOL) &&
                   highDur <= (NEC_BIT_0_HIGH + NEC_BIT_HIGH_TOL)) {
            // bit = 0 — nothing to set
        } else {
            return false;                  // bad HIGH pulse width
        }
    }

    // 3. Extract bytes (LSB-first transmission)
    uint8_t addr     = raw & 0xFF;
    uint8_t addr_inv = (raw >> 8)  & 0xFF;
    uint8_t cmd      = (raw >> 16) & 0xFF;
    uint8_t cmd_inv  = (raw >> 24) & 0xFF;

    // 4. Address decoding: complement check distinguishes standard vs extended
    if ((addr ^ addr_inv) == 0xFF && (cmd ^ cmd_inv) == 0xFF) {
        decodedAddress = addr;             // standard 8-bit address
    } else {
        decodedAddress = (addr << 8) | addr_inv; // extended 16-bit address
    }
    decodedCommand = cmd;
    decodedRaw     = raw;

    return true;
}
