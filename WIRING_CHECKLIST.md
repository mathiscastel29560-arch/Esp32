# ESP32-S3 Wiring Checklist - Complete Pin Configuration

**Device:** ESP32-S3-N16R8  
**Last Updated:** 2026-09-28  
**Status:** ✓ Dual-bus optimized, zero conflicts

---

## Pre-Assembly Verification

- [ ] Read GPIO_AUDIT.md completely
- [ ] Read this checklist completely
- [ ] Gather all components (list below)
- [ ] Verify component datasheets match GPIO assignments
- [ ] Use multimeter for continuity checks after soldering

---

## Component Checklist

### Display & Touchscreen
- [ ] 2.8" ILI9341 TFT LCD Display (320x240)
  - Datasheet reference: ILI9341 controller
  - Required pinout: CS, DC, MOSI, MISO, SCK, RST, BL

- [ ] XPT2046 Touchscreen Controller (on same board usually)
  - Datasheet reference: XPT2046 4-wire touch
  - Required pinout: CS, MOSI, MISO, SCK

### Radio Modules (SPI Bus 2)
- [ ] CC1101 433MHz Sub-GHz Transceiver
  - Datasheet: TI CC1101
  - Frequency: 433.92 MHz (fixed)
  - Required signals: CS, GDO0, GDO2, SPI (MOSI/MISO/SCK)

- [ ] NRF24L01+ 2.4GHz Transceiver with PA/LNA
  - Datasheet: Nordic NRF24
  - Frequency: 2400-2525 MHz (tunable)
  - Required signals: CS, CE, IRQ, SPI (MOSI/MISO/SCK)

### Sensors & Peripherals
- [ ] DS3231 RTC Clock Module
  - I2C Address: 0x68
  - Required: SDA, SCL, power, ground

- [ ] NEO-6M GPS Module
  - UART: 9600 baud
  - Required: RX, TX, power, ground

- [ ] 4-button membrane keypad (UP, DOWN, SELECT, BACK)
  - Type: momentary switches to GND
  - Pull-ups: Internal GPIO pullups (INPUT_PULLUP)

- [ ] Buzzer (active or passive with driver transistor)
  - GPIO 21 output
  - Power: ≤500mA at 3.3V

- [ ] IR Receiver (VS1838B 38kHz demodulator)
  - GPIO 39 input
  - Power: ≤50mA at 3.3V

- [ ] IR LED (940nm with transistor driver)
  - GPIO 38 output
  - Power: via transistor driver (~50-200mA at 3.3V)

- [ ] Battery Voltage Monitor
  - Resistor divider on GPIO 7 (ADC)
  - Typical: 100k + 100k (1:2 divider)

---

## Detailed Wiring Guide

### SPI Bus 1 (Display & Touchscreen) — 40MHz

```
TFT Display (ILI9341):
├─ VCC      → 3.3V (regulated)
├─ GND      → GND
├─ CS       → GPIO 5 (chip select, active low)
├─ DC       → GPIO 16 (data/command)
├─ RST      → Board reset (not GPIO, tied to reset line)
├─ MOSI     → GPIO 11 (SPI Bus 1)
├─ MISO     → GPIO 13 (SPI Bus 1)
├─ SCK      → GPIO 12 (SPI Bus 1)
└─ BL       → GPIO 48 (backlight PWM)

Touchscreen (XPT2046):
├─ VCC      → 3.3V
├─ GND      → GND
├─ CS       → GPIO 45 (chip select, active low)
├─ DIN/MOSI → GPIO 11 (SPI Bus 1)
├─ DO/MISO  → GPIO 13 (SPI Bus 1)
├─ CLK/SCK  → GPIO 12 (SPI Bus 1)
└─ IRQ      → GPIO 46 (optional, not currently used)
```

**Verification:**
```
Multimeter test (power off):
- GPIO 5, 16: Continuity to TFT CS, DC ✓
- GPIO 45: Continuity to touch CS ✓
- GPIO 11, 12, 13: Shared SPI bus ✓
- GPIO 48: PWM signal to backlight ✓
```

### SPI Bus 2 (RF Modules) — 10MHz

```
CC1101 (433MHz):
├─ VCC      → 3.3V
├─ GND      → GND
├─ CS       → GPIO 10 (chip select, active low)
├─ GDO0     → GPIO 4 (RX interrupt, active high)
├─ GDO2     → GPIO 40 (status line, optional)
├─ MOSI/SI  → GPIO 22 (SPI Bus 2)
├─ MISO/SO  → GPIO 25 (SPI Bus 2)
├─ SCK      → GPIO 24 (SPI Bus 2)
└─ Antenna  → 433MHz monopole or dipole (~17cm)

NRF24 (2.4GHz):
├─ VCC      → 3.3V (add 10µF capacitor close to VCC pin)
├─ GND      → GND
├─ CS       → GPIO 14 (chip select, active low)
├─ CE       → GPIO 15 (chip enable, active high)
├─ IRQ      → GPIO 41 (interrupt, active low)
├─ MOSI     → GPIO 22 (SPI Bus 2)
├─ MISO     → GPIO 25 (SPI Bus 2)
├─ SCK      → GPIO 24 (SPI Bus 2)
└─ Antenna  → 2.4GHz chip antenna or external
```

**Verification:**
```
Multimeter test (power off):
- GPIO 10: Continuity to CC1101 CS ✓
- GPIO 4, 40: Continuity to CC1101 GDO pins ✓
- GPIO 14, 15: Continuity to NRF24 CS, CE ✓
- GPIO 41: Continuity to NRF24 IRQ ✓
- GPIO 22, 24, 25: Shared SPI Bus 2 ✓
```

### I2C Bus (0x68 RTC, optional 0x24 NFC)

```
DS3231 RTC:
├─ VCC      → 3.3V
├─ GND      → GND
├─ SDA      → GPIO 8 (I2C SDA, pull-up usually on module)
├─ SCL      → GPIO 9 (I2C SCL, pull-up usually on module)
└─ 32.768   → Onboard crystal (not GPIO)

PN532 NFC (optional, not in base config):
├─ VCC      → 3.3V
├─ GND      → GND
├─ SDA      → GPIO 8 (I2C SDA, shared with RTC)
├─ SCL      → GPIO 9 (I2C SCL, shared with RTC)
└─ Antenna  → MIFARE/NFC 13.56MHz
```

**Verification:**
```
Multimeter test (power off):
- GPIO 8: I2C pull-ups present, continuity to SDA ✓
- GPIO 9: I2C pull-ups present, continuity to SCL ✓
```

### UART1 (GPS, 9600 baud)

```
NEO-6M GPS:
├─ VCC      → 3.3V
├─ GND      → GND
├─ RX       → GPIO 18 (ESP32 receives from GPS TX)
├─ TX       → GPIO 17 (ESP32 sends to GPS RX)
├─ PPS      → Not connected (optional 1-pulse-per-second)
└─ Antenna  → Active or passive 1575.42 MHz L1
```

**Verification:**
```
Multimeter test (power off):
- GPIO 17: Continuity to GPS RX ✓
- GPIO 18: Continuity to GPS TX ✓
```

### Buttons (4-way Menu)

```
Buttons (all tied to GND when pressed):
├─ UP       → GPIO 1 (INPUT_PULLUP)
├─ DOWN     → GPIO 2 (INPUT_PULLUP)
├─ SELECT   → GPIO 6 (INPUT_PULLUP)
├─ BACK     → GPIO 42 (INPUT_PULLUP)
└─ Common   → GND (shared)

Note: Internal pullups enabled in firmware
      Add 10-100nF capacitor if board is noisy
```

**Verification:**
```
Multimeter test (power off):
- GPIO 1, 2, 6, 42: ~47kΩ resistance to GND (pullup R) ✓

Multimeter test (power on, button not pressed):
- GPIO 1, 2, 6, 42: ~3.3V (pullup active) ✓
- GPIO 1, 2, 6, 42: ~0V (pullup with button pressed) ✓
```

### Audio Output

```
Buzzer:
├─ GPIO 21  → PWM signal
├─ GND      → Ground
└─ Power    → 3.3V (via PWM driver transistor)

Note: PWM frequency for tones is tuned in firmware
      Can generate beeps at any frequency 100Hz-20kHz
```

**Verification:**
```
Multimeter test (power off):
- GPIO 21: Continuity to buzzer signal ✓

Test with tone:
- Play tone on buzzer (menu → diagnostics → test buzzer) ✓
```

### IR Subsystem

```
IR Receiver (VS1838B):
├─ VCC      → 3.3V
├─ GND      → GND
└─ OUT      → GPIO 39 (active low on IR detection)

IR Transmitter (940nm LED with driver transistor):
├─ LED+     → Transistor collector (via 100Ω resistor)
├─ LED-     → GND
├─ Base     → GPIO 38 (PWM for modulation)
└─ Power    → Via transistor (3.3V with current limiting)

Note: Modulation frequency ~38kHz for TV remote control
```

**Verification:**
```
Visual test:
- Receive IR remote, watch GPIO 39 voltage drop ✓
- Transmit IR: Watch LED pulse (camera can see IR) ✓
```

### Power Management

```
Battery Monitor (Resistor Divider):
├─ Battery+ → R1 (100kΩ typical)
├─ R1→R2    → GPIO 7 (ADC1 CH6)
├─ R2       → GND
├─ R2 Value → 100kΩ (creates 1:2 divider)
└─ Expected: 0-2.048V at ADC when battery 0-4.1V

Note: Actual resistor values depend on battery voltage
      Typical 0-4.2V battery → 0-2.1V ADC input
```

**Verification:**
```
Multimeter test (power on):
- GPIO 7: 0-2.048V depending on battery voltage ✓
- Check voltage increases with charging ✓
- Check percentage displayed in menu matches actual ✓
```

---

## Assembly Sequence

### Phase 1: Core Hardware (10 mins)
1. [ ] Solder ESP32-S3 module to breakout board
2. [ ] Solder I2C pull-ups (if not on RTC module) on GPIO 8, 9
3. [ ] Solder bypass capacitors (100nF ceramic) on VCC/GND near each IC
4. [ ] Verify power supply: 3.3V, <1A capacity

### Phase 2: Display System (15 mins)
5. [ ] Solder TFT Display SPI Bus 1 connections:
   - GPIO 11, 12, 13 → shared SPI pins
   - GPIO 5 → TFT CS (chip select)
   - GPIO 16 → TFT DC (data/command)
   - GPIO 48 → TFT BL (backlight)
   - Board reset → TFT RST

6. [ ] Solder Touchscreen SPI Bus 1 connections:
   - GPIO 11, 12, 13 → same SPI bus as TFT
   - GPIO 45 → Touch CS (separate from TFT)
   
7. [ ] Power on, verify display shows boot logo

### Phase 3: Radio Modules (20 mins)
8. [ ] Solder CC1101 SPI Bus 2 connections:
   - GPIO 22, 24, 25 → dedicated SPI bus 2
   - GPIO 10 → CC1101 CS
   - GPIO 4 → CC1101 GDO0 (RX interrupt)
   - GPIO 40 → CC1101 GDO2 (optional)
   - 433MHz antenna (17cm monopole)

9. [ ] Solder NRF24 SPI Bus 2 connections:
   - GPIO 22, 24, 25 → same SPI bus 2 as CC1101
   - GPIO 14 → NRF24 CS
   - GPIO 15 → NRF24 CE
   - GPIO 41 → NRF24 IRQ
   - 10µF capacitor close to NRF24 VCC
   - 2.4GHz antenna

10. [ ] Test isolated RF modules (menu → hardware test)

### Phase 4: Sensors & Peripherals (15 mins)
11. [ ] Solder I2C devices:
    - GPIO 8, 9 → DS3231 RTC
    - Run hardware test for RTC

12. [ ] Solder UART1 GPS:
    - GPIO 17, 18 → NEO-6M GPS
    - Run hardware test for GPS (wait for satellite fix)

13. [ ] Solder buttons:
    - GPIO 1, 2, 6, 42 → 4-button membrane
    - Test button presses in menu

14. [ ] Solder peripherals:
    - GPIO 21 → Buzzer (test tone in menu)
    - GPIO 38, 39 → IR TX/RX (test IR reflection)
    - GPIO 7 → Battery ADC (verify voltage reading)

### Phase 5: Final Verification (10 mins)
15. [ ] Run complete hardware test suite
16. [ ] Verify all menu navigation works
17. [ ] Test each feature briefly
18. [ ] Check memory usage (menu → debug info)

---

## Post-Assembly Testing

### Electrical Safety
```
[ ] Continuity test: VCC → GND through 3.3V regulator (resistance ~0Ω)
[ ] Isolation test: VCC (3.3V) does not connect to GND directly (resistance >1MΩ)
[ ] No shorts: Test each GPIO pin does not short to neighbors
```

### GPIO Functionality
```
[ ] Buttons: All 4 respond in menu without glitches
[ ] Buzzer: Makes tones at different frequencies
[ ] IR: Receiver detects remote, transmitter sends
[ ] Battery: ADC reading matches actual voltage
```

### Display System
```
[ ] TFT: Shows menu at 40MHz, colors correct
[ ] Touch: Calibrates and responds to screen taps
[ ] Backlight: PWM dims/brightens
```

### Radio Modules
```
[ ] CC1101: Detects 433MHz signals, shows RSSI
[ ] NRF24: Scans 2.4GHz channels, shows signal strength
[ ] Both together: No interference between modules
```

### Sensors
```
[ ] RTC: Shows current time, temperature, battery flag
[ ] GPS: Acquires satellites within 30 seconds
```

### Comprehensive Test
```
[ ] Run Hardware Test Mode (isolates each driver)
[ ] Check all 13 menu tabs load without crashes
[ ] Verify no memory leaks (heap stable after 1 minute)
[ ] Test WiFi scanning (if enabled)
[ ] Test BLE scanning (if enabled)
[ ] Save and restore settings
```

---

## Troubleshooting Quick Reference

### Display doesn't show anything
- Check GPIO 11, 12, 13 continuity to TFT
- Verify GPIO 5 (TFT CS), GPIO 16 (TFT DC)
- Check TFT backlight (GPIO 48) getting PWM
- Try factory reset: Hold BACK button on boot

### Touchscreen doesn't respond
- Check GPIO 45 continuity (Touch CS)
- Calibrate: Menu → Settings → Calibrate Touch
- Check that Touch is on SPI Bus 1 (GPIO 11, 12, 13)

### CC1101 shows "module not connected"
- Check GPIO 10 (CS), GPIO 4 (GDO0), GPIO 40 (GDO2)
- Check SPI Bus 2: GPIO 22, 24, 25
- Verify antenna connection (433MHz)
- Antenna should be ~17cm monopole or dipole

### NRF24 shows "module not connected"
- Check GPIO 14 (CS), GPIO 15 (CE), GPIO 41 (IRQ)
- Check SPI Bus 2 shared with CC1101: GPIO 22, 24, 25
- Add 10µF capacitor to NRF24 VCC if unstable
- Verify 2.4GHz antenna connection

### GPS doesn't acquire satellites
- Check GPIO 17, 18 UART continuity
- Outdoor location with clear sky view required
- Wait up to 30 seconds for first fix (hot start ~5 seconds)
- Check LED on GPS module (should blink 1/second when locked)

### RTC shows wrong time
- Check GPIO 8, 9 I2C continuity
- RTC keeps time on battery when powered off
- Set time in menu: Settings → Date/Time

### Buttons don't respond
- Check GPIO 1, 2, 6, 42 continuity to GND
- Verify pullups are ~47kΩ resistance to GND
- Try cleaning button membrane contacts
- Test in menu with debug output enabled

### No sound from buzzer
- Check GPIO 21 continuity
- Check transistor driver (if present)
- Test with menu test tone
- Verify 3.3V power to buzzer circuit

### Battery reads 0% or 100%
- Check GPIO 7 resistor divider values
- Typical: 100kΩ + 100kΩ (1:2 divider)
- Check ADC connection: should read 0-4095 counts
- Calibrate: Measure actual battery voltage vs. reported

---

## Electrical Specifications

```
Supply Voltage:        3.3V ±5% (1.8V-3.6V logic)
Supply Current:        ~300mA nominal, ~700mA peak (WiFi/BLE+TX)
GPIO Maximum:          40mA per pin, 200mA total
ADC Resolution:        12-bit (0-4095 counts)
I2C Speed:             400 kHz (standard mode)
SPI Bus 1 Speed:       40 MHz (display)
SPI Bus 2 Speed:       10 MHz (RF modules)
UART Speed:            9600 baud (GPS)
```

---

## Bill of Materials (BOM) Summary

Component | Qty | Part Number | Notes
-----------|-----|-------------|-------
ESP32-S3-N16R8 | 1 | - | Dual-core, 16MB flash, 8MB PSRAM
TFT Display 2.8" | 1 | ILI9341 | 320x240 color, SPI interface
XPT2046 Touch | 1 | - | Integrated with TFT board usually
CC1101 Radio | 1 | TI CC1101 | 433MHz sub-GHz
NRF24L01+PA/LNA | 1 | Nordic NRF24 | 2.4GHz WiFi band
DS3231 RTC | 1 | - | I2C real-time clock with battery
NEO-6M GPS | 1 | u-blox NEO-6M | UART NMEA output
4-Button Keypad | 1 | - | Membrane switch, momentary
Buzzer | 1 | - | Active or passive with driver
IR Receiver | 1 | VS1838B | 38kHz demodulator
IR LED | 1 | 940nm | With transistor driver
Resistors | 4 | 100kΩ | I2C pullups + battery divider
Capacitors | 6 | 100nF + 10µF | Power supply bypass, NRF24 bulk
Push buttons | 4 | - | UP, DOWN, SELECT, BACK

---

**Audit Status: ✓ COMPLETE - Ready for Assembly**

Following this checklist ensures zero GPIO conflicts and proper operation of all 9 hardware subsystems.
