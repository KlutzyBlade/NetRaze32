# Production Update - SD Card Disabled, Internal Flash Storage

## Date: 2024
## Version: 1.0.1

## Summary
SD card functionality disabled due to hardware pin conflict on ESP32-32E board.
All storage migrated to internal flash (NVS + SPIFFS).

## Hardware Limitation
ESP32-32E SD card slot uses SDIO pins that overlap with display pins:
- SD CMD (GPIO15) = Display CS
- SD CLK (GPIO14) = Display SCK  
- SD D0 (GPIO2) = Display DC
- SD D3 (GPIO13) = Display MOSI

Cannot use both simultaneously.

## Files Modified

### Core Changes
1. **main/sd_card.c** - Disabled with error message
2. **main/board_config.h** - Documented pin conflict
3. **main/display.c** - Removed SPI conflict flags
4. **main/menu.c** - Removed SD Browser from Tools menu (count: 4 items)

### Storage Migration  
5. **main/settings_menu.c** - NVS storage (was SD card)
6. **main/target_manager.c** - NVS storage (was SD card)
7. **main/packet_capture.c** - SPIFFS storage (was SD card, limited to ~1-2MB)

### Bug Fixes
8. **main/rf_functions.c** - Fixed spectrum analyzer crash on exit
9. **main/wifi_functions.c** - Updated packet capture path to SPIFFS
10. **main/utils.c** - Removed SD card indicator from status bar

### Removed Files
- main/usb_msc.c (stub, not functional)
- main/usb_msc.h (stub, not functional)

## Storage Details

### NVS (Non-Volatile Storage)
- Settings: namespace "settings", blob storage
- Targets: namespace "targets", blob storage  
- Persistent across reboots
- No size limitations for small data

### SPIFFS (File System)
- Packet captures: /spiffs/*.pcap
- Limited to ~1-2MB total
- Suitable for short captures only

## Functional Status
✅ All WiFi/BLE/RF attacks working
✅ Settings persist (NVS)
✅ Targets persist (NVS)
✅ Packet capture (SPIFFS, limited)
✅ Spectrum analyzer fixed
❌ SD Browser removed
❌ Large packet captures not supported

## Build Instructions
No changes to build process. Standard ESP-IDF build:
```
idf.py build
idf.py flash
```

## Migration Notes
- Existing SD card data will NOT be migrated
- Users must reconfigure settings
- Targets must be re-saved
- Old packet captures on SD card remain accessible if card removed and read on PC
