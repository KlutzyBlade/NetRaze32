# NetRaze32 Production Release

**Version:** 1.0.0  
**Release Date:** January 2026  
**Target Hardware:** ESP32-32E with 2.8" ILI9341 Display

## What's Included

- Complete source code for ESP32-based security testing device
- WiFi security testing suite (deauth, evil twin, WPS, packet capture)
- Bluetooth/BLE attack capabilities
- SubGHz RF support (requires CC1101 module)
- Touch-based user interface
- SD card logging (PCAP format)
- Attack automation and scheduling
- Battery monitoring

## Legal Notice

This software is provided for educational and authorized security testing purposes only.

**WARNING:** Unauthorized access to computer networks is illegal. Users are responsible for ensuring compliance with all applicable laws and regulations in their jurisdiction.

By using this software, you agree to:
- Only use it on networks you own or have explicit permission to test
- Comply with all local, state, and federal laws
- Accept full responsibility for your actions

The developers assume no liability for misuse of this software.

## Build & Flash

See BUILD.md for complete instructions.

Quick start:
```bash
idf.py build
idf.py -p PORT flash
```

## Support

- Documentation: /docs/
- Hardware guide: docs/CC1101_WIRING.md
- Issues: Report via GitHub

## License

See LICENSE file for terms and conditions.
