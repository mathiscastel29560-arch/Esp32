# Build and Hardware Validation Guide

## Build Status ✅

**Latest Build:** SUCCESS
- **Compiler:** PlatformIO / ESP32-S3 Xtensa GCC
- **Target:** ESP32-S3-N16R8 (Octal PSRAM, 16MB Flash)
- **Compilation Time:** ~90 seconds
- **Errors:** 0
- **Warnings:** 0

### Memory Usage

```
RAM:   [===       ]  25.1% (used 82,396 / 327,680 bytes)
Flash: [======    ]  61.9% (used 1,947,285 / 3,145,728 bytes)
```

**Headroom:**
- RAM: 245,284 bytes free (sufficient for dynamic buffers)
- Flash: 1,198,443 bytes free (19.1% - safe margin for future features)

### PSRAM Status

Octal PSRAM 8MB configured and available for:
- Display framebuffers (TFT sprite canvas)
- Scan result buffers
- WiFi/BLE packet capture
- Log storage during operations

---

## Hardware Diagnostics at Boot

### Automatic Detection

The firmware now runs a complete hardware diagnostic at boot (before showing the main menu). Each peripheral is tested and reports its status:

#### Tested Components

1. **RTC (DS3231)** - Real-time clock on I2C 0x68
   - Tests: I2C communication to address
   - Expected: ✓ OK (if module is wired)
   - If missing: ✗ MISSING (timestamp will be unavailable until manual set via web panel)

2. **GPS (NEO-6M)** - UART1 on pins 18 (RX) / 17 (TX)
   - Tests: UART initialization
   - Expected: ✓ OK (ready for NMEA polling)
   - If missing: Warning logged but firmware continues

3. **Display (TFT or OLED)** - 2 options:
   - **TFT ILI9341** (320×240 color, SPI)
   - **OLED SSD1306** (128×64 mono, I2C)
   - Tests: Probe both interfaces
   - Expected: Exactly one should be ✓ OK
   - If missing: ✗ CRITICAL - Menu display will fail

4. **CC1101** (Sub-GHz 433MHz) - SPI with CS on GPIO 10
   - Tests: SPI read of version register
   - Expected: ✓ OK (version 0x04 typical)
   - If missing: ✗ MISSING (Sub-GHz tools unavailable)

5. **NRF24L01+** (2.4GHz) - SPI with CS on GPIO 14
   - Tests: SPI read of CONFIG register
   - Expected: ✓ OK (config register non-zero)
   - If missing: ✗ MISSING (2.4GHz/NRF tools unavailable)

6. **Battery Monitor** - ADC on GPIO 7
   - Tests: Analog read from voltage divider
   - Expected: ✓ OK (reading 0-100%)
   - If missing: ✗ Possible issue (open circuit or short)

7. **Buzzer** - GPIO 21 (active high)
   - Tests: GPIO initialization
   - Expected: ✓ OK (always available)

8. **Buttons** (4-way menu) - GPIOs with pullup
   - Tests: Read all 4 button pins (should be high when unpressed)
   - Expected: ✓ OK (all 4 buttons initialized)
   - Warning: If already pressed at boot

9. **Storage (LittleFS)** - Formatted on first boot
   - Tests: Filesystem mount and space check
   - Expected: ✓ OK (n KB used of m KB)
   - If missing: ✗ CRITICAL - Logs and config will fail

10. **PSRAM (Octal)** - External 8MB RAM
    - Tests: Size query and free memory check
    - Expected: ✓ OK (8 MB available)
    - If missing: ✗ CRITICAL - Display and large buffers will fail

### Diagnostic Output

The diagnostics output appears in two places:

1. **Serial Console** (115200 baud):
   ```
   ========== SYSTEM DIAGNOSTICS START ==========
   [✓] RTC (DS3231): OK
   [✓] GPS (NEO-6M): OK (UART1 ready for polling)
   [✓] Display (TFT ILI9341): OK (320x240 color)
   [✗] CC1101 (Sub-GHz 433MHz): MISSING (SPI version read failed)
   [✓] NRF24L01+ (2.4GHz): OK (CONFIG 0x0E)
   [✓] Battery Monitor (ADC): OK (reading 87%)
   [✓] Buzzer (GPIO 21): OK (GPIO initialized)
   [✓] Buttons (4-way menu): OK (all 4 buttons initialized)
   [✓] Storage (LittleFS): OK (2456 KB used of 2048 KB)
   [✓] PSRAM (Octal external RAM): OK (8 MB, 8120 KB free)
   ========== SYSTEM DIAGNOSTICS END ==========
   ```

2. **TFT Screen** (if present):
   - Displays hardware status for 2 seconds before boot completes
   - Shows ✓ for detected, ✗ for missing

### Recovery Strategy

If critical components are missing:

| Component | Missing | Behavior |
|-----------|---------|----------|
| Display | ✗ | Firmware halts at boot (can't show menu) |
| Storage | ✗ | Logs to RAM only, no persistent config |
| PSRAM | ✗ | Display framebuffer fails, no large buffers |
| Buttons | ✗ | No menu navigation (web panel only) |
| RTC | ✗ | No accurate timestamps (still logs with placeholder) |
| GPS | ✗ | Wardriving disabled; other tools work |
| CC1101 | ✗ | Sub-GHz tools disabled; others work |
| NRF24 | ✗ | NRF24-specific tools disabled; others work |
| Battery | ✗ | Battery monitor shows 0%; firmware continues |

---

## Code Quality & Robustness

### Compilation

- **Compiler:** PlatformIO (xtensa-esp32-elf-g++ 5.2.0)
- **Warnings:** None active (all warnings treated as errors are clean)
- **Errors:** 0
- **Build time:** ~90 seconds (cached dependencies)

### Module Initialization

Each module's `begin()` function includes:
- Hardware availability check
- Graceful fallback if hardware missing
- Logged error messages instead of crashes

Examples:
- `Display::begin()` - Auto-probes TFT then OLED; returns gracefully if neither found
- `RtcClock::begin()` - Returns `false` if I2C device not responding
- `SubGhz::begin()` - Skips SPI init if CC1101 not detected
- `Nrf24Tools::begin()` - Skips SPI init if NRF24 not detected

### Memory Management

- **PSRAM Allocation**: Display sprites use PSRAM (8MB) when available
- **Stack Safety**: Large buffers (WiFi frames, scan results) allocated on heap/PSRAM
- **No Blocking Delays**: Critical paths (WiFi/BLE scanning) use cooperative yields
- **Watchdog**: Enabled (default 26 seconds) — long operations must feed watchdog

### Bus Protection (SPI)

The shared SPI bus (TFT + CC1101 + NRF24) is accessed sequentially:
- Only one device selected (CS low) at a time
- Transactions are atomic (SPI.beginTransaction / endTransaction)
- No concurrent access from different tasks
- Each device has dedicated CS pin to prevent conflicts

---

## Hardware Validation Checklist

Use this checklist to validate the device after flashing the firmware.

### Pre-Power Checklist

Before connecting power:
- [ ] Verify ESP32-S3 module is seated correctly on carrier board
- [ ] Check all solder joints are clean (no bridges)
- [ ] Verify no loose components or wires
- [ ] Ensure power switch is OFF before connecting battery

### Initial Boot (Power On)

1. **Power and Serial Console**
   - [ ] Connect USB serial adapter (CP2102 or compatible) to UART0 pins
   - [ ] Power on device (or plug into USB if powered from serial adapter)
   - [ ] Open serial monitor (115200 baud)
   - [ ] Watch for diagnostic output starting with `========== SYSTEM DIAGNOSTICS START ==========`
   - [ ] Note which components show ✓ OK vs ✗ MISSING

2. **Display Check**
   - [ ] Watch TFT or OLED turn on (screen should initialize)
   - [ ] Confirm boot screen appears with version number and AP SSID
   - [ ] Verify text is readable (no garbage or flicker)
   - [ ] All 4 buttons should be responsive

3. **Diagnostic Review**
   - [ ] Check serial console for `SYSTEM DIAGNOSTICS` output
   - [ ] Verify expected components show ✓ (based on your wiring)
   - [ ] Note any ✗ MISSING (expected if hardware not wired)

---

### Individual Peripheral Validation

#### 1. RTC (DS3231)

**Wiring:**
- SDA → GPIO 8
- SCL → GPIO 9
- VCC → 3.3V
- GND → GND

**Test:**
- [ ] Diagnostic shows ✓ OK
- [ ] Connect to web panel (http://<AP_IP>)
- [ ] Navigate to Settings → Time
- [ ] Verify timestamp updates every second
- [ ] Set custom time and confirm it persists after reboot

**Expected Diagnostic:**
```
[✓] RTC (DS3231): OK
```

---

#### 2. GPS (NEO-6M)

**Wiring:**
- RX → GPIO 18 (ESP32 RX, GPS TX)
- TX → GPIO 17 (ESP32 TX, GPS RX)
- VCC → 3.3V
- GND → GND

**Test:**
- [ ] Diagnostic shows ✓ OK (UART1 ready for polling)
- [ ] Point antenna outdoors or near window
- [ ] Open menu → Wardriving
- [ ] Observe GPS position updating in log
- [ ] Confirm latitude/longitude changing
- [ ] Check log file `/logs/wardrive.csv` for GPS data

**Expected Diagnostic:**
```
[✓] GPS (NEO-6M): OK (UART1 ready for polling)
```

---

#### 3. TFT Display (ILI9341, 320×240)

**Wiring:**
- CS → GPIO 5 (via TFT_CS in platformio.ini)
- DC → GPIO 6 (via TFT_DC in platformio.ini)
- RST → GPIO 7 (via TFT_RST in platformio.ini)
- BL → GPIO 46 (via TFT_BL in platformio.ini, backlight)
- SCK → GPIO 12 (shared SPI)
- MOSI → GPIO 11 (shared SPI)
- MISO → GPIO 13 (shared SPI)
- VCC → 3.3V
- GND → GND

**Test:**
- [ ] Diagnostic shows ✓ OK (320x240 color)
- [ ] Boot screen visible with text
- [ ] Menu navigates smoothly with 4 buttons
- [ ] Scan results display with graphs/histograms
- [ ] Colors are vibrant (no fading or noise)
- [ ] Backlight turns on/off when toggled via menu

**Expected Diagnostic:**
```
[✓] Display (TFT ILI9341): OK (320x240 color)
```

---

#### 4. OLED Display (SSD1306, 128×64, fallback only)

**Wiring:**
- SDA → GPIO 8 (shared I2C with RTC)
- SCL → GPIO 9 (shared I2C with RTC)
- VCC → 3.3V
- GND → GND

**Test (if TFT not installed):**
- [ ] Diagnostic shows ✓ OK (128x64 monochrome)
- [ ] Menu visible on small screen (text wraps)
- [ ] Basic functionality works (navigation, scans)

**Note:** OLED is auto-detected if TFT is not present. If both are connected, TFT has priority.

**Expected Diagnostic:**
```
[✓] Display (OLED SSD1306): OK (128x64 monochrome)
```

---

#### 5. CC1101 (Sub-GHz 433 MHz)

**Wiring:**
- SCK → GPIO 12 (shared SPI)
- MOSI → GPIO 11 (shared SPI)
- MISO → GPIO 13 (shared SPI)
- CS → GPIO 10 (dedicated)
- GDO0 → GPIO 4
- GDO2 → GPIO 40
- VCC → 3.3V
- GND → GND
- Antenna: 433 MHz dipole (~17.3 cm)

**Test:**
- [ ] Diagnostic shows ✓ OK (version 0x04)
- [ ] Open menu → RF Tools → Spectrum Analyzer
- [ ] Point antenna toward 433 MHz source (e.g., key fob, weather station)
- [ ] Observe frequency peaks in log output
- [ ] Sub-GHz replay tools should be available in menu

**Expected Diagnostic:**
```
[✓] CC1101 (Sub-GHz 433MHz): OK (version 0x04)
```

---

#### 6. NRF24L01+ (2.4 GHz)

**Wiring:**
- SCK → GPIO 12 (shared SPI)
- MOSI → GPIO 11 (shared SPI)
- MISO → GPIO 13 (shared SPI)
- CS → GPIO 14 (dedicated)
- CE → GPIO 15 (dedicated)
- IRQ → GPIO 41 (dedicated)
- VCC → 3.3V (with 10µF bypass cap recommended)
- GND → GND
- Antenna: 2.4 GHz dipole or chip antenna (built-in on many modules)

**Test:**
- [ ] Diagnostic shows ✓ OK (CONFIG 0x0E)
- [ ] Open menu → RF Tools → NRF24 Scanner
- [ ] Observe device addresses in log
- [ ] NRF24 replay/injection tools available in menu

**Expected Diagnostic:**
```
[✓] NRF24L01+ (2.4GHz): OK (CONFIG 0x0E)
```

---

#### 7. Battery Monitor (ADC)

**Wiring:**
- ADC Input (via resistor divider) → GPIO 7
- Typical: 100k + 47k resistor divider on 4.2V max (Li-ion battery)
- Adjusted for your actual supply voltage

**Test:**
- [ ] Diagnostic shows ✓ OK (reading N%)
- [ ] Display shows battery percentage in status bar
- [ ] Percentage changes realistically with voltage (measure with multimeter if available)
- [ ] Low battery warning appears below 20%

**Expected Diagnostic:**
```
[✓] Battery Monitor (ADC): OK (reading 85%)
```

---

#### 8. Buzzer (GPIO 21)

**Wiring:**
- Signal → GPIO 21 (via transistor driver for power delivery)
- GND → GND

**Test:**
- [ ] Diagnostic shows ✓ OK
- [ ] Beep/buzz when menu item selected
- [ ] Beep when WiFi scan completes
- [ ] (Optional) Adjust buzzer volume in settings

**Expected Diagnostic:**
```
[✓] Buzzer (GPIO 21): OK (GPIO initialized)
```

---

#### 9. Buttons (4-Way Menu)

**Wiring (all to GND via tactile switches):**
- UP → GPIO 1
- DOWN → GPIO 2
- SELECT → GPIO 6
- BACK → GPIO 42

**Test:**
- [ ] Diagnostic shows ✓ OK (all 4 buttons initialized)
- [ ] Press each button and observe menu navigation
- [ ] UP/DOWN scroll list items
- [ ] SELECT confirms choice
- [ ] BACK returns to previous menu
- [ ] No ghost inputs

**Expected Diagnostic:**
```
[✓] Buttons (4-way menu): OK (all 4 buttons initialized)
```

---

#### 10. Storage (LittleFS)

**Wiring:** None (internal flash)

**Test:**
- [ ] Diagnostic shows ✓ OK (n KB used, m KB total)
- [ ] Open menu → Logs
- [ ] Run a WiFi scan → logs to `/logs/wardrive.csv`
- [ ] Use web panel to download logs
- [ ] Confirm logs persist after reboot

**Expected Diagnostic:**
```
[✓] Storage (LittleFS): OK (1024 KB used of 2048 KB)
```

---

#### 11. PSRAM (8 MB Octal)

**Wiring:** None (on-module Octal PSRAM)

**Test:**
- [ ] Diagnostic shows ✓ OK (8 MB, n KB free)
- [ ] Display renders smoothly (uses PSRAM for framebuffer)
- [ ] Large scans complete without crashes
- [ ] No memory errors in serial output

**Expected Diagnostic:**
```
[✓] PSRAM (Octal external RAM): OK (8 MB, 8144 KB free)
```

---

### System-Wide Validation

Once all individual components pass:

1. **Boot Sequence**
   - [ ] Device powers on without hanging
   - [ ] Diagnostics run in <5 seconds
   - [ ] Boot screen shows AP SSID
   - [ ] Menu fully responsive after boot

2. **Menu Navigation**
   - [ ] All menu levels open without lag
   - [ ] Scan results display correctly
   - [ ] Buttons navigate smoothly

3. **WiFi Scanning**
   - [ ] WiFi scan finds nearby networks
   - [ ] RSSI bars display correctly
   - [ ] Histogram/gauge visualizations render
   - [ ] Log saved to `/logs/wardrive.csv`

4. **BLE Scanning**
   - [ ] BLE scan finds nearby devices
   - [ ] Device distribution histogram shows
   - [ ] Strongest signal gauge updates

5. **RF/Spectrum Tools**
   - [ ] CC1101 tools available (if wired)
   - [ ] NRF24 tools available (if wired)
   - [ ] Spectrum analysis displays waterfall

6. **Web Control Panel**
   - [ ] Connect to AP with SSID shown at boot
   - [ ] Web panel loads at http://<AP_IP> (usually 192.168.4.1)
   - [ ] Settings, logs, and commands accessible

7. **Stress Test** (optional)
   - [ ] Run WiFi scan for 1+ minute
   - [ ] Run BLE scan continuously
   - [ ] No crashes or watchdog resets
   - [ ] Check RAM usage (should stay <50%)

---

## Troubleshooting

### "Firmware won't boot / crashes on power-on"

1. Check serial console for diagnostics output
2. Verify power supply voltage (should be 5V or Li-ion battery regulated)
3. Check for loose components or solder bridges
4. If PSRAM/flash not detected, verify chip select lines

### "Display blank or garbage"

1. Verify TFT wiring (CS, DC, RST, BL pins)
2. Check SPI SCK/MOSI/MISO pins are connected
3. Try OLED if TFT unavailable (auto-detected)
4. Check TFT_CS, TFT_DC, TFT_RST, TFT_BL definitions in platformio.ini

### "Menu won't respond to buttons"

1. Check button wiring (all should go to GND)
2. Verify GPIO pins 1, 2, 6, 42 are free
3. Check serial console for button diagnostic
4. Try web panel instead (if WiFi works)

### "WiFi/BLE scans don't find anything"

1. Verify nearby devices are actually on (WiFi on, BLE beaconing)
2. Try scanning from different location
3. Check CC1101/NRF24 wiring if using RF tools
4. Confirm antennas are connected

### "SD card / logs not working"

1. Verify LittleFS is initialized (check diagnostic)
2. Try resetting device to reformat filesystem
3. Check `/logs` directory exists on device

---

## Support & Logging

All system logs are captured on serial console (115200 baud):

```bash
# On Linux/Mac:
screen /dev/ttyUSB0 115200

# On Windows:
putty (set speed to 115200 baud)
```

Key log levels:
- `[BOOT]` - Boot sequence
- `[ERROR]` - Fatal errors
- `[WARNING]` - Recoverable issues
- `[INFO]` - Status messages
- `[DEBUG]` - Detailed diagnostics (if enabled)

---

## Next Steps

Once validation is complete:

1. Configure AP password (change `AP_PASSWORD` in config.h)
2. Set time via web panel if RTC not installed
3. Review `/logs/` for any error messages
4. Enable wardriving / custom features as needed

Enjoy your ESP32-S3 Audit Tool! 🎉
