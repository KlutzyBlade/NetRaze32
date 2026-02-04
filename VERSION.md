# NetRaze32 Version 1.0.0

**Release Date:** January 7, 2026  
**Build:** Production Release  
**Target:** ESP32-WROOM-32E  

## Release Notes

### New Features
- Complete WiFi security testing suite (11 functions)
- Bluetooth/BLE attack capabilities (9 functions)
- SubGHz RF support with CC1101 module
- Touch-based user interface
- SD card packet logging (PCAP format)
- Battery monitoring with runtime estimation
- Attack automation and scheduling
- Target profile management
- Real-time statistics tracking
- USB Mass Storage mode

### Hardware Support
- ESP32-32E with 2.8" ILI9341 display
- XPT2046 resistive touch controller
- MicroSD card slot
- Optional CC1101 SubGHz transceiver
- Battery voltage monitoring

### Performance
- Boot time: ~3 seconds
- Menu response: <100ms
- WiFi scan: ~2 seconds
- BLE scan: ~5 seconds
- Packet capture: Real-time to SD card

### Known Limitations
- GPS/NFC/IR require external modules (not included)
- CC1101 required for SubGHz features
- SD card must be FAT32 formatted
- Maximum 32GB SD card supported

### System Requirements
- ESP-IDF v5.5 or later
- Python 3.8+
- 4MB flash minimum
- USB data cable for programming

## Checksums

Build artifacts checksums (SHA256):
```
[Generated during build process]
```

## Compatibility

**Tested On:**
- ESP32-32E Development Board (LCDWiki 2.8")
- ESP-IDF v5.5
- Windows 10/11
- Ubuntu 22.04
- macOS 13+

**Not Compatible:**
- ESP32-S2/S3/C3 (different architecture)
- Other display sizes/controllers
- ESP8266

## Support

For issues, questions, or contributions:
- GitHub Issues
- Documentation: `/docs/`
- Community Wiki

---

**NetRaze32 v1.0.0** - Professional Security Testing Device
