# ESP32-S3 Audit Tool - Deployment Guide

## Table of Contents
1. [Hardware Assembly](#hardware-assembly)
2. [Pin Configuration Verification](#pin-configuration-verification)
3. [Software Setup](#software-setup)
4. [Flashing the Firmware](#flashing-the-firmware)
5. [Hardware Validation](#hardware-validation)
6. [Post-Deployment Checklist](#post-deployment-checklist)
7. [Troubleshooting](#troubleshooting)
8. [Field Operation](#field-operation)

---

## Hardware Assembly

### Component List
- **ESP32-S3-DevKitC-1** (or compatible ESP32-S3 board)
- **CC1101** (433MHz SubGHz transceiver)
- **NRF24L01+** (2.4GHz transceiver)
- **PN532** (NFC/RFID reader module)
- **DS3231** (Real-Time Clock)
- **SSD1306** (128x64 OLED display)
- **Button module** (for menu navigation)
- **Battery management** (for portable operation)
- **Buzzer** (GPIO21, for audio feedback)
- **LEDs** (for visual indicators)

### Soldering Checklist

#### 1. SPI Bus Connections (SCK=GPIO36, MOSI=GPIO35, MISO=GPIO37)
These three pins are shared by TFT, CC1101, and NRF24:

**CC1101 (433MHz)**
- SCK → GPIO36
- MOSI → GPIO35
- MISO → GPIO37
- CS → GPIO10
- GND → GND
- VCC → 3.3V

**NRF24L01+**
- SCK → GPIO36
- MOSI → GPIO35
- MISO → GPIO37
- CS → GPIO11
- CE → GPIO12
- VCC → 3.3V
- GND → GND

**SSD1306 Display (I2C)**
- SCL → GPIO9
- SDA → GPIO8
- VCC → 3.3V
- GND → GND

#### 2. I2C Bus Connections (SCL=GPIO9, SDA=GPIO8)
Shared by RTC and PN532:

**DS3231 RTC**
- SCL → GPIO9
- SDA → GPIO8
- VCC → 3.3V
- GND → GND

**PN532 NFC Reader**
- SCL → GPIO9
- SDA → GPIO8
- VCC → 3.3V
- GND → GND

#### 3. UART Connections

**GPS Module (UART1)**
- TX → GPIO20
- RX → GPIO19
- VCC → 3.3V
- GND → GND

#### 4. GPIO Connections

**Button Matrix**
- Button 1 (Enter/Select) → GPIO18
- Button 2 (Back) → GPIO17
- Button 3 (Next) → GPIO16
- GND → GND (common ground)

**Buzzer**
- Signal → GPIO21 (PWM capable)
- GND → GND

**Battery ADC**
- Battery Voltage → GPIO4 (ADC input)
- GND → GND

### Recommended Layout
```
ESP32-S3 with shared SPI bus (SCK/MOSI/MISO)
├─ CC1101 (CS=GPIO10)
├─ NRF24 (CS=GPIO11, CE=GPIO12)
└─ Display (if SPI)

Shared I2C bus (SCL=GPIO9, SDA=GPIO8)
├─ DS3231 RTC @ 0x68
└─ PN532 NFC @ 0x24

UART1 (TX=GPIO20, RX=GPIO19)
└─ GPS Module @ 9600bps

GPIO
├─ Buzzer (PWM, GPIO21)
├─ Battery ADC (GPIO4)
└─ Status LEDs (GPIO5,13,14)
```

---

## Pin Configuration Verification

Verify in `include/config.h`:

```cpp
// SPI Bus (Shared)
#define PIN_SCK    36
#define PIN_MOSI   35
#define PIN_MISO   37

// Chip Select
#define PIN_CS_CC1101   10
#define PIN_CE_NRF24    12
#define PIN_CS_NRF24    11

// I2C
#define PIN_I2C_SDA  8
#define PIN_I2C_SCL  9

// UART1 (GPS)
#define PIN_UART1_TX  20
#define PIN_UART1_RX  19

// GPIO
#define PIN_BUZZER       21
#define PIN_BATTERY_ADC  4
```

---

## Software Setup

### Prerequisites
- PlatformIO CLI
- Python 3.7+
- USB cable

### Install & Build
```bash
cd /home/user/Esp32
pio pkg install
pio run -e esp32-s3-devkitc-1
```

---

## Flashing the Firmware

### Connect & Upload
```bash
# Check device
ls /dev/ttyUSB*

# Flash
pio run -e esp32-s3-devkitc-1 -t upload

# Monitor output
pio device monitor -b 115200
```

### Expected Startup Output
```
╔═══════════════════════════════════════╗
║    ESP32-S3 Audit Tool v2.0.0        ║
║     Hardware Initialization           ║
╚═══════════════════════════════════════╝

[✓] GPIO System Initialized
[✓] ADC Battery Monitoring Ready
[✓] Buzzer Audio Ready

[I2C Bus @ 100kHz]
  [✓] DS3231 RTC @ 0x68
  [✓] PN532 NFC @ 0x24

[SPI Bus @ 1MHz]
  [✓] CC1101 433MHz
  [✓] NRF24L01+ 2.4GHz

[UART1 @ 9600bps]
  [⚠] GPS Waiting...

[System] Ready for audit.
```

---

## Hardware Validation

### Test Mode
1. Power on → Main Menu
2. Navigate to "System" tab
3. Select "Hardware Test"
4. Run tests:

- **GPIO Test**: Button debounce (20ms), long press (1000ms)
- **RTC Test**: Time, temperature, battery status
- **GPS Test**: NMEA parsing, satellite lock
- **PN532 Test**: NFC firmware, card detection
- **CC1101 Test**: 433MHz initialization, frequency
- **NRF24 Test**: 2.4GHz initialization, addresses

### Verification Checklist
- [ ] All tests pass
- [ ] Serial monitor no errors
- [ ] Menu responsive
- [ ] Display visible
- [ ] Buzzer sounds
- [ ] Battery level shows
- [ ] Time displays

---

## Troubleshooting

### Device Not Detected
```bash
lsusb | grep Silicon
sudo usermod -a -G dialout $USER
# Logout and login
```

### Upload Fails
```bash
pio run -t erase
pio run -t upload
```

### Hardware Test Fails

**RTC (I2C 0x68)**
- Check GPIO9/GPIO8 connections
- Verify 3.3V power

**CC1101 (SPI, 433MHz)**
- Check GPIO36/35/37 (SCK/MOSI/MISO)
- Verify GPIO10 (CS)
- Must be 3.3V (NOT 5V)

**NRF24 (SPI, 2.4GHz)**
- Check GPIO36/35/37 connections
- Verify GPIO11 (CS), GPIO12 (CE)
- Needs 150mA+ current

**PN532 (I2C 0x24)**
- Check GPIO9/GPIO8
- Same I2C bus as RTC

**GPS (UART1, 9600bps)**
- Check GPIO20 (TX), GPIO19 (RX)
- Allow 30+ seconds for cold start

### Garbled Serial Output
```bash
# Must use 115200 baud
pio device monitor -b 115200
```

### Random Resets
- Check power supply (stable 5V)
- Reduce clock speed if needed
- Check temperature (<60°C)

---

## Field Operation

### Before Audit
- [ ] Battery fully charged
- [ ] All tests pass
- [ ] Time synchronized
- [ ] GPS locked (if needed)

### During Audit
- [ ] Monitor power usage
- [ ] Note RF interference
- [ ] Save results

### After Audit
- [ ] Export logs
- [ ] Backup LittleFS
- [ ] Recharge battery

---

## Support

- **Issues**: GitHub Issues
- **Docs**: CLAUDE.md
- **Pinout**: include/config.h

Last Updated: 2026-09-22
Version: 2.0.0
