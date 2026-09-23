# Testing & Validation Guide

## Quick Start

### Run All Tests
```bash
# Compile test suite
cd /home/user/Esp32
pio test -e esp32-s3-audit

# Or manually compile tests
g++ -o test test/test_framework.h test/test_wpa2_utils.cpp
./test
```

### Run CI/CD Pipeline Locally
```bash
# Simulate GitHub Actions
bash -c '$(grep -A 100 "build:" .github/workflows/build-and-test.yml)'
```

## Test Coverage

### 1. WPA2 Module Tests ✅
```
PBKDF2 Implementation
├─ Password/SSID length validation
├─ Output size verification
└─ Test vector comparison

MAC Address Parsing
├─ Valid format: AA:BB:CC:DD:EE:FF
├─ Invalid format rejection
└─ Byte-wise validation

WPS PIN Checksum
├─ Luhn algorithm implementation
└─ Invalid PIN detection
```

### 2. Network Validation Tests ✅
```
SSID Validation
├─ Length bounds (0-32 bytes)
├─ Hidden network detection
└─ Special character handling

WiFi Channel Validation
├─ 2.4GHz range: 1-13
├─ 5GHz range: 36-165
└─ Invalid channel rejection

BSSID Format
├─ MAC address parsing
├─ Broadcast address rejection
└─ Zero address rejection
```

### 3. Hardware Driver Tests ❌ (Requires Hardware)
```
CC1101 (433MHz)
├─ Initialization test
├─ Frequency setting (±100kHz)
├─ Transmit/Receive cycle
└─ Power levels (0-10 dbm)

NRF24 (2.4GHz)
├─ SPI communication
├─ Channel switching
├─ CRC validation
└─ Payload size limits

PN532 (NFC/RFID)
├─ I2C communication
├─ Firmware version query
├─ Card detection
└─ ISO 14443A handshake

GPS NEO-6M
├─ UART reception
├─ NMEA parsing
├─ Fix validation
└─ Satellite count

RTC DS3231
├─ Time read/write
├─ Temperature sensor
├─ Battery backup flag
└─ Alarm settings
```

## Validation Methods

### Static Analysis
```bash
# Check for unsafe functions
grep -r "strcpy\|sprintf\|gets" src/ --include="*.cpp"

# Check for hardcoded credentials
grep -r "password.*=" src/ --include="*.cpp" | grep -v "//"

# Verify error handling
grep -r "return result" src/ --include="*.cpp" | wc -l
```

### Compilation Warnings
```bash
# Build with extra warnings
CXXFLAGS="-Wall -Wextra -Wpedantic" pio run -e esp32-s3-audit

# Check for:
# - Unused variables
# - Missing return statements
# - Type conversions
```

### Runtime Testing

#### Manual Module Test
```cpp
// From Hardware Test mode - validates driver in isolation
Press: 8 (Hardware Test) → Select device → Run test
Results:
✓ GPIO: Button polling, buzzer, battery ADC
✓ RTC: Time display, temperature
✓ GPS: 30-second fix attempt
✓ PN532: Firmware query, card scan
✓ CC1101: 10-second listen on 433.92 MHz
✓ NRF24: 5-channel sweep on 2.4GHz
```

#### Attack Tool Validation
```
1. WiFi KRACK Simulation
   - Verifies deauth frame format
   - Counts transmitted frames
   - Checks frame control bytes
   
2. WPA2 Dictionary Attack
   - Test with known password
   - Verify PBKDF2 output
   - Check MIC calculation
   
3. BLE Audio Hijacking
   - Scan for targets
   - Enumerate services
   - Verify UUID parsing
```

## Continuous Integration

### GitHub Actions Workflow
```yaml
On every push:
1. Build firmware (30s)
2. Check firmware size
3. Security analysis
4. Code quality check
5. Documentation verification

Artifacts:
- firmware.bin (2.1 MB)
- Build logs
- Security report
```

### Local CI Simulation
```bash
# Run all CI checks locally
./scripts/run-ci.sh

# Or manually:
pio run -e esp32-s3-audit           # Build
grep -r "warning:" .pio/build/      # Check warnings
grep -r "password=" src/ --include="*.cpp"  # Security check
```

## Performance Benchmarks

### Expected Performance
```
Operation                    Time      Memory
─────────────────────────────────────────────────
WiFi KRACK simulation        5-10s     ~15KB
WPA2 dictionary (10 attempts) 3-5s     ~20KB
BLE scan (30s)              30s        ~25KB
SubGhz frequency sweep      15-30s     ~10KB
NFC tag read               1-2s        ~5KB
```

### Memory Usage
```
Component          Heap Used   Flash Used
──────────────────────────────────────────
WiFi/BLE stack     ~80KB       ~400KB
RadioLib           ~10KB       ~200KB
MFRC522            ~5KB        ~30KB
Crypto (mbedtls)   ~8KB        ~100KB
Application        ~20KB       ~150KB
────────────────────────────────────────
Total              ~123KB      ~880KB (28%)
Available          ~205KB      ~2,265KB (72%)
```

## Security Testing

### Input Validation Checklist
```
[ ] MAC address format "AA:BB:CC:DD:EE:FF"
[ ] SSID length 0-32 bytes
[ ] WiFi channel 1-13 or 36-165
[ ] Password 8-63 characters
[ ] Duration > 0 and < 1 hour
[ ] Buffer overflow on 256-byte arrays
[ ] Integer wraparound in timeout calc
```

### Cryptographic Validation
```
[ ] PBKDF2 uses 4096 iterations
[ ] HMAC uses SHA1 (not MD5)
[ ] Key derivation produces 32 bytes
[ ] MIC calculation correct length
[ ] No hardcoded keys in code
```

### Memory Safety
```
[ ] All strcpy() replaced with memcpy()
[ ] All sprintf() use fixed format strings
[ ] All pointers checked for NULL
[ ] All buffer writes have bounds checks
[ ] No use of gets() or similar
```

## Troubleshooting Tests

### Build Fails
```bash
# Clean build
pio run -e esp32-s3-audit -t clean
pio run -e esp32-s3-audit

# Check library versions
pio lib list
pio platform show espressif32

# Verify Python version
python3 --version  # Should be 3.8+
```

### Tests Report Errors
```bash
# Run with verbose output
pio run -e esp32-s3-audit -v

# Check specific compiler warnings
grep "error:" build.log

# Verify library compatibility
grep -r "include <" src/ | head -20
```

### Hardware Tests Fail
```
See: HARDWARE_GUARDS.md for conflict detection
Or: Hardware Test mode (menu item 7) for isolation test
```

## Future Test Coverage

- [ ] Unit tests for cryptographic functions
- [ ] Integration tests for multi-module interactions
- [ ] Stress tests (long-running operations)
- [ ] Memory leak detection
- [ ] Fuzzing of input parsers
- [ ] Hardware emulation tests

## References

- `SECURITY.md` - Security review and recommendations
- `CLAUDE.md` - Architecture and hardware documentation
- `.github/workflows/build-and-test.yml` - CI/CD configuration
