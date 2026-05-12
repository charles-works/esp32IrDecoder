# ESP32-C3 NEC Infrared Decoder with SSD1315 OLED

A modular, low‑power NEC infrared remote control decoder for the **ESP32‑C3 SuperMini**. The decoded raw address and command bytes (original + inverse) are shown in hexadecimal on a 0.96‑inch I²C OLED display (SSD1315 driver).

## Features

- **NEC protocol decoding** – Supports both standard 8‑bit address and extended 16‑bit address, with complement verification.
- **Raw byte display** – OLED shows address and command as original + inverse byte pairs (e.g. `A:00 FF` / `C:0C F3`), no `0x` prefix, matching what the NEC frame actually transmits.
- **Timeout‑based frame capture** – Edge durations are accumulated in the ISR buffer and only decoded after a 110 ms silence window, ensuring complete NEC frame capture.
- **Serial debug output** – Decoded bytes and frame edge count are printed to the serial monitor for debugging.
- **Accurate timing** – Interrupt‑driven pulse capture with glitch filtering (sub‑100 µs) ensures reliable decoding of most consumer remotes.
- **Modular design** – Separate classes for IR decoding and display management make the code easy to extend.
- **SSD1315 native support** – Uses the U8g2 library with explicit I²C pin configuration from `config.h`.

## Hardware Requirements

- ESP32‑C3 SuperMini (or compatible board)
- 1838 infrared receiver (38 kHz, NEC compatible)
- 0.96″ I²C OLED module (128×64 pixels, SSD1315 controller)
- Jumper wires, breadboard, 3.3 V power supply

### Wiring

| OLED Module | ESP32‑C3 SuperMini |
|-------------|-------------------|
| SDA         | GPIO 8            |
| SCL         | GPIO 9            |
| VCC         | 3.3 V             |
| GND         | GND               |

| 1838 IR Receiver | ESP32‑C3 SuperMini |
|------------------|-------------------|
| OUT              | GPIO 4            |
| VCC              | 3.3 V             |
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
   cd esp32IrDecoder
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
   Decoded codes are printed to the serial monitor in the same format as the OLED.

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

1. Power up the board. The OLED displays **"Waiting for IR…"**.
2. Point an NEC‑compatible remote at the receiver and press a button.
3. The screen shows the raw original + inverse byte pairs for address and command:
   ```
   A:00 FF
   C:0C F3
   ```
   The serial monitor prints the same:
   ```
   NEC: A:00 FF  C:0C F3
   ```
4. After 1 minute of inactivity, the display returns to waiting mode.
5. Pressing another key instantly refreshes the display and resets the timer.

### How to read the display

| Label | Meaning | Example |
|-------|---------|---------|
| `A:XX XX` | Address original + inverse byte | `A:00 FF` means address 0x00, inverse 0xFF (standard 8‑bit address) |
| `C:XX XX` | Command original + inverse byte | `C:0C F3` means command 0x0C, inverse 0xF3 |

## Project Structure

```
.
├── platformio.ini
├── README.md
├── README_CN.md
└── src/
    ├── config.h
    ├── IRDecoder.h
    ├── IRDecoder.cpp
    ├── DisplayManager.h
    ├── DisplayManager.cpp
    └── main.cpp
```

- **IRDecoder** – Captures pulse widths via pin‑change interrupt into a 128‑entry buffer. After a 110 ms silence window (indicating the NEC frame is complete), it snapshots the buffer and decodes the NEC leader + 32 data bits.
- **DisplayManager** – Initialises the SSD1315 (via U8g2 with explicit I²C pins), handles screen updates and the 1‑minute timeout.
- **main.cpp** – Main loop that links both modules.

## Troubleshooting

| Problem                | Possible solutions                                                                 |
|------------------------|------------------------------------------------------------------------------------|
| No OLED output         | Check I²C wiring. Try changing `OLED_ADDR` to 0x3D. Ensure the display works at 3.3 V. |
| IR not decoding        | Open the serial monitor (`pio device monitor -b 115200`). If you see `frame captured: 0 edges`, check that the 1838 VCC is 3.3 V and the OUT pin is connected to GPIO 4. If edges are captured but decode fails, ambient IR interference (fluorescent lights) may be the cause. |
| Partial or missing bytes | Some remotes use extended 16‑bit addresses. The decoder handles both formats; the raw bytes shown will tell you exactly what was received. |


## Firmware Files Explained

Building the project produces two firmware files:

| File | Path | Description |
|------|------|-------------|
| `firmware.bin` | `.pio/build/esp32-c3-supermini/firmware.bin` | Raw binary firmware image. This is the file you flash onto the ESP32-C3's flash memory — it is the actual program that runs on the chip. |
| `firmware.elf` | `.pio/build/esp32-c3-supermini/firmware.elf` | ELF binary with full symbol table and debug information. Much larger than the `.bin`. Used for **GDB debugging** and **crash backtrace analysis** (`addr2line`). **Not suitable for normal flashing.** |

> **In short:** Use `firmware.bin` for day-to-day flashing. Only reach for `firmware.elf` when you need to debug a crash.

## Flashing with Other Tools

Besides PlatformIO, you can use any of the following tools to flash `firmware.bin` to the ESP32-C3.

### Option 1: esptool.py (Recommended)

[esptool](https://github.com/espressif/esptool) is Espressif's official command-line flash tool (PlatformIO uses it under the hood).

```bash
# Install
pip install esptool

# Flash (0x0 is the default firmware offset for ESP32-C3)
esptool.py --chip esp32c3 --port /dev/ttyUSB0 --baud 460800 \
  write_flash 0x0 .pio/build/esp32-c3-supermini/firmware.bin
```

> **Windows:** The port is usually `COM3`, `COM4`, etc. Check Device Manager.
>
> **macOS:** The port is usually `/dev/cu.usbserial-xxxx` or `/dev/cu.usbmodem-xxxx`.

If flashing fails, try adding `--before default_reset --after hard_reset`, or hold the **BOOT** button on the board while running the command.

### Option 2: Espressif Flash Download Tool (Windows GUI)

1. Download from the [official tools page](https://www.espressif.com/en/support/download/other-tools) and extract.
2. Open the tool, select **ESP32-C3** chip type.
3. Configure:
   - **SPISpeed:** 80 MHz
   - **SPIMode:** QIO
   - **Flash Size:** 4 MB
4. Load `firmware.bin` and set the address to `0x0`.
5. Select the correct COM port, then click **START**.

### Option 3: ESP Web Flasher (In-Browser)

No installation required. Works in Chrome / Edge only.

1. Visit [ESP Web Flasher](https://espressif.github.io/esptool-js/)
2. Connect the board via USB, click **Connect**, and select the serial port.
3. Load `firmware.bin`, set address to `0x0`, click **Program**.

> **Note:** Requires a browser with Web Serial API support (Chrome 89+, Edge 89+).

### Option 4: espflash (Rust-based)

If you prefer a Rust toolchain, [espflash](https://github.com/esp-rs/espflash) is a solid alternative:

```bash
# Install
cargo install espflash

# Flash
espflash --chip esp32c3 /dev/ttyUSB0 .pio/build/esp32-c3-supermini/firmware.bin
```

### Entering Download Mode

If the flash tool reports "Failed to connect", you may need to manually enter download mode:

1. **Press and hold** the **BOOT (GPIO9)** button on the board.
2. **Briefly press** the **RST (RESET)** button.
3. **Release** the BOOT button.
4. The chip is now in download mode — re-run the flash command.

## License

MIT – feel free to use, modify, and share.
