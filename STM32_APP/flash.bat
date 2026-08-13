@echo off
setlocal enableextensions
title Drier STM32 Firmware Flasher

REM ============================================================
REM  Drier (HGJ) STM32F030F4 production flasher via SEGGER J-Link
REM  Use: connect J-Link (SWD) to the board, power it, run me.
REM       Keep HGJ.hex next to this .bat (or it falls back to
REM       build\release\HGJ.hex). One unit per keypress.
REM  Tip: for a clean production PC, copy flash.bat + HGJ.hex
REM       into a simple folder like C:\DrierFlash and run there.
REM ============================================================

REM ----- settings (edit if your setup differs) -----
set "DEVICE=STM32F030F4"
set "IFACE=SWD"
set "SPEED=4000"

REM ----- locate the firmware HGJ.hex -----
set "HEX=%~dp0HGJ.hex"
if not exist "%HEX%" set "HEX=%~dp0build\release\HGJ.hex"
if not exist "%HEX%" (
  echo [ERROR] HGJ.hex not found.
  echo         Put HGJ.hex next to this .bat, or build the firmware first.
  echo.
  pause
  exit /b 1
)

REM ----- locate JLink.exe -----
set "JLINK=C:\Program Files\SEGGER\JLink\JLink.exe"
if not exist "%JLINK%" set "JLINK=C:\Program Files (x86)\SEGGER\JLink\JLink.exe"
if not exist "%JLINK%" (
  echo [ERROR] JLink.exe not found.
  echo         Install SEGGER J-Link software, or edit JLINK at the top of this file.
  echo.
  pause
  exit /b 1
)

REM ----- copy hex to an ASCII temp path (J-Link loadfile dislikes spaces/Chinese) -----
set "HEXTMP=%TEMP%\drier_HGJ.hex"
copy /y "%HEX%" "%HEXTMP%" >nul 2>nul
if not exist "%HEXTMP%" set "HEXTMP=%HEX%"

REM ----- build the J-Link command script (erase+program+verify, then run) -----
set "SCRIPT=%TEMP%\drier_flash.jlink"
(
  echo connect
  echo r
  echo h
  echo loadfile "%HEXTMP%"
  echo r
  echo g
  echo qc
) > "%SCRIPT%"

echo ==================================================
echo   Drier STM32 Firmware Flasher
echo   Device  : %DEVICE%   ( %IFACE% @ %SPEED% kHz )
echo   Firmware: %HEX%
echo ==================================================

:loop
echo.
echo  Connect J-Link (SWD) to the board and power it ON,
echo  then press any key to FLASH this unit...
pause >nul
echo  Flashing, please wait...
echo.
"%JLINK%" -device %DEVICE% -if %IFACE% -speed %SPEED% -ExitOnError 1 -CommanderScript "%SCRIPT%"
echo.
if errorlevel 1 (
  echo  ===============  [ FAIL ]  ===============
  echo  Check SWD wiring / power / target, then retry.
) else (
  echo  ===============  [  OK  ]  ===============
  echo  Programmed and verified OK.
)
echo.
echo  Swap in the NEXT unit and press any key, or close this window to finish.
pause >nul
goto loop
