# ESP32-S3 Offensive Security Platform - Project Documentation

## Overview

Professional offensive security platform built on ESP32-S3 with **real hardware drivers** (not simulated). Includes WiFi/BLE/RF/IoT attack tools with professional UI, debug logging, and isolated hardware testing.

**Key Features:**
- Real CC1101 (433MHz) & NRF24 (2.4GHz) radio transceivers
- NFC/RFID reader (PN532)
- GPS module (NEO-6M)
- RTC clock (DS3231)
- Professional serial menu with colors & status bar
- Real-time battery/WiFi/memory monitoring
- Isolated hardware test mode
- Professional results formatting

## Architecture

### Hardware Layer
```
ESP32-S3 (240MHz dual-core, 16MB Flash, 8MB PSRAM)
├── GPIO (buttons, buzzer, IR, battery ADC)
├── I2C (SDA=8, SCL=9)
│   ├── RTC DS3231 (0x68)
│   └── PN532 NFC/RFID (0x24)
├── UART1 (RX=18, TX=17, 9600 baud)
│   └── GPS NEO-6M
└── SPI (SCK=12, MOSI=11, MISO=13)
    ├── CC1101 (CS=10, GDO0=4, 433.92MHz)
    └── NRF24 (CS=14, CE=15, 2.4GHz)
```

### Software Architecture

```
main.cpp
├── Display::begin()
├── Hardware::initAll()
│   ├── GPIO init
│   ├── RTC init
│   ├── GPS init (with UART1 guard)
│   ├── PN532 init
│   ├── CC1101 init (with SubGhz guard)
│   └── NRF24 init
└── Menu::loop()
```

### Menu System

**Main Menu (13 tabs):**
1. WiFi Tools - Network scanning, deauth, jamming
2. BLE Tools - Device scanning, attacks
3. RF/2.4GHz - NRF24, drone tracking, IR
4. IoT/Advanced - Zigbee, MQTT, NFC, MIFARE
5. System - TX arm, battery, GPS, dualboot
6. Settings - Date/time, brightness, contrast
7. Hardware Test - Isolated driver tests
8. Device Info - Configuration, pinout, guards
9. Debug Info - Memory, battery, modules, errors
10. Calibration - ADC, RF signal, display
11. About - Version, serial, MAC, flash, uptime
12. Network - WiFi status, IP, hostname
13. Help - Documentation

### Key Files

**Hardware Drivers:**
- `src/drivers/cc1101_driver.cpp` - 433MHz transceiver
- `src/drivers/nrf24_driver.cpp` - 2.4GHz transceiver
- `src/drivers/gpio_driver.cpp` - Buttons, buzzer, battery
- `src/drivers/rtc_driver.cpp` - DS3231 clock
- `src/drivers/gps_driver.cpp` - NEO-6M GPS
- `src/drivers/pn532_driver.cpp` - NFC/RFID reader

**UI System:**
- `src/menu.cpp` - Main menu with 13 tabs
- `src/menu_enhanced.cpp` - Advanced visualization
- `src/results_formatter.cpp` - Professional result formatting
- `include/debug_logger.h` - Colored debug output

**Hardware Testing:**
- `src/hardware_test_mode.cpp` - Isolated driver tests
- `include/hardware_test_mode.h` - Test interface

**Configuration:**
- `include/hw_config.h` - Pin mapping (maps config.h pins)
- `include/HARDWARE_GUARDS.md` - Conflict documentation

## Hardware Conflicts & Guards

### 1. GPS/UART1 Conflict ⚠️
- **Issue:** GpsModule (TinyGPSPlus) and GPSDriver both use UART1
- **Guard:** Check Serial1.baudRate() before GPS init
- **Resolution:** Use one or the other, not both

### 2. CC1101/SubGhz Conflict ⚠️
- **Issue:** RadioLib CC1101 and CC1101Driver compete
- **Guard:** Warning message during CC1101Driver init
- **Resolution:** Choose SubGhz or CC1101Driver per session

### 3. SPI Bus Sharing ⚠️
- **Issue:** TFT, CC1101, NRF24 share SPI pins (SCK=12, MOSI=11, MISO=13)
- **Guard:** Separate CS pins (TFT via TFT_eSPI, CC1101=10, NRF24=14)
- **Resolution:** Initialize Display first, then radio modules

## UI Features

### Menu Display
- Status bar: WiFi ✓/✗, Battery [████░░] %, Time 🕐 HH:MM
- Selected item: Yellow highlight with green borders
- Icons: Unique emoji per menu section
- Descriptions: Info shown for selected items
- Navigation: ▲▼ navigate, ● select, ◄ back

### Results Formatting
- Success (green ✓): With progress bar
- Warning (yellow ⚠): Alert indicators
- Error (red ✗): Failure details
- Info (cyan ℹ): Status updates
- Scan (blue 📊): Table of results

### Advanced Visuals
- Battery bar: █████░░░░ 50%
- Signal strength: ████████░░ Excellent
- Progress bars: [██████░░░░] 60%
- Signal map: RSSI indicators per channel

## Testing

### Hardware Test Mode
Access from menu: **Hardware Test** tab

Each driver tested in isolation:
- GPIO: Button polling, buzzer, battery ADC
- RTC: Time display, temperature, battery flag
- GPS: 30-second satellite fix detection
- PN532: Firmware check, card scanning
- CC1101: 10-second 433MHz listening
- NRF24: 5-channel 2.4GHz sweep

### Isolation Benefits
- No conflicts between modules
- Easy diagnosis of hardware issues
- Safe validation before field use
- Detailed error messages

## Building & Flashing

### Prerequisites
```bash
# PlatformIO (recommended)
pip install platformio

# Arduino CLI alternative
arduino-cli core install esp32:esp32
```

### Build
```bash
cd /home/user/Esp32
pio run -e esp32s3 -t build
```

### Flash
```bash
pio run -e esp32s3 -t upload
```

### Monitor Serial
```bash
pio run -e esp32s3 -t monitor --baud 115200
```

## Pin Reference

### GPIO
```
Buttons:   UP=1, DOWN=2, SELECT=6, BACK=42
Buzzer:    GPIO 21 (PWM)
IR TX:     GPIO 38, IR RX: GPIO 39
Battery:   GPIO 7 (ADC)
```

### I2C (SDA=8, SCL=9)
```
RTC DS3231:    0x68
PN532 NFC:     0x24
```

### UART
```
UART1: RX=18, TX=17, Baud=9600 (GPS)
UART0: USB console
```

### SPI (SCK=12, MOSI=11, MISO=13)
```
CC1101 CS=10, GDO0=4, GDO2=40
NRF24  CS=14, CE=15, IRQ=41
```

## Deployment Checklist

- [ ] Verify all pin connections with multimeter
- [ ] Test GPIO (buttons, buzzer) - no conflicts
- [ ] Test I2C (RTC + PN532) - separate addresses
- [ ] Test UART1 (GPS) - confirm data reception
- [ ] Test CC1101 - run isolated test, verify TX/RX
- [ ] Test NRF24 - run isolated test, verify channels
- [ ] Test Display + one radio together
- [ ] Verify no memory leaks (check free heap)
- [ ] Test all menu navigation
- [ ] Validate results formatting
- [ ] Check battery monitoring
- [ ] Verify settings persistence

## Debug Output

All debug output uses color codes:
```
[ERROR]   Red    - Critical failures
[WARN]    Yellow - Warnings, degraded state
[INFO]    Green  - Normal operations
[VERBOSE] Cyan   - Detailed diagnostics
```

Enable/disable: `#define DEBUG_ENABLED 1` in `debug_logger.h`

## Future Improvements

- [ ] Live signal graph (2D plot)
- [ ] Tool result export (CSV/JSON)
- [ ] Log persistence (SD card)
- [ ] Web dashboard (over WiFi)
- [ ] Firmware OTA updates
- [ ] Keyboard shortcuts
- [ ] Wireless configuration
- [ ] Battery calibration UI
- [ ] Tool history/replay
- [ ] Custom attack sequences

## Troubleshooting

### No menu display?
1. Check serial connection (9600 baud, 8N1)
2. Verify UART0 on USB-C
3. Check Display::begin() succeeded
4. Verify buttons.h initialization

### Hardware test fails?
1. Check pins with multimeter
2. Review specific error message
3. Check HARDWARE_GUARDS.md for conflicts
4. Isolate module and test alone

### Memory issues?
1. Check free heap in Debug Info menu
2. Disable unnecessary debug output
3. Reduce buffer sizes if needed
4. Monitor PSRAM usage

### RF not working?
1. Verify SPI pins: SCK=12, MOSI=11, MISO=13
2. Check CS pins: CC1101=10, NRF24=14
3. Run isolated hardware tests
4. Check antenna connections

## Support

For issues:
1. Check serial logs (Debug Info menu)
2. Review HARDWARE_GUARDS.md
3. Run isolated tests (Hardware Test menu)
4. Check pins with multimeter
5. Review HARDWARE.md for pinout

---

**Project Version:** 2.0.0
**Hardware:** ESP32-S3-N16R8
**Last Updated:** 2025-09-22
