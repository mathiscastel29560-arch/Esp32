# PN532 RFID Integration Guide

## Hardware
**Hilacini PN532 V3 Kit** (11,99€ Amazon.fr)
- 2x PN532 modules (SPI-capable)
- 2x Mifare S50 tags (1K memory)
- 2x NFC keychains
- Cables + headers

---

## When Hardware Arrives

### 1️⃣ Add Library to platformio.ini

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps =
    ArduinoJson
    WebServer
    adafruit/Adafruit PN532  ← ADD THIS
```

Then: `pio run` to download lib

### 2️⃣ Update rfid.cpp

Replace stub with real PN532 code:

```cpp
#include "rfid.h"
#include "config.h"
#include <LittleFS.h>
#include <SPI.h>
#include <Adafruit_PN532.h>   // ← UNCOMMENT THIS

// Create PN532 instance (SPI mode: CS=GPIO8, RST=GPIO7)
Adafruit_PN532 nfc(RFID_CS, RFID_RST);  // GPIO 8, GPIO 7

namespace RFID {

ScanResult scan() {
    ScanResult result{false, {}, 0, ""};
    uint32_t startTime = millis();
    
    // Initialize PN532 if not done
    static bool initialized = false;
    if (!initialized) {
        nfc.begin();
        uint32_t versiondata = nfc.getFirmwareVersion();
        if (!versiondata) {
            result.error = "PN532 not found on SPI";
            return result;
        }
        nfc.SAMConfig();  // Configure for reading
        initialized = true;
    }
    
    // Wait up to 5 seconds for a tag
    uint32_t timeout = millis() + 5000;
    while (millis() < timeout) {
        uint8_t uid[7];
        uint8_t uidLen;
        
        // Try to read ISO14443A targets (Mifare, etc)
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen)) {
            // Tag found!
            result.found = true;
            result.tag.uidLen = uidLen;
            memcpy(result.tag.uid, uid, uidLen);
            
            // Detect tag type
            if (uidLen == 4) {
                result.tag.type = "Mifare Classic 1K";
                result.tag.capacity = 1024;
                result.tag.sectorCount = 16;
            } else if (uidLen == 7) {
                result.tag.type = "Mifare Classic 4K";
                result.tag.capacity = 4096;
                result.tag.sectorCount = 40;
            } else {
                result.tag.type = "Unknown";
                result.tag.capacity = 0;
            }
            
            result.tag.isWritable = true;
            result.readTimeMs = millis() - startTime;
            
            g_lastScannedTag = result.tag;
            g_lastScanTime = millis();
            return result;
        }
        
        delay(100);
    }
    
    result.readTimeMs = millis() - startTime;
    result.error = "No tag detected within 5 seconds";
    return result;
}

CloneResult clone(const TagData &sourceTag) {
    CloneResult result{false, "", "", 0, ""};
    
    // This is advanced and requires sector-by-sector write
    // Full implementation depends on tag type and authentication keys
    
    // Simplified version:
    // 1. Read all sectors from source (requires auth)
    // 2. Write sectors to target
    // 3. Verify integrity
    
    // For now, return stub
    result.error = "Clone operation requires RFID auth keys (see docs)";
    return result;
}

} // namespace RFID
```

### 3️⃣ Compile & Test

```bash
cd /home/user/Esp32
pio run
```

Expected output in Serial Monitor (115200 baud):
```
PN532 firmware version detected
RFID: Scan ready
```

### 4️⃣ Test via Web UI

Navigate to `http://esp32.local:8080`:
- Click "Scan Tag (5s)"
- Place Mifare tag near reader
- Should see: `UID: 1234ABCD | Type: Mifare Classic 1K | Capacity: 1024 bytes`

---

## Important Notes

### SPI Bus Sharing
- TFT and PN532 share CLK/MOSI/MISO (GPIO 12/11/13)
- Each has separate Chip Select (TFT=GPIO10, RFID=GPIO8)
- ✅ No conflicts at protocol level

### Authentication Keys
- Mifare Classic requires **A/B keys** to read/write sectors
- Default key is usually: `FF FF FF FF FF FF`
- Clone requires full sector access
- See Adafruit_PN532 examples for auth

### Limitations
- This implementation is **passive scan only** (reads existing tags)
- Full clone requires implementing Mifare MFOC/nfcutils
- Consider using Mifare Ultralight for easier writes (no auth needed)

---

## Resources
- [Adafruit PN532 Library Docs](https://github.com/adafruit/Adafruit-PN532)
- [PN532 Datasheet](https://www.nxp.com/docs/en/user-manual/UM0701.pdf)
- [Mifare Classic Protocol](https://www.nxp.com/docs/en/data-sheet/MF1S70YYX.pdf)

---

## Timeline
1. 📦 Order kit (est. 3-7 days delivery)
2. 🔧 Receive kit, validate SPI connections
3. 💻 Update `rfid.cpp` with real driver (1-2 hours)
4. ✅ Test scan + clone (30 mins)

Ready? Just send me a message when hardware arrives! 🚀
