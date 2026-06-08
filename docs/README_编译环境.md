# STM32 控制板固件 — VSCode + CMake 编译环境说明

> 纯 VSCode、无 Keil 的编译/烧录环境。芯片 **STM32F030F4P6**（Cortex-M0, 16KB Flash / 4KB RAM）。
> 屏(DWIN)是独立模块，不在本工程内；本工程只管 **STM32 控制固件**。

---

## 一、要装什么（一次性）

### 编译工具（二选一）
- **路线A（推荐，省事）**：装 **STM32CubeCLT**（ST 官方免费命令行工具包），一次性带齐 `arm-none-eabi-gcc` + `cmake` + `ninja` + `gdb`。装完用它的 “**STM32CubeCLT** 命令行” 或确保这些工具进了系统 PATH。
- **路线B（手动）**：分别装
  - Arm GNU Toolchain（`arm-none-eabi-gcc`）：ARM 官网下载
  - CMake：cmake.org
  - Ninja：github.com/ninja-build

### 烧录工具
- **SEGGER J-Link 软件包**（你已有 J-Link 硬件，但要装它的 PC 软件，VSCode 调试时会调用 `JLinkGDBServer`）。安装包就在仓库 `5_工具与文档/JLink_Windows_x86_64_V750.exe`。

### VSCode 扩展（打开本文件夹时会自动提示安装）
- **CMake Tools**（ms-vscode.cmake-tools）
- **C/C++**（ms-vscode.cpptools）
- **Cortex-Debug**（marus25.cortex-debug）—— 负责 J-Link 烧录+调试

> 装完验证：终端里 `arm-none-eabi-gcc --version`、`cmake --version`、`ninja --version` 都能打印版本，就 OK。若提示找不到 `arm-none-eabi-gcc`，去改 `cmake/gcc-arm-none-eabi.cmake` 里的 `TOOLCHAIN_PREFIX` 为绝对路径。

---

## 二、怎么编译

### 方式1：图形界面（推荐新手）
1. VSCode **打开文件夹** `1_STM32固件_控制板`（注意是打开这个文件夹本身）。
2. 右下角弹窗提示装推荐扩展 → 全装。
3. CMake Tools 会自动读 `CMakePresets.json`，底部状态栏选 **debug** 预设。
4. 点状态栏的 **Build**（或按 F7）。
5. 成功后在 `build/debug/` 下生成 **`HGJ.elf` / `HGJ.hex` / `HGJ.bin`**，并在输出末尾打印 Flash/RAM 占用。

### 方式2：命令行
```powershell
cmake --preset debug
cmake --build --preset debug
```

---

## 三、怎么烧录

- **VSCode 内**：J-Link 接好 SWD（原理图 H2 口：SWDIO/SWCLK/3.3V/GND），接通电源，按 **F5** → 自动烧录并进入调试。
- **或手动**：用 SEGGER **J-Flash Lite**，选器件 `STM32F030F4`、接口 SWD，加载 `build/debug/HGJ.hex` 烧录。
- 若 F5 报器件名不符，改 `.vscode/launch.json` 里的 `"device"` 为 J-Link 设备库里的准确名称。

---

## 四、⚠️ 编译与验证（重要）

固件为 **V2.0**（双区 PI + 安全联锁），已编译/烧录/上机运行。

1. 编译：Build 出现 **0 error** 即可（warning 多数可忽略），产物在 `build/<debug|release>/HGJ.hex`。
2. ⚠️ **发布前务必台架验证安全**：拔 NTC 探头→停加热、拔 FPC 排线→停加热、模拟死机→看门狗复位。PI 增益与安全阈值是占位值（`USER_APPLICATION/control.c` 顶部，标 `[TUNE]`/`[BENCH]`），需整定。

---

## 五、常见问题
| 现象 | 原因 / 解决 |
|---|---|
| 找不到 `arm-none-eabi-gcc` | 没进 PATH。改 `cmake/gcc-arm-none-eabi.cmake` 的 `TOOLCHAIN_PREFIX` 为绝对路径 |
| 找不到 `ninja` | 没装 ninja，或把 `CMakePresets.json` 的 `"generator"` 改成 `"MinGW Makefiles"` 等 |
| 中文注释乱码 | 源码是 GBK；`.vscode/settings.json` 已设 `files.encoding=gbk`，重开文件即可 |
| 链接报 `region FLASH overflowed` | 代码超 16KB。检查是否误开了 `-O0`/调试信息进了 Flash（Release 用 `-Os`）|
| F5 连不上 J-Link | 检查 SWD 接线、供电；确认装了 J-Link 软件包；`launch.json` 的 device 名 |

---

## 六、目录里这些新文件是干嘛的
| 文件 | 作用 |
|---|---|
| `CMakeLists.txt` | 工程定义：源文件、头路径、宏、编译/链接选项 |
| `CMakePresets.json` | 一键配置（debug/release 预设） |
| `cmake/gcc-arm-none-eabi.cmake` | GCC 交叉编译工具链定义 |
| `STM32F030F4Px_FLASH.ld` | 链接脚本（16KB Flash / 4KB RAM 内存布局）|
| `.vscode/` | 扩展推荐、CMake 设置、J-Link 调试配置 |
| `.gitignore` | 忽略 build 产物 |

> 这套是手写的（等价于 CubeMX 选 CMake 生成的结果），所以你**不装 CubeMX 也能编译**。以后若要改引脚/外设配置，再用 CubeMX 打开根目录的 `HGJ.ioc` 即可。
