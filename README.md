# NetRaze32

This is an ESP-IDF compilation of pen-testing functions based on currently circulating software such as Bruce, Marauder, ESP-DIV and others, all vibe coded together with A.I. specifically configured for the 2.8" ESP32-32E Display board from LCDWiki.

## Hardware Requirements

- 2.8" ESP32-32E Display board (https://www.lcdwiki.com/2.8inch_ESP32-32E_Display)
- ILI9341 240x320 TFT display
- XPT2046 touch controller

## Pin Configuration

The project is configured for the ESP32-32E display board with the following pin assignments:

### Display (ILI9341)
- CS: GPIO15
- DC: GPIO2  
- RST: Connected to ESP32 EN pin (auto-reset)
- BL: GPIO21
- MOSI: GPIO13
- MISO: GPIO12
- SCK: GPIO14

### Touch (XPT2046)
- CS: GPIO33
- IRQ: GPIO36
- MOSI: GPIO32
- MISO: GPIO39
- SCK: GPIO25

### Navigation
- Touch-based navigation with virtual buttons on screen
- Direct touch selection of menu items

## Build Instructions

1. Install ESP-IDF v5.0 or later
2. Set up ESP-IDF environment:
   ```
   . $HOME/esp/esp-idf/export.sh
   ```

3. Navigate to project directory:
   ```
   cd ESP32-DIV-ESP-IDF
   ```

4. Configure the project:
   ```
   idf.py menuconfig
   ```

5. Build the project:
   ```
   idf.py build
   ```

6. Flash to device:
   ```
   idf.py -p COMx flash monitor
   ```
   (Replace COMx with your actual COM port)

## Features

- Full touchscreen navigation with virtual buttons
- Menu system with WiFi, Bluetooth, and other tools
- Battery voltage monitoring
- Optimized for ESP32-32E display board
- No physical buttons required

## Project Structure

- `main/` - Main application code
- `components/` - Custom components (if any)
- `board_config.h` - Hardware pin definitions
- `display.c/h` - ILI9341 display driver
- `touchscreen.c/h` - XPT2046 touch driver
- `menu.c/h` - Menu system
- `utils.c/h` - Utility functions
- `wifi_functions.c/h` - WiFi functionality
- `bluetooth_functions.c/h` - Bluetooth functionality

## Notes

- This is a minimal port focusing on the core functionality
- The display driver uses basic SPI communication optimized for the ESP32-32E board
- Touch calibration may need adjustment based on your specific display
