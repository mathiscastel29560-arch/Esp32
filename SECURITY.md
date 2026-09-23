# Security Considerations - ESP32-S3 Offensive Security Platform

**⚠️ WARNING: This platform implements real attack techniques. Use only in authorized testing environments.**

## Legal & Ethical

- **Authorized Use Only**: All tools require explicit authorization on networks/devices tested
- **Scope**: Designed for penetration testing, security research, CTF competitions
- **Liability**: Users are responsible for all actions taken with this platform
- **Compliance**: Verify compliance with local regulations before deployment

## Cryptographic Implementation Review

### WPA2 Key Derivation (PBKDF2-SHA1)
- **Location**: `src/wpa2_handshake_cracker_impl.cpp`, `src/wps_bruteforce.cpp`
- **Implementation**: Manual PBKDF2 using `mbedtls_md_hmac()`
- **Security**: Standard 4096 iterations as per IEEE 802.11i specification
- **Risk Level**: ✅ LOW - Uses proven mbedtls HMAC functions

### WiFi Frame Injection
- **Location**: `src/wifi_krack.cpp`, `src/wifi_association_hijacker_impl.cpp`
- **API**: Raw IEEE 802.11 frame injection via `esp_wifi_80211_tx()`
- **Validation**: BSSID format checked, frame structure validated
- **Risk Level**: ✅ LOW - Hardware API used, no custom implementations

### BLE Attacks (NimBLE 1.4.3)
- **Location**: `src/ble_*.cpp` modules
- **API**: NimBLE-Arduino official library, no custom Bluetooth stack
- **Validation**: BLE address format validated, service/characteristic checks
- **Risk Level**: ✅ LOW - Uses well-tested library

### RF/SubGhz Operations (RadioLib 6.6.0)
- **Location**: `src/subghz_*.cpp` modules
- **API**: RadioLib official library for CC1101 transceiver
- **Validation**: Frequency bounds checked, payload size limits enforced
- **Risk Level**: ✅ LOW - Industry-standard library

## Memory Safety

### Potential Issues Fixed
1. ✅ Buffer overflows - All arrays have size validation
2. ✅ Pointer dereferencing - Null checks added on service/characteristic access
3. ✅ Integer overflow - millis() wraparound handled in timeout comparisons
4. ✅ Stack overflow - Large buffers (256B max) allocated, no recursion

### Remaining Considerations
- Stack usage: ~4KB available per task, monitor with tools
- Heap fragmentation: Long-running operations may allocate repeatedly
- PSRAM usage: Not utilized in current implementation

## Input Validation

### MAC Address Parsing
```cpp
// Validated in: wpa2_handshake_cracker_impl.cpp, wifi_krack.cpp, etc.
// Format: "AA:BB:CC:DD:EE:FF" (17 chars)
// Validated with: sscanf() + bounds check
```

### BSSID/SSID Validation
```cpp
// Length checks: SSID 0-32 chars, null checks for hidden networks
// Character validation: No special parsing, raw bytes preserved
```

### WiFi Channel Validation
```cpp
// Bounds: 1-13 (2.4GHz), 36-165 (5GHz)
// Validated in: wifi_krack.cpp, subghz_*.cpp
```

### Timeout Handling
```cpp
// Safe comparison: (int32_t)(millis() - deadline) < 0
// Handles 49-day wraparound correctly
```

## Code Review Findings

### ✅ Secure Practices
- No format string vulnerabilities (all Serial.printf use fixed strings)
- No SQL injection (no database layer)
- No path traversal (LittleFS paths validated)
- Proper use of mbedtls for cryptography

### ⚠️ Areas to Monitor
1. **Error Handling**: Some modules use `Serial.println()` instead of structured error codes
2. **Logging**: No audit trail for attack actions - recommend adding logging
3. **Configuration**: Sensitive parameters (AP names, passwords) not encrypted
4. **Memory Leaks**: Large allocations in loops should verify cleanup

## Hardware-Specific Considerations

### SPI Bus Conflicts
- TFT_eSPI, CC1101, NRF24 share SPI pins
- Separate CS pins prevent conflicts
- **Risk**: Simultaneous access could corrupt data
- **Mitigation**: Initialize Display first, test in isolation mode

### UART1/GPS Conflict
- GPS module and GpsModule library both use UART1
- **Risk**: Duplicate initialization causes conflicts
- **Mitigation**: Use one or the other, check Serial1.baudRate()

### I2C Address Conflicts
- RTC DS3231 at 0x68, PN532 at 0x24
- **Risk**: Different addresses, no conflict
- **Status**: ✅ Safe

## Attack Tool Validation

### WiFi Tools
- ✅ KRACK simulation uses real IEEE 802.11 frame structure
- ✅ WPA2 cracking uses standard PBKDF2 + HMAC
- ✅ Hidden network reveal uses passive scanning + beacon analysis

### BLE Tools
- ✅ Audio hijacking uses NimBLE service enumeration
- ✅ Credential harvesting parses characteristic values
- ✅ MITM relay uses established connections

### RF Tools
- ✅ SubGhz fuzzing generates valid payloads
- ✅ Frequency scanner sweeps 433-920 MHz
- ✅ Jammer suite uses OOK modulation

### NFC/RFID Tools
- ✅ MIFARE classic emulation uses MFRC522 library
- ✅ NFC relay uses I2C PN532 protocol
- ✅ Card detection uses proper ISO 14443A handshake

## Recommendations

### Before Deployment
1. **Code Review**: Independent security audit recommended
2. **Hardware Test**: Validate all drivers on actual ESP32-S3
3. **Network Testing**: Test on isolated lab networks only
4. **Logging Setup**: Implement audit logging for all attacks
5. **Documentation**: Document tool capabilities and limitations

### For Production Use
1. **Configuration Encryption**: Encrypt stored AP credentials
2. **Audit Trail**: Log all tool invocations with timestamps
3. **Rate Limiting**: Implement delays between operations
4. **Status Monitoring**: Add heartbeat/health checks
5. **Secure Boot**: Enable ESP32 secure boot if available

### For Research
1. **Fuzzing**: Use LLVM sanitizers to detect memory issues
2. **Code Coverage**: Measure coverage of attack paths
3. **Performance**: Profile memory/CPU under load
4. **Reliability**: Test long-running operations (>1 hour)

## Incident Response

**If platform is compromised:**
1. Power off immediately (disconnect USB)
2. Backup flash contents before reimaging
3. Review LittleFS logs for attack history
4. Check radio for unauthorized transmissions

**If attack exceeds scope:**
1. Abort operation (BACK button or power off)
2. Document what occurred
3. Reset device in Hardware Test mode
4. Verify recovery before next test

## Version Info

- **Platform**: ESP32-S3-DevKitC-1 (N8 variant)
- **Build**: 2079145 bytes / 3145728 bytes Flash
- **Libraries**: NimBLE 1.4.3, RadioLib 6.6.0, RF24, MFRC522
- **Last Reviewed**: 2025-09-23

---

**Questions?** File issue on GitHub or review CLAUDE.md for architecture details.
