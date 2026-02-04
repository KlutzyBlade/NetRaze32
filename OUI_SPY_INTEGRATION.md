# OUI-Spy Integration for NetRaze32

## Overview

OUI-Spy functionality has been integrated into NetRaze32, providing BLE device detection and tracking based on MAC addresses or OUI (Organizationally Unique Identifier) prefixes.

## Features

- **Built-in OUI Database**: 35+ surveillance/security device OUIs
  - Ring doorbells/cameras (11 OUIs)
  - Axon body cameras
  - Flock Safety ALPR cameras
  - DJI drones (8 OUIs)
  - Parrot drones (5 OUIs)
  - Skydio drones
  - Hak5 devices
  - Alfa Networks
  - Apple devices (6 OUIs)
- **OUI Filtering**: Detect devices by manufacturer (first 3 bytes of MAC)
- **Full MAC Matching**: Track specific devices by complete MAC address
- **Device Tracking**: Persistent device history with RSSI values
- **Cooldown System**: Prevents alert spam (3-second cooldown)
- **Real-time Display**: Shows detected devices with signal strength

## Location

**Menu Path**: Bluetooth → OUI-Spy

## Usage

### Adding Filters

```c
// Add OUI filter (manufacturer)
oui_spy_add_filter("aa:bb:cc", "Apple Devices");

// Add full MAC filter (specific device)
oui_spy_add_filter("aa:bb:cc:dd:ee:ff", "My iPhone");
```

### Running Scan

1. Navigate to Bluetooth menu
2. Select "OUI-Spy"
3. Scan runs for 30 seconds
4. Touch screen to exit early

### Display Format

```
OUI-Spy Scanner
─────────────────
DB: 35 OUIs | Filters: 0
Scanning...

Detected: 3 devices
18:7f:88:11:22:33 -45 Ring
aa:bb:cc:44:55:66 -52 Apple
dd:ee:ff:77:88:99 -68
```

## Built-in OUI Database

The following manufacturers are automatically detected:

### Surveillance/Security
- **Ring** (11 OUIs) - Doorbells, cameras, chimes
- **Axon** (1 OUI) - Body cameras, law enforcement
- **Flock Safety** (1 OUI) - ALPR cameras

### Drones
- **DJI** (8 OUIs) - Mavic, Phantom, Inspire, Mini
- **Parrot** (5 OUIs) - Anafi, Bebop, AR.Drone
- **Skydio** (1 OUI) - Skydio 2/3/X2

### Security Research
- **Hak5** (1 OUI) - WiFi Pineapple, etc.
- **Alfa Networks** (1 OUI) - WiFi adapters

### Common Devices
- **Apple** (6 OUIs) - iPhones, iPads, MacBooks

**Total: 35 OUIs**

No configuration needed - these are detected automatically!

## Implementation Details

### Files Added

- `main/oui_spy.h` - Header file with API
- `main/oui_spy.c` - Implementation

### Files Modified

- `main/menu.c` - Added OUI-Spy to Bluetooth submenu
- `main/main.c` - Initialize OUI-Spy on boot
- `main/CMakeLists.txt` - Added oui_spy.c to build

### Memory Usage

- **Filters**: Max 10 filters (OUI or MAC)
- **Devices**: Max 50 tracked devices
- **Storage**: ~2KB RAM

## API Reference

```c
// Initialize OUI-Spy (called automatically)
esp_err_t oui_spy_init(void);

// Add filter (OUI or full MAC)
void oui_spy_add_filter(const char* mac_or_oui, const char* description);

// Clear all filters and devices
void oui_spy_clear_filters(void);

// Get number of detected devices
int oui_spy_get_device_count(void);

// Run scan (30 seconds)
void oui_spy_scan(void);
```

## Differences from Original

The NetRaze32 implementation is **minimal** compared to the full OUI-Spy detector:

### Not Included
- Web configuration portal
- WiFi AP mode
- Device aliasing
- Persistent NVS storage
- Audio/buzzer feedback
- NeoPixel LED support
- Burn-in configuration
- JSON serial output

### Included
- BLE scanning with OUI/MAC filtering
- Real-time device detection
- RSSI tracking
- Cooldown system
- Touch-based UI

## Future Enhancements

To add full OUI-Spy features:

1. **Web Portal**: Add WiFi AP + web server for filter configuration
2. **Storage**: Use NVS to persist filters and device history
3. **Audio**: Add buzzer alerts on detection
4. **Aliases**: Allow custom device names
5. **Export**: Save detected devices to SD card

## Example Use Cases

### Track Apple Devices
```c
oui_spy_add_filter("00:1c:b3", "Apple Inc");
oui_spy_add_filter("00:1e:c2", "Apple Inc");
oui_spy_add_filter("00:1f:5b", "Apple Inc");
```

### Monitor Specific Device
```c
oui_spy_add_filter("aa:bb:cc:dd:ee:ff", "Target Phone");
```

### Detect Surveillance
```c
oui_spy_add_filter("00:13:37", "Hak5");
oui_spy_add_filter("00:c0:ca", "Alfa Networks");
```

## Credits

Based on [OUI-Spy Detector](https://github.com/colonelpanichacks/ouispy-detector) by colonelpanichacks.

Adapted for NetRaze32 ESP32-32E hardware with minimal footprint.
