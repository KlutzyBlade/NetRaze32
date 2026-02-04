# NetRaze32 Build Instructions

## Prerequisites

- ESP-IDF v5.5 or later
- Python 3.8+
- Git
- USB drivers for ESP32

## Quick Start

1. Install ESP-IDF:
   ```bash
   # Follow official ESP-IDF installation guide
   # https://docs.espressif.com/projects/esp-idf/en/latest/get-started/
   ```

2. Set up environment:
   ```bash
   . $HOME/esp/esp-idf/export.sh
   ```

3. Clone and build:
   ```bash
   cd NetRaze32_Production
   idf.py build
   ```

4. Flash to device:
   ```bash
   idf.py -p PORT flash monitor
   ```
   Replace PORT with your serial port (e.g., COM3, /dev/ttyUSB0)

## Configuration

Default configuration is optimized for ESP32-32E with 2.8" display.
To customize:
```bash
idf.py menuconfig
```

## Troubleshooting

- Ensure USB cable supports data transfer
- Check correct COM port selection
- Verify ESP-IDF environment is activated
- Use `idf.py erase-flash` if experiencing issues

## Hardware Requirements

- ESP32-32E Development Board with 2.8" ILI9341 display
- MicroSD card (FAT32, max 32GB)
- Optional: CC1101 module for SubGHz features
