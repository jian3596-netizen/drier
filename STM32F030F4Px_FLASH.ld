/*
******************************************************************************
**  Linker script for STM32F030F4Px
**  Device: STM32F030F4P6  ->  FLASH = 16 KB, RAM = 4 KB
**  FLASH : 0x08000000 - 0x08003FFF (16K)
**  RAM   : 0x20000000 - 0x20000FFF (4K)
**  Toolchain: GNU Arm Embedded (arm-none-eabi-gcc), Cortex-M0
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack (end of RAM) */
_estack = ORIGIN(RAM) + LENGTH(RAM);   /* = 0x20001000 */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0x200;   /* required amount of heap  */
_Min_Stack_Size = 0x400;   /* required amount of stack */

/* Memory definition */
MEMORY
{
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 4K
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
}

/* Sections */
SECTIONS
{
  /* The startup code into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
  } >FLASH

  /* Constant data into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* Firmware version string -- KEEP so --gc-sections never strips it */
  .fw_version :
  {
    . = ALIGN(4);
    KEEP(*(.fw_version))
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections into RAM, load from FLASH */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section (BSS) */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
