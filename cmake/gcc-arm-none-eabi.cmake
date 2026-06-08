# ============================================================
#  CMake 工具链文件：GNU Arm Embedded (arm-none-eabi-gcc)
#  目标芯片：STM32F030F4P6 (Cortex-M0)
#  用法：configure 时通过 --toolchain 指定本文件（CMakePresets 已配好）
# ============================================================

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  arm)

# 交叉编译时不让 CMake 去链接一个完整可执行做编译器测试（会因缺 _start 失败）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 工具链前缀。若 arm-none-eabi-* 已在 PATH 中，直接用名字即可；
# 否则把下面一行改成绝对路径，例如：
#   set(TOOLCHAIN_PREFIX "C:/Program Files/Arm GNU Toolchain/bin/arm-none-eabi-")
set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_C_COMPILER    ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY       ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "")
set(CMAKE_SIZE          ${TOOLCHAIN_PREFIX}size    CACHE INTERNAL "")

set(CMAKE_EXECUTABLE_SUFFIX_C   .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)

# 在 Windows 上 Ninja/Make 找 .exe
if(WIN32)
  set(CMAKE_C_COMPILER    ${TOOLCHAIN_PREFIX}gcc.exe)
  set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PREFIX}gcc.exe)
  set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PREFIX}g++.exe)
endif()
