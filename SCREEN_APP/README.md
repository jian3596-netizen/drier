# DWIN 屏幕应用

屏幕应用分为页面资源和屏内程序两部分。温度之所以能显示，是 `T5L51` 程序接收 STM32 的原始电阻、完成温度换算后写入 DGUS VP；DGUS 页面本身不会读取 STM32 串口协议。

## `DGUS/`

- 工程入口：`DWprj.hmi`
- 页面图片与控件配置：当前目录内的图片、`DisplayConfig.xls`、`TouchConfig.xls` 等
- 可下载到屏幕的文件：`DWIN_SET/`

DGUS Tool 用于编辑页面和生成下载资源。页面控件配置中的 VP 地址必须由 T5L51 程序或 DGUS 运行时实际写入，单独配置地址不会自动产生数据。

## `T5L51/`

- `USER/main.c`：主程序与界面状态逻辑
- `HANDWARE/TASK/task.c`：任务和业务状态处理
- `HANDWARE/UART2/uart2.c`：与 STM32 的自定义串口协议
- `USER/sys.c`：DGUS VP 等底层调用
- `USER/template.uvproj`：原始 Keil C51 工程，仅作为现有工程入口和编译参数参考

该源码已经支持 **SDCC 4.6+**，无需安装 Keil。仓库根目录的 VS Code Build Task 会调用 `T5L51/build.ps1`，输出 `T5L51/dist/T5L51.bin`。

首次安装工具链：

```powershell
powershell -ExecutionPolicy Bypass -File SCREEN_APP/T5L51/setup_sdcc.ps1
```

命令行构建：

```powershell
powershell -ExecutionPolicy Bypass -File SCREEN_APP/T5L51/build.ps1
```

VS Code 中按 `Ctrl+Shift+B`，选择 **T5L51: Build (SDCC)**。若确认要更新烧屏包，选择 **T5L51: Build + update DWIN_SET**；普通 Build 不会覆盖当前已验证的 `DGUS/DWIN_SET/T5L51.bin`。

构建脚本会检查 T5L 特有的 `FF FF DWINT5` 签名、应用入口和 UART2/Timer2 中断向量，并限制映像不超过 32 KiB。
