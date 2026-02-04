@echo off
REM NetRaze32 Build Verification Script
echo ========================================
echo NetRaze32 Build Verification
echo ========================================
echo.

if not defined IDF_PATH (
    echo ERROR: ESP-IDF environment not set up
    echo Please run: %%USERPROFILE%%\esp\esp-idf\export.bat
    pause
    exit /b 1
)

echo [1/3] Checking ESP-IDF environment...
echo IDF_PATH: %IDF_PATH%
echo.

echo [2/3] Building project...
idf.py build
if errorlevel 1 (
    echo.
    echo BUILD FAILED!
    pause
    exit /b 1
)
echo.

echo [3/3] Checking binary sizes...
python -c "import os; size = os.path.getsize('build/esp32_div.bin'); print(f'Firmware size: {size:,} bytes ({size/1024/1024:.2f} MB)')"
echo.

echo ========================================
echo BUILD VERIFICATION COMPLETE!
echo ========================================
echo.
echo Next: idf.py -p COMx flash
echo.
pause
