@echo off
echo ========================================
echo NetRaze32 Firmware Extraction Tool
echo ========================================
echo.

set /p COMPORT="Enter COM port (e.g., COM3): "

set ESPTOOL=C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\esptool.exe

echo.
echo Creating backup directory...
mkdir firmware_backup 2>nul

echo.
echo Reading chip info...
%ESPTOOL% -p %COMPORT% chip_id
%ESPTOOL% -p %COMPORT% flash_id

echo.
echo Extracting full flash (4MB)...
%ESPTOOL% -p %COMPORT% read_flash 0x0 0x400000 firmware_backup\full_flash.bin

echo.
echo Extracting bootloader...
%ESPTOOL% -p %COMPORT% read_flash 0x1000 0x7000 firmware_backup\bootloader.bin

echo.
echo Extracting partition table...
%ESPTOOL% -p %COMPORT% read_flash 0x8000 0x1000 firmware_backup\partition_table.bin

echo.
echo Extracting app partition...
%ESPTOOL% -p %COMPORT% read_flash 0x10000 0x300000 firmware_backup\app.bin

echo.
echo Extracting NVS partition...
%ESPTOOL% -p %COMPORT% read_flash 0x9000 0x6000 firmware_backup\nvs.bin

echo.
echo ========================================
echo Extraction complete!
echo Files saved to: firmware_backup\
echo ========================================
pause
