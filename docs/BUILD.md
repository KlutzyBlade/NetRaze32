# Building NetRaze32 v1.0

## Prerequisites

### Required Software
1. **ESP-IDF v5.5 or later**
   - Download: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
   - Follow installation guide for your OS

2. **Python 3.8+** (included with ESP-IDF)

3. **Git** (for ESP-IDF installation)

### Hardware
- ESP32-32E development board with 2.8" display
- USB cable (data capable)
- Computer with available USB port

---

## Build Steps

### 1. Set Up ESP-IDF Environment

**Windows:**
```cmd
%USERPROFILE%\esp\esp-idf\export.bat
```

**Linux/macOS:**
```bash
. $HOME/esp/esp-idf/export.sh
```

### 2. Navigate to Project
```bash
cd NetRaze32_Distro_1.0
```

### 3. Configure (Optional)
```bash
idf.py menuconfig
```
Default configuration works for most users.

### 4. Build
```bash
idf.py build
```

Build time: ~2-5 minutes (first build)

### 5. Flash to Device
```bash
idf.py -p COM3 flash
```
Replace `COM3` with your port:
- **Windows:** COM3, COM4, etc.
- **Linux:** /dev/ttyUSB0, /dev/ttyACM0
- **macOS:** /dev/cu.usbserial-*

### 6. Monitor Serial Output (Optional)
```bash
idf.py -p COM3 monitor
```
Press `Ctrl+]` to exit monitor.

---

## One-Line Flash Command
```bash
idf.py -p COM3 flash monitor
```

---

## Troubleshooting

### "Port not found"
- Check USB cable (must support data)
- Install CH340 drivers if needed
- Try different USB port

### "Permission denied" (Linux)
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

### Build errors
```bash
idf.py fullclean
idf.py build
```

### Flash errors
- Hold BOOT button while connecting USB
- Press EN button to reset after flash

---

## Build Output

Successful build creates:
```
build/
├── bootloader/
│   └── bootloader.bin
├── partition_table/
│   └── partition-table.bin
└── netraze32.bin
```

Flash size: ~1.2MB

---

## Advanced Options

### Custom Partition Table
Edit `partitions.csv` before building.

### Debug Build
```bash
idf.py -DCMAKE_BUILD_TYPE=Debug build
```

### Optimization Level
Edit `sdkconfig.defaults`:
```
CONFIG_COMPILER_OPTIMIZATION_SIZE=y
```

---

## Clean Build
```bash
idf.py fullclean
idf.py build
```

---

## Verify Installation
After flashing, device should:
1. Show orange "NetRaze32" splash screen
2. Display progress bar
3. Load main menu with 10 items
4. Respond to touch input

If not, check serial monitor for errors.
