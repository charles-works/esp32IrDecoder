# ESP32-C3 NEC 红外解码器（SSD1315 OLED 显示）

基于 **ESP32‑C3 SuperMini** 的模块化 NEC 红外遥控解码项目。解码后的原始地址码和命令码（原码+反码）以十六进制形式显示在 0.96 寸 I²C OLED 屏幕上（SSD1315 驱动芯片）。

## 功能特点

- **NEC 协议解码** – 支持标准 8 位地址及扩展 16 位地址，带反码校验。
- **原码+反码显示** – OLED 显示地址码和命令码的原码与反码字节对（如 `A:00 FF` / `C:0C F3`），无 `0x` 前缀，忠实反映 NEC 帧实际传输内容。
- **超时帧捕获** – ISR 将边沿持续时间累积到缓冲区，仅在 110 ms 静默窗口后才尝试解码，确保完整 NEC 帧被捕获。
- **串口调试输出** – 解码字节和帧边沿数实时输出到串口监视器，方便调试。
- **精确时序** – 基于引脚中断的脉冲捕捉，带毛刺过滤（<100 µs），可稳定解码大多数家电遥控器。
- **模块化设计** – 红外解码与显示管理各自独立，便于后续修改和扩展。
- **SSD1315 专用驱动** – 使用 U8g2 库，通过 `config.h` 显式配置 I²C 引脚。

## 硬件需求

- ESP32‑C3 SuperMini（或其他兼容板）
- 1838 红外接收头（38 kHz，NEC 协议兼容）
- 0.96 寸 I²C OLED 显示屏（128×64，SSD1315 控制器）
- 面包板、杜邦线、3.3 V 电源

### 接线

| OLED 模块 | ESP32‑C3 SuperMini |
|-----------|-------------------|
| SDA       | GPIO 8            |
| SCL       | GPIO 9            |
| VCC       | 3.3 V             |
| GND       | GND               |

| 1838 红外接收头 | ESP32‑C3 SuperMini |
|----------------|-------------------|
| OUT            | GPIO 4            |
| VCC            | 3.3 V             |
| GND            | GND               |

> **注意：** OLED 的 I²C 地址通常为 **0x3C**。若您的屏幕使用 **0x3D**（SA0 引脚接高电平），请在 `src/config.h` 中修改 `OLED_ADDR` 的值。

## 软件依赖

- [PlatformIO](https://platformio.org/)（推荐）或 Arduino IDE
- 以下库由 PlatformIO 自动管理：
  - **U8g2**（作者 olikraus）
  - **Arduino core for ESP32**

## 安装与编译

1. **克隆仓库：**
   ```bash
   git clone <仓库地址>
   cd esp32IrDecoder
   ```

2. **使用 VS Code 打开项目**（需安装 PlatformIO 扩展）。

3. **编译并上传：**
   - 通过 USB 连接 ESP32‑C3 开发板。
   - 点击 PlatformIO: Upload 按钮，或执行：
     ```bash
     pio run -t upload
     ```

4. **串口监视**（可选）：
   ```bash
   pio device monitor -b 115200
   ```
   每次收到新红外码时，串口也会打印解码结果。

## 配置

所有可调参数均位于 `src/config.h` 文件中：

| 参数                 | 默认值         | 说明                           |
|----------------------|---------------|-------------------------------|
| `IR_RECEIVER_PIN`    | 4             | 1838 信号输出引脚               |
| `OLED_SDA`           | 8             | I²C 数据线                     |
| `OLED_SCL`           | 9             | I²C 时钟线                     |
| `OLED_ADDR`          | 0x3C          | 7 位 I²C 地址（0x3C 或 0x3D） |
| `DISPLAY_HOLD_TIME`  | 60000（1 分钟）| 解码结果保持时间（毫秒）         |

## 使用方法

1. 上电后，OLED 显示 **"Waiting for IR…"**。
2. 将 NEC 协议遥控器对准接收头，按下任意键。
3. 屏幕显示地址码和命令码的原码+反码字节对：
   ```
   A:00 FF
   C:0C F3
   ```
   串口同步输出：
   ```
   NEC: A:00 FF  C:0C F3
   ```
4. 若 1 分钟内无新按键，屏幕自动回到等待界面。
5. 按下其他按键会刷新显示并重新计时。

### 如何阅读显示内容

| 标签 | 含义 | 示例 |
|------|------|------|
| `A:XX XX` | 地址原码 + 反码 | `A:00 FF` 表示地址 0x00，反码 0xFF（标准 8 位地址） |
| `C:XX XX` | 命令原码 + 反码 | `C:0C F3` 表示命令 0x0C，反码 0xF3 |

## 项目结构

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

- **IRDecoder** – 通过引脚电平变化中断将脉冲持续时间存入 128 条目的缓冲区。在 110 ms 静默窗口后（表示 NEC 帧传输完成），快照缓冲区并解码起始位+32 位数据。
- **DisplayManager** – 使用 U8g2 初始化 SSD1315（显式配置 I²C 引脚），管理画面切换和 1 分钟超时。
- **main.cpp** – 主循环，连接两个模块。

## 常见问题

| 问题                   | 可能的原因及解决方法                                                         |
|------------------------|------------------------------------------------------------------------------|
| OLED 无显示            | 检查 I²C 接线。尝试将 `OLED_ADDR` 改为 0x3D。确认显示屏支持 3.3 V 供电。      |
| 无法解码红外信号        | 打开串口监视器（`pio device monitor -b 115200`）。若看到 `frame captured: 0 edges`，检查 1838 VCC 是否 3.3 V、OUT 是否接 GPIO 4。若有边沿但解码失败，可能是环境红外干扰（日光灯）。 |
| 接收到的字节不完整或异常 | 部分遥控器使用扩展 16 位地址。解码器兼容两种格式；显示的原始字节可精确反映实际接收到的内容。 |


## 固件文件说明

项目编译后会生成两个固件文件：

| 文件 | 路径 | 说明 |
|------|------|------|
| `firmware.bin` | `.pio/build/esp32-c3-supermini/firmware.bin` | 主固件二进制文件，用于烧录到 ESP32-C3 的 Flash 中。这是实际运行的程序。 |
| `firmware.elf` | `.pio/build/esp32-c3-supermini/firmware.elf` | ELF 格式的完整固件（含符号表和调试信息），体积较大。用于 GDB 调试、崩溃栈回溯（addr2line）等高级调试场景，**不能直接用于普通烧录**。 |

> **简单来说：** 日常烧录只需 `firmware.bin`；需要调试崩溃问题时才用到 `firmware.elf`。

## 使用其他工具烧录固件

除了 PlatformIO，您也可以使用以下工具将编译好的 `firmware.bin` 烧录到 ESP32-C3 开发板。

### 方式一：esptool.py（推荐）

[esptool](https://github.com/espressif/esptool) 是 Espressif 官方命令行烧录工具，PlatformIO 底层也依赖它。

```bash
# 安装
pip install esptool

# 烧录（地址 0x0 是 ESP32-C3 的默认固件起始地址）
esptool.py --chip esp32c3 --port /dev/ttyUSB0 --baud 460800 \
  write_flash 0x0 .pio/build/esp32-c3-supermini/firmware.bin
```

> **Windows 用户：** 端口名通常为 `COM3`、`COM4` 等，可在设备管理器中查看。
>
> **macOS 用户：** 端口名通常为 `/dev/cu.usbserial-xxxx` 或 `/dev/cu.usbmodem-xxxx`。

如果烧录失败，可尝试添加 `--before default_reset --after hard_reset` 参数，或按住开发板上的 **BOOT** 按键后重新执行烧录命令。

### 方式二：乐鑫 Flash 下载工具（Windows 图形界面）

1. 从 [Flash 下载工具官网](https://www.espressif.com/en/support/download/other-tools) 下载并解压。
2. 打开工具，选择 **ESP32-C3** 芯片类型。
3. 设置参数：
   - **SPISpeed：** 80MHz
   - **SPIMode：** QIO
   - **Flash Size：** 4MB
4. 在固件栏中加载 `firmware.bin`，地址填 `0x0`。
5. 选择正确的 COM 端口，点击 **START** 开始烧录。

### 方式三：ESP Web Flasher（浏览器在线烧录）

适用于不想安装任何工具的场景，仅支持 Chrome / Edge 浏览器。

1. 访问 [ESP Web Flasher](https://espressif.github.io/esptool-js/)
2. 用 USB 连接开发板，点击 **Connect** 并选择串口。
3. 加载 `firmware.bin`，地址设为 `0x0`，点击 **Program**。

> **注意：** 在线烧录工具要求浏览器支持 Web Serial API（Chrome 89+、Edge 89+）。

### 方式四：esptool-rs / espflash（Rust 实现）

如果您偏好 Rust 工具链，可以使用 [espflash](https://github.com/esp-rs/espflash)：

```bash
# 安装
cargo install espflash

# 烧录
espflash --chip esp32c3 /dev/ttyUSB0 .pio/build/esp32-c3-supermini/firmware.bin
```

### 进入下载模式

如果烧录工具提示 "Failed to connect"，需要手动进入下载模式：

1. **按住** 开发板上的 **BOOT（GPIO9）** 按键不放。
2. **短按** 一下 **RST（RESET）** 按键。
3. **松开** BOOT 按键。
4. 此时芯片进入下载模式，重新执行烧录命令即可。

## 许可证

MIT 许可证 – 您可以自由使用、修改和分发本项目。
