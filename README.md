# 烘干器 — STM32 控制板固件

STM32F030F4P6（Cortex-M0）控制板固件。本板是烘干器的**控制执行端**，与迪文 DWIN 屏（另一颗处理器）通过串口协同：屏负责测温/设定/界面，本板负责**双区 PI 控温 + 安全联锁**，驱动 2 路加热 + 2 路风扇。

**版本：V2.0** ｜ 工具链：CMake + arm-none-eabi-gcc（纯 VSCode，无 Keil）

---

## 快速开始

> ⚠️ **构建路径不能有空格/中文**（Cortex-Debug/objcopy 会因此出错）。本仓库通过英文别名 `E:\Drier_FW` 访问构建。

```powershell
# 配置 + 编译（在仓库根目录）
cmake --preset debug
cmake --build --preset debug      # 产物: build/debug/HGJ.hex
# 正式发布版
cmake --preset release
cmake --build --preset release    # 产物: build/release/HGJ.hex
```

- **烧录/调试**：VSCode 装 Cortex-Debug，按 **F5**（J-Link 接 SWD，原理图 H2 口）。
- **纯烧录**：J-Flash Lite，器件 `STM32F030F4`、SWD，加载 `build/release/HGJ.hex`。
- 详细步骤见 **[docs/README_编译环境.md](docs/README_编译环境.md)**。

---

## 代码结构

| 路径 | 作用 |
|---|---|
| `USER_APPLICATION/control.c` | **控温核心**：双区 PI + 安全联锁 + 看门狗 |
| `USER_DRIVE/` | 驱动：ADC 采集、DWIN 串口、串口缓冲 |
| `Core/` | CubeMX 生成的外设初始化 + `main.c` + 中断 |
| `Drivers/` | ST HAL 库 + CMSIS |
| `CMakeLists.txt` / `CMakePresets.json` / `cmake/` / `*.ld` | CMake 构建系统 |
| `docs/` | 全部文档（见下） |

详细目录说明见 **[docs/目录说明.md](docs/目录说明.md)**。

---

## 文档（`docs/`）

| 文档 | 内容 |
|---|---|
| [系统总览.md](docs/系统总览.md) | 整个双处理器系统的架构、协议、分工 |
| [README_编译环境.md](docs/README_编译环境.md) | 装什么、怎么编、怎么烧 |
| [目录说明.md](docs/目录说明.md) | 本固件的目录/文件逐项说明 |
| [改进意见与安全评估.md](docs/改进意见与安全评估.md) | 加热能力评估、安全风险与已实现的固件处置 |

---

## ⚠️ 安全提示（加热产品）

- PI 增益（`PI_KP`/`PI_KI_INC`）与安全阈值（`RES_MIN/MAX_OHM`、`TEMP_HARD_MAX`）标着 `[TUNE]`/`[BENCH]`，**需台架整定/核实**。
- 发布前务必通过安全测试：拔 NTC 探头→停加热、拔排线→停加热、模拟死机→看门狗复位。
- 强烈建议硬件加**独立温度保护**（温控开关/温度保险丝）——软件替代不了。
- 详见 [docs/改进意见与安全评估.md](docs/改进意见与安全评估.md)。
