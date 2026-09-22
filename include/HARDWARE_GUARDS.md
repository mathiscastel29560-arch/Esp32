# Hardware Driver Conflict Guards

This document explains the safety guards added to prevent hardware conflicts on the ESP32-S3 audit tool.

## Overview

Several hardware components share communication buses or GPIO. The drivers include guards to prevent double-initialization and resource conflicts.

## Known Conflicts

### 1. GPS/UART1 Conflict ⚠️

**Affected Components:**
- `GpsModule` (existing) - uses UART1, TinyGPSPlus library
- `GPSDriver` (new) - also uses UART1

**Guard Mechanism:**
```cpp
// GPSDriver checks if Serial1 is already in use
if (Serial1.baudRate() > 0) {
    Serial.println("UART1 already in use (GpsModule active) - skipping GPSDriver");
    return false;
}
```

**Resolution:**
- Use existing `GpsModule` (recommended)
- OR use `GPSDriver` (new, more integrated)
- **NOT both simultaneously**

**Recommended:** Keep using `GpsModule` - it's proven and integrated into the system

---

### 2. CC1101/RadioLib Conflict ⚠️

**Affected Components:**
- `SubGhz` module - uses RadioLib's CC1101 class
- `CC1101Driver` (new) - custom SPI-based CC1101 driver
- `TPMS Spoofer` - now uses `CC1101Driver`
- `Mavic Jammer` - uses `NRF24Driver` (not affected)

**Guard Mechanism:**
```cpp
// CC1101Driver warns at initialization
Serial.println("[CC1101] ⚠️  WARNING: Ensure SubGhz module is NOT active");
```

**Resolution:**
- **Option A:** Keep using `SubGhz` with RadioLib (proven)
- **Option B:** Use new `CC1101Driver` with adapted tools (TPMS Spoofer)
- **NOT both simultaneously**

**Recommended:** 
- For **Sub-GHz recording/replay** → Use `SubGhz` (RadioLib)
- For **TPMS spoofing** → Use `CC1101Driver`
- **Important:** Disable one before using the other

---

### 3. SPI Bus Sharing ⚠️

**Affected Components:**
- TFT Display - uses SPI (managed by TFT_eSPI library)
- CC1101 - uses HSPI via `SPIClass(HSPI)`
- NRF24 - uses HSPI via `SPIClass(HSPI)`

**Pin Configuration:**
```
SCK  = GPIO 12
MOSI = GPIO 11
MISO = GPIO 13

CS pins (separate per device):
- TFT:     platformio.ini (handled separately)
- CC1101:  GPIO 10
- NRF24:   GPIO 14
```

**Guard Mechanism:**
```cpp
// NRF24Driver warns about shared SPI bus
Serial.println("[NRF24] ⚠️  WARNING: SPI bus shared with TFT, CC1101");
```

**Resolution:**
- TFT uses its own SPI setup (handled by TFT_eSPI)
- CC1101Driver and NRF24Driver both use HSPI with separate CS pins
- **Should work** if TFT is initialized first (Display::begin() before Hardware::initAll())
- **Watch for:** Timing issues if rapid switching between TFT and radio modules

---

## How to Use Hardware::initAll()

### Initialization Order (Hardware::initAll)

```
1. GPIO (Buttons, Buzzer, IR, Battery)
2. RTC (I2C)
3. GPS (UART1) - Checks if GpsModule active first
4. PN532 (I2C) - Shares I2C with RTC, different addresses
5. CC1101 (SPI) - Warns about SubGhz conflicts
6. NRF24 (SPI) - Warns about bus sharing
```

### Guard Behavior

| Situation | Hardware Behavior | Guard Behavior |
|-----------|---|---|
| GpsModule active, then Hardware::initAll() | UART1 blocked | GPSDriver skipped, warning logged |
| Hardware::initAll(), then SubGhz.begin() | RadioLib tries to use CC1101 | Potential conflict (manual coordination) |
| Rapid TFT + NRF24 switching | SPI bus contention possible | Works if CS pins prevent overlap |

### Safe Scenarios

✅ **These combinations are SAFE:**
1. Display + GPIO + RTC + GPS (existing setup)
2. Display + GPIO + RTC + PN532 + TPMS Spoofer (uses CC1101Driver)
3. Display + GPIO + RTC + GPS + NRF24 tools (Mavic Jammer)

⚠️ **These require CARE:**
1. SubGhz + CC1101Driver (choose one)
2. GpsModule + GPSDriver (choose one)
3. Multiple rapid SPI device switches

---

## Testing Checklist Before Field Use

- [ ] Verify pins with multimeter
- [ ] Test GPIO (buttons, buzzer) - no conflicts
- [ ] Test I2C (RTC + PN532) - separate addresses
- [ ] Test UART1 (GPS) - confirm TinyGPSPlus receiving data
- [ ] Test CC1101 - run TPMS Spoofer in isolation
- [ ] Test NRF24 - run Mavic Jammer in isolation
- [ ] Test Display + one radio module together
- [ ] **Never** mix SubGhz + TPMS Spoofer in same session

---

## When Guards Trigger

### GPS Guard
```
[GPS] ⚠️  UART1 already in use (GpsModule active)
[GPS] Cannot initialize GPSDriver - use existing GpsModule instead
```
**Action:** Use existing GpsModule, don't call GPSDriver::init()

### CC1101 Guard
```
[CC1101] ⚠️  WARNING: Ensure SubGhz module is NOT active
[CC1101] Conflicts possible if SubGhz and CC1101Driver used simultaneously
```
**Action:** If using SubGhz for recording, don't initialize CC1101Driver. If using TPMS Spoofer, disable SubGhz.

### NRF24 Guard
```
[NRF24] ⚠️  WARNING: SPI bus shared with TFT, CC1101
[NRF24] Ensure Display and CC1101 are compatible or not active simultaneously
```
**Action:** Ensure Display::begin() runs before Hardware::initAll(). Don't use CC1101 while NRF24 transmission active.

---

## Emergency Protocol

If hardware hangs or behaves erratically:

1. **Check serial output** for guard messages
2. **Power cycle** the device (remove USB + battery)
3. **Isolate** the problematic module:
   - Comment out driver init in Hardware::initAll()
   - Test module in isolation
4. **Report** exact guard messages seen

---

## Future Improvements

- [ ] Mutex-based SPI bus arbitration
- [ ] Automated conflict detection at runtime
- [ ] Tool pre-flight checks before launch
- [ ] SPI transaction queuing for Display + radio modules
