# ESP32-C3 NEC 红外解码器（SSD1315 OLED 显示）

基于 **ESP32‑C3 SuperMini** 的模块化 NEC 红外遥控解码项目。解码后的 16 位地址码和 8 位操作码以十六进制形式显示在 0.96 寸 I²C OLED 屏幕上（SSD1315 驱动芯片）。

## 功能特点

- **NEC 协议解码** – 支持标准 8 位地址及扩展 16 位地址，带反码校验。
- **清晰的 OLED 显示** – 未收到信号时显示“Waiting for IR…”，收到信号后显示 `Addr:0xXXXX` 和 `Cmd: 0xYY`，保持 1 分钟（或直到收到新码）。
- **精确时序** – 基于引脚中断的脉冲捕捉，可稳定解码大多数家电遥控器。
- **模块化设计** – 红外解码与显示管理各自独立，便于后续修改和扩展。
- **SSD1315 专用驱动** – 使用 U8g2 库，自动完成电荷泵、COM 配置等初始化。

## 硬件需求

- ESP32‑C3 SuperMini（或其他兼容板）
- 1838 红外接收头（38 kHz，NEC 协议兼容）
- 0.96 寸 I²C OLED 显示屏（128×64，SSD1315 控制器）
- 面包板、杜邦线、3.3 V 电源

### 接线

| OLED 模块 | ESP32‑C3 SuperMini |
|-----------|-------------------|
| SDA       | GPIO 8            |
| SCL       | GPIO 9            |
| VCC       | 3.3 V             |
| GND       | GND               |

| 1838 红外接收头 | ESP32‑C3 SuperMini |
|----------------|-------------------|
| OUT            | GPIO 4            |
| VCC            | 3.3 V             |
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
   cd esp32-nec-ir-decoder
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

1. 上电后，OLED 显示 **“Waiting for IR…”**。
2. 将 NEC 协议遥控器对准接收头，按下任意键。
3. 屏幕立即显示地址码和命令码，例如：
   ```
   Addr:0x00FF
   Cmd: 0x0C
   ```
4. 若 1 分钟内无新按键，屏幕自动回到等待界面。
5. 按下其他按键会刷新显示并重新计时。

## 项目结构

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

- **IRDecoder** – 通过引脚电平变化中断记录脉冲宽度，识别 NEC 起始位/数据位，提取地址码和命令码。
- **DisplayManager** – 使用 U8g2 初始化 SSD1315，管理画面切换和 1 分钟倒计时。
- **main.cpp** – 主循环，连接两个模块。

## 常见问题

| 问题                   | 可能的原因及解决方法                                                         |
|------------------------|------------------------------------------------------------------------------|
| OLED 无显示            | 检查 I²C 接线。尝试将 `OLED_ADDR` 改为 0x3D。确认显示屏支持 3.3 V 供电。      |
| 无法解码红外信号        | 确认 1838 供电为 3.3 V，信号线连接正确。大多数接收头模块内部已含上拉电阻。     |
| 接收到的码值不稳定或错误 | 减少环境红外干扰（如日光灯）。可在正常室内光线条件下测试。                      |

## 许可证

MIT 许可证 – 您可以自由使用、修改和分发本项目。
