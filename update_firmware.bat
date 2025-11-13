@echo off
REM Script to update firmware binaries in the firmware/ folder

echo === Centauri Carbon Monitor - Firmware Update Script ===
echo.

set BUILD_DIR=.pio\build\nologo_esp32c3_super_mini
set FIRMWARE_DIR=firmware

REM Check if build exists
if not exist "%BUILD_DIR%\firmware.bin" (
    echo Error: Build not found. Run 'platformio run' first.
    exit /b 1
)

REM Create firmware directory if it doesn't exist
if not exist "%FIRMWARE_DIR%" mkdir "%FIRMWARE_DIR%"

echo Copying firmware files...

REM Copy binaries
copy /Y "%BUILD_DIR%\bootloader.bin" "%FIRMWARE_DIR%\" >nul
copy /Y "%BUILD_DIR%\partitions.bin" "%FIRMWARE_DIR%\" >nul
copy /Y "%BUILD_DIR%\firmware.bin" "%FIRMWARE_DIR%\" >nul

echo Done! Firmware files updated:
echo.
echo    bootloader.bin  -^> %FIRMWARE_DIR%\
echo    partitions.bin  -^> %FIRMWARE_DIR%\
echo    firmware.bin    -^> %FIRMWARE_DIR%\
echo.

echo File sizes:
dir "%FIRMWARE_DIR%\*.bin" | find ".bin"
echo.

echo Done! Firmware files are ready for git commit.
echo.
echo Next steps:
echo   1. git add firmware/
echo   2. git commit -m "Update firmware binaries"
echo   3. git push
echo.

pause
