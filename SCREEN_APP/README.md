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

该源码目前仍使用旧 C51 工具链习惯，尚未完成 SDCC 兼容层和 VS Code 构建脚本。后续迁移时应保留 DGUS API、内存模型和中断声明的行为，并用现有 `DWIN_SET/T5L51.bin` 做尺寸和设备回归对照。
