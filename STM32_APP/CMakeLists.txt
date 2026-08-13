# ============================================================
#  烘干器 STM32 控制板固件 — CMake 工程
#  芯片: STM32F030F4P6 (Cortex-M0, 16KB Flash / 4KB RAM)
#  工具链: arm-none-eabi-gcc (见 cmake/gcc-arm-none-eabi.cmake)
#  说明: 纯 STM32 控制固件; 屏(DWIN)为独立模块, 不在本工程内
# ============================================================
cmake_minimum_required(VERSION 3.20)

# 语言: C + 汇编(启动文件)
project(HGJ C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 默认 Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(TARGET_NAME ${PROJECT_NAME})

# ---------------- 芯片/CPU 编译开关 ----------------
set(CPU_FLAGS -mcpu=cortex-m0 -mthumb -mfloat-abi=soft)

# ---------------- 宏定义 ----------------
add_compile_definitions(
  USE_HAL_DRIVER
  STM32F030x6
)

# ---------------- 头文件路径(取自原 Keil 工程) ----------------
include_directories(
  ${CMAKE_SOURCE_DIR}/Core/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F0xx_HAL_Driver/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F0xx/Include
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
  ${CMAKE_SOURCE_DIR}/RTT
  ${CMAKE_SOURCE_DIR}/USER_DRIVE
  ${CMAKE_SOURCE_DIR}/USER_APPLICATION
)

# ---------------- 源文件 ----------------
# CMSIS/HAL 启动代码(GCC 版! 不是 Keil 的 arm 版)
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f030x6.s)

# 应用层 + HAL 源
file(GLOB CORE_SRC   ${CMAKE_SOURCE_DIR}/Core/Src/*.c)
file(GLOB HAL_SRC    ${CMAKE_SOURCE_DIR}/Drivers/STM32F0xx_HAL_Driver/Src/*.c)
file(GLOB DRIVE_SRC  ${CMAKE_SOURCE_DIR}/USER_DRIVE/*.c)
file(GLOB APP_SRC    ${CMAKE_SOURCE_DIR}/USER_APPLICATION/*.c)

# 排除 HAL 模板文件(*_template.c, 不参与编译)
list(FILTER HAL_SRC EXCLUDE REGEX ".*_template\\.c$")

# RTT: 只要 C 文件; 那个 *_ASM_ARMv7M.S 是给 Cortex-M3/4 的, M0 不能用 -> 不加
set(RTT_SRC
  ${CMAKE_SOURCE_DIR}/RTT/SEGGER_RTT.c
  ${CMAKE_SOURCE_DIR}/RTT/SEGGER_RTT_printf.c
)

add_executable(${TARGET_NAME}
  ${CORE_SRC}
  ${HAL_SRC}
  ${DRIVE_SRC}
  ${APP_SRC}
  ${RTT_SRC}
  ${STARTUP_FILE}
)

# ---------------- 编译选项 ----------------
target_compile_options(${TARGET_NAME} PRIVATE
  ${CPU_FLAGS}
  -ffunction-sections -fdata-sections   # 配合 --gc-sections 去掉未用代码
  -Wall
  $<$<CONFIG:Debug>:-Os -g3 -gdwarf-2>   # -Os: 16KB Flash 偏紧, 用大小优化(仍带调试符号)
  $<$<CONFIG:Release>:-Os>
)

# ---------------- 链接选项 ----------------
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F030F4Px_FLASH.ld)

target_link_options(${TARGET_NAME} PRIVATE
  ${CPU_FLAGS}
  -T${LINKER_SCRIPT}
  --specs=nano.specs          # 精简版 newlib(nano)
  --specs=nosys.specs         # 无操作系统的 syscall 桩
  -Wl,--gc-sections
  -Wl,-Map=${TARGET_NAME}.map,--cref
  -Wl,--print-memory-usage    # 编译末尾打印 Flash/RAM 占用
  -lc -lm
)

# 链接脚本变化时重新链接
set_target_properties(${TARGET_NAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# ---------------- 生成 .hex / .bin, 并打印体积 ----------------
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE_NAME:${TARGET_NAME}> ${TARGET_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE_NAME:${TARGET_NAME}> ${TARGET_NAME}.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE_NAME:${TARGET_NAME}>
  WORKING_DIRECTORY $<TARGET_FILE_DIR:${TARGET_NAME}>   # 在 build 目录内用相对文件名, 避开中文路径
  COMMENT "生成 ${TARGET_NAME}.hex / ${TARGET_NAME}.bin 并统计大小"
)
