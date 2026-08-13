# 烘干器固件项目

本仓库包含两个通过 UART 协作的独立应用：STM32 控制板固件和 DWIN 屏幕应用。建议始终用 VS Code 打开本目录，STM32 的 CMake 与 J-Link 配置已经指向子工程。

## 目录

| 目录 | 内容 | 主要工具 |
|---|---|---|
| `STM32_APP/` | STM32F030F4 控温、安全联锁、加热和风扇驱动 | VS Code + CMake + Arm GCC + J-Link |
| `SCREEN_APP/DGUS/` | DWIN DGUS 页面、控件配置和 `DWIN_SET` 下载包 | DGUS Tool |
| `SCREEN_APP/T5L51/` | T5L 屏内 8051 程序源码 | 当前为旧 C51 工程；后续可迁移到 SDCC |
| `.temp/` | 本地分析、抓取和临时文件，不纳入版本管理 | — |

## STM32 快速操作

在根目录打开 VS Code 后：

- CMake Tools 会把 `STM32_APP` 作为源目录；选择 `debug` 或 `release` 预设即可 Build。
- 按 F5 使用 J-Link 烧录并调试 Debug 固件。
- 量产/仅烧录时运行 `STM32_APP/flash.bat`；它默认烧录 `STM32_APP/build/release/HGJ.hex`。

命令行编译：

```powershell
cd STM32_APP
cmake --preset debug
cmake --build --preset debug
cmake --preset release
cmake --build --preset release
```

更多说明见 [STM32_APP/README.md](STM32_APP/README.md) 和 [SCREEN_APP/README.md](SCREEN_APP/README.md)。两端串口数据的边界见 [PROTOCOL.md](PROTOCOL.md)。

## 屏幕应用

- 用 DGUS Tool 打开 `SCREEN_APP/DGUS/DWprj.hmi`。
- 屏幕下载目录为 `SCREEN_APP/DGUS/DWIN_SET/`。
- `SCREEN_APP/T5L51/` 已保存可读的 8051 源码；它不是单靠 DGUS 页面配置生成的程序。

屏幕程序目前还未完成 SDCC 构建迁移，因此“无需 Keil 的 T5L51 编译”是下一阶段工作，不应把现有 `.uvproj` 误认为 VS Code 能直接编译。
