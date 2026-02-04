# CC1101 Module Integration Guide

## Hardware Connections

Your CC1101 module has 8 pins. Connect to ESP32-32E as follows:

### Pin Mapping
| CC1101 Pin | ESP32 GPIO | Wire Color (typical) |
|------------|------------|---------------------|
| VCC        | 3.3V       | Red                 |
| GND        | GND        | Black               |
| MOSI       | GPIO 23    | Blue                |
| MISO       | GPIO 19    | Green               |
| SCK        | GPIO 18    | Yellow              |
| CSN        | GPIO 27    | Orange              |
| GDO0       | GPIO 26    | Purple              |
| GDO2       | GPIO 5     | White (optional)    |

## Important Notes

1. **Voltage**: CC1101 is 3.3V only - DO NOT use 5V
2. **Antenna**: Module comes with 433MHz spring antenna
   - Pre-tuned for 433.92 MHz
   - ~17.3 cm length
   - Can replace with wire antenna if needed

3. **SPI Bus Sharing**: The CC1101 shares the SPI bus with:
   - ILI9341 Display (CS=GPIO15)
   - SD Card (CS=GPIO4)
   - CC1101 (CS=GPIO5)

## Software Features Enabled

Once connected, the following SubGHz menu items will work:

### Spectrum Analyzer
- Scans 315/433/868/915 MHz bands
- Real-time RSSI display
- Visual signal strength graph

### Signal Capture/Replay
- Capture remote control signals
- Save to SD card
- Replay captured signals

### Supported Protocols
- Car key fobs (315/433 MHz)
- Garage door openers
- Wireless doorbells
- Weather stations
- Tire pressure sensors

## Testing

1. Build and flash:
   ```bash
   idf.py build flash monitor
   ```

2. Check boot log for:
   ```
   CC1101 detected: Part=0x00 Ver=0x14
   ```

3. If not detected:
   - Check wiring
   - Verify 3.3V power
   - Ensure good GND connection
   - Check solder joints

## Troubleshooting

**"CC1101 Not Found" message:**
- Module not connected
- Wrong wiring
- Bad solder joints
- Faulty module

**No signal reception:**
- Missing antenna
- Wrong frequency selected
- Antenna too short/long
- Module in TX mode instead of RX

**Interference with display:**
- Should not occur (separate CS pins)
- If it does, add 10µF capacitor near CC1101 VCC

## Optional Enhancements

- Add external antenna connector (U.FL/SMA)
- Add RF amplifier for longer range
- Add band-pass filters for better selectivity
- Multiple CC1101 modules for simultaneous frequencies
