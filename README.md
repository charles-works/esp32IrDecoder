# ESP32-C3 NEC Infrared Decoder with SSD1315 OLED

A modular, low‑power NEC infrared remote control decoder for the **ESP32‑C3 SuperMini**. The decoded 16‑bit address and 8‑bit command are shown in hexadecimal on a 0.96‑inch I²C OLED display (SSD1315 driver).

## Features

- **NEC protocol decoding** – Supports both standard 8‑bit address and extended 16‑bit address, with complement verification.
- **Clean OLED output** – The screen shows “Waiting for IR…” until a key is pressed, then displays `Addr: 0xXXXX` and `Cmd: 0xYY` for 1 minute (or until a new code arrives).
- **Accurate timing** – Interrupt‑driven pulse capture ensures reliable decoding of most consumer remotes.
- **Modular design** – Separate classes for IR decoding and display management make the code easy to extend.
- **SSD1315 native support** – Uses the U8g2 library with proper charge‑pump and COM hardware initialisation.

## Hardware Requirements

- ESP32‑C3 SuperMini (or compatible board)
- 1838 infrared receiver (38 kHz, NEC compatible)
- 0.96″ I²C OLED module (128×64 pixels, SSD1315 controller)
- Jumper wires, breadboard, 3.3 V power supply

### Wiring

| OLED Module | ESP32‑C3 SuperMini |
|-------------|-------------------|
| SDA         | GPIO 8            |
| SCL         | GPIO 9            |
| VCC         | 3.3 V             |
| GND         | GND               |

| 1838 IR Receiver | ESP32‑C3 SuperMini |
|------------------|-------------------|
| OUT              | GPIO 4            |
| VCC              | 3.3 V             |
| GND              | GND               |

> **Note:** The OLED I²C address is usually **0x3C**. If your display uses **0x3D** (SA0 pin pulled HIGH), change `OLED_ADDR` in `src/config.h`.

## Software Dependencies

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- Libraries (automatically managed by PlatformIO):
  - **U8g2** by olikraus
  - **Arduino core for ESP32**

## Installation & Build

1. **Clone the repository:**
   ```bash
   git clone <your-repo-url>
   cd esp32-nec-ir-decoder
   ```

2. **Open in VS Code** with the PlatformIO extension installed.

3. **Build & upload:**
   - Connect your ESP32‑C3 board via USB.
   - Click the PlatformIO: Upload button, or run:
     ```bash
     pio run -t upload
     ```

4. **Serial monitor** (optional):
   ```bash
   pio device monitor -b 115200
   ```
   Decoded codes are also printed to the serial monitor.

## Configuration

All settings are in `src/config.h`:

| Parameter           | Default       | Description                          |
|---------------------|---------------|--------------------------------------|
| `IR_RECEIVER_PIN`   | 4             | GPIO pin for 1838 OUT                |
| `OLED_SDA`          | 8             | I²C data pin                         |
| `OLED_SCL`          | 9             | I²C clock pin                        |
| `OLED_ADDR`         | 0x3C          | 7‑bit I²C address (0x3C or 0x3D)    |
| `DISPLAY_HOLD_TIME` | 60000 (1 min) | Code display duration (milliseconds) |

## Usage

1. Power up the board. The OLED displays **“Waiting for IR…”**.
2. Point an NEC‑compatible remote at the receiver and press a button.
3. The screen immediately shows the address and command, e.g.:
   ```
   Addr:0x00FF
   Cmd: 0x0C
   ```
4. After 1 minute of inactivity, the display returns to waiting mode.
5. Pressing another key instantly refreshes the code and resets the timer.

## Project Structure

```
.
├── platformio.ini
├── README.md
└── src/
    ├── config.h
    ├── IRDecoder.h
    ├── IRDecoder.cpp
    ├── DisplayManager.h
    ├── DisplayManager.cpp
    └── main.cpp
```

- **IRDecoder** – Captures pulse widths via pin‑change interrupt, recognises NEC start/bit patterns, and extracts address & command.
- **DisplayManager** – Initialises the SSD1315 (via U8g2), handles screen updates and the 1‑minute timeout.
- **main.cpp** – Main loop that links both modules.

## Troubleshooting

| Problem                | Possible solutions                                                                 |
|------------------------|------------------------------------------------------------------------------------|
| No OLED output         | Check I²C wiring. Try changing `OLED_ADDR` to 0x3D. Ensure the display works at 3.3 V. |
| IR not decoding        | Verify 1838 power (3.3 V) and signal connection. Most modules include a pull‑up.  |
| Wrong or unstable codes| Reduce ambient IR interference (e.g., fluorescent lights). Test in a normally lit room. |

## License

MIT – feel free to use, modify, and share.
