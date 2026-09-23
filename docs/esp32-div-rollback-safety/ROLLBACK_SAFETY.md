# OTA Rollback Safety for ESP32-DIV

## Overview

The `rollback_safety.cpp` file has been added to enable OTA (Over-The-Air) firmware update dual-boot rollback protection on the ESP32-DIV.

**Note:** This file is NOT part of the upstream cifertech/esp32-div project. It has been added locally for enhanced safety during firmware updates.

## What It Does

The `verifyRollbackLater()` function is called by the ESP32 bootloader to determine if the current firmware is valid. By returning `true`, it indicates the firmware has passed validation and should not be rolled back to the previous version.

- **Returning true**: Current firmware is valid, keep it as primary (ota_0)
- **Returning false**: Current firmware is invalid, fall back to ota_1 on next boot

## Compilation & Flashing

### Prerequisites
- Arduino IDE v1.8.19+ or Arduino CLI
- ESP32 board support installed in Arduino
- USB cable connected to ESP32-DIV

### Method 1: Arduino IDE
1. Open `ESP32-DIV.ino` in Arduino IDE
2. Select Board: `ESP32S3 Dev Module`
3. Verify the sketch compiles: **Sketch** → **Verify/Compile**
4. Flash to OTA slot 1 (ota_1) at address 0x310000:
   - **Tools** → **Flash Size** → 16MB
   - **Tools** → **Partition Scheme** → Choose a dual-boot scheme
   - Use `esptool.py` to flash: 
   ```bash
   esptool.py --port /dev/ttyUSB0 --chip esp32s3 \
     write_flash 0x310000 ESP32-DIV.ino.esp32s3.bin
   ```

### Method 2: Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32s3:FlashSize=16M ESP32-DIV

# Flash to ota_1 slot (0x310000)
esptool.py --port /dev/ttyUSB0 --chip esp32s3 \
  write_flash 0x310000 build/esp32:esp32:esp32s3/ESP32-DIV.ino.esp32s3.bin
```

### Method 3: esptool.py Direct
```bash
# Get the binary from Arduino IDE build directory and flash
esptool.py --port /dev/ttyUSB0 --chip esp32s3 \
  write_flash -z \
  0x0    bootloader.bin \
  0x8000 partitions.bin \
  0x10000 ESP32-DIV.ino.bin
```

## File Details

- **File**: `rollback_safety.cpp`
- **Location**: `ESP32-DIV/` (same directory as ESP32-DIV.ino)
- **Size**: ~200 bytes (negligible impact)
- **Dependencies**: None (standard C++)

## Verification

After flashing, verify rollback protection is active:
1. Power cycle the device
2. Check serial output for boot messages
3. Connect via web panel or serial monitor
4. Observe status showing dual-boot is active

## Troubleshooting

**Compilation fails with undefined reference to `verifyRollbackLater`**
- Ensure `rollback_safety.cpp` is in the same directory as `ESP32-DIV.ino`
- Arduino IDE should auto-compile all `.cpp` files in the sketch directory

**Flash address conflicts**
- Verify partition scheme supports dual-boot (min 2 app slots)
- Use `parttool.py` to inspect partition table:
  ```bash
  parttool.py --port /dev/ttyUSB0 list_partitions
  ```

**Bootloop after flashing to ota_1**
- This is expected behavior - the bootloader is validating the new firmware
- If valid, device boots normally; if invalid, it rolls back to ota_0
- Monitor serial output at 115200 baud during boot

## Security Notes

1. The rollback function ALWAYS returns `true`, so valid firmware is never rolled back
2. To force rollback testing, modify the function to return `false` temporarily
3. The first boot after flashing may be slower due to validation
4. Ensure AP_PASSWORD is changed before field deployment (see main Audit Logger documentation)

## Reverting

To remove rollback safety:
1. Delete `rollback_safety.cpp`
2. Recompile and flash to ota_0 (primary slot, address 0x10000)
3. Device operates without dual-boot protection

## References

- [ESP32 OTA Dual-Boot Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/ota.html)
- [esptool.py Documentation](https://github.com/espressif/esptool)
- [ESP32 Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/partition-tables.html)

---

**Origin**: Added by Claude for Audit Logger ESP32 project  
**Date**: 2026-09-22  
**Compatibility**: ESP32-S3 with dual-boot partition scheme
