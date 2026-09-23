# Future Improvements Roadmap

## Priority 1: Security Fixes 🔴

### 1.1 Fix strcat() Usage in BLE MITM Relay
**File**: `src/ble_mitm_relay.cpp` (lines with `strcat`)
**Issue**: `strcat()` is unsafe - can overflow
**Fix**: Replace with safe string building
```cpp
// BEFORE
strcat(hexData, hex);  // UNSAFE

// AFTER
if (offset + 2 < sizeof(hexData)) {
    snprintf(hexData + offset, sizeof(hexData) - offset, "%s", hex);
    offset += strlen(hex);
}
```
**Impact**: Security fix, no runtime cost

### 1.2 Secure Test Credentials
**File**: `src/wps_bruteforce.cpp`, `src/mqtt_hijacker_impl.cpp`
**Issue**: Hardcoded test credentials in production code
**Fix**: Move to separate test configuration
```cpp
#ifdef TEST_MODE
const char* TEST_PASSWORDS[] = {"password", "12345", ...};
#endif
```
**Impact**: Security improvement for deployment

### 1.3 Input Sanitization Review
**Files**: Attack modules with user input
**Issue**: Limited input validation on user parameters
**Fix**: Add comprehensive InputValidator usage
**Impact**: Prevent injection attacks

---

## Priority 2: Feature Completeness 🟡

### 2.1 Configuration Persistence
**Missing**: System to save/load tool settings
**Suggested**:
- LittleFS configuration files
- JSON format for readability
- Encryption for sensitive settings

**Example**:
```json
{
  "wifi_scan": {
    "timeout_ms": 30000,
    "channel_start": 1,
    "channel_end": 13
  },
  "ble_attacks": {
    "duration_ms": 60000,
    "scan_window": 1000
  }
}
```

### 2.2 Audit Logging System
**Missing**: History of all attack actions
**Needed For**:
- Accountability
- Troubleshooting
- Post-operation analysis

**Implementation**:
- LittleFS log files (one per operation)
- CSV format for easy analysis
- Timestamp, module, parameters, result

### 2.3 Battery & Power Management
**Missing**: Low battery warnings
**Suggested**:
- ADC monitoring threshold
- Emergency shutdown at critical level
- Power profile optimization

---

## Priority 3: Testing Expansion 🟠

### 3.1 Unit Tests for Cryptography
**Current**: Framework exists, few tests
**Needed**:
- PBKDF2 test vectors (RFC 6070)
- HMAC-SHA1 validation
- WPA2 MIC calculation

### 3.2 Hardware Driver Tests
**Current**: Framework exists, no hardware access
**Needed** (when hardware available):
- CC1101 transmission/reception
- NRF24 channel hopping
- PN532 card detection
- GPS satellite fix
- RTC time accuracy

### 3.3 Integration Tests
**Missing**: Multi-module interactions
**Examples**:
- WiFi scan + channel parsing
- BLE enumerate + credential harvest
- RF detection + frequency analysis

---

## Priority 4: Documentation & UX 💙

### 4.1 Tool-Specific Documentation
**Missing**: Usage guide for each attack tool
**Should Include**:
- What it does (real attack description)
- Parameters explained
- Expected output examples
- Limitations
- Legal/ethical notes

### 4.2 Video/Screenshot Guide
**Missing**: Visual how-to documentation
**Needed**:
- GIF of menu navigation
- Screenshots of each tool
- Output examples
- Result interpretation

### 4.3 API Documentation
**Missing**: For developers
**Should Include**:
- Function signatures
- Input/output formats
- Error codes
- Usage examples

---

## Priority 5: Performance & Optimization 💚

### 5.1 Memory Optimization
**Current**: 26.3% RAM, 66.1% Flash
**Opportunities**:
- Reduce buffer sizes where safe
- String pooling for constants
- PSRAM usage for large buffers
- Code compression

### 5.2 Speed Optimization
**Areas**:
- WiFi scanning (multi-channel parallel?)
- BLE service enumeration (faster discovery?)
- Cryptographic operations (hardware acceleration?)

### 5.3 Energy Efficiency
**Measure**:
- Current consumption per tool
- Optimize based on measurements
- Power modes when idle

---

## Priority 6: Compatibility & Portability 🟣

### 6.1 Multi-ESP32 Support
**Current**: ESP32-S3 only
**Target**: ESP32, ESP32-C3, ESP32-H2
**Changes**:
- Conditional compilation flags
- Pin mapping per variant
- Feature parity matrix

### 6.2 Cross-Platform Testing
**Needed**:
- Windows compilation
- macOS compilation
- Linux validation

---

## Completed Items ✅

- [x] **Compilation**: All 60+ modules compile
- [x] **Security Review**: SECURITY.md completed
- [x] **Testing Framework**: Unit test infrastructure
- [x] **CI/CD Pipeline**: GitHub Actions workflow
- [x] **Error Handling**: Centralized ErrorCode system
- [x] **Input Validation**: InputValidator framework
- [x] **Documentation**: CLAUDE.md, TESTING.md, SECURITY.md
- [x] **Build Automation**: Makefile and scripts
- [x] **Code Quality**: Local CI simulation

---

## Implementation Guide

### For Contributors

1. **Pick an item** from above
2. **Create branch**: `feature/xxxx` from `claude/github-review-9rtnb3`
3. **Implement changes**
4. **Run tests**: `make ci && make security`
5. **Push & create PR** with reference to TODO item
6. **Update this file** when item is completed

### For Core Team

1. **Review quarterly** to update priorities
2. **Assign team members** to Priority 1 items
3. **Track completion** with PR/commit references

---

## Estimated Effort

| Priority | Items | Total Hours | Impact |
|----------|-------|-------------|--------|
| 1 (Security) | 3 items | 8-12h | HIGH |
| 2 (Features) | 3 items | 16-20h | MEDIUM |
| 3 (Testing) | 3 items | 12-16h | MEDIUM |
| 4 (Docs) | 3 items | 8-12h | LOW |
| 5 (Perf) | 3 items | 12-16h | LOW |
| 6 (Compat) | 2 items | 20-24h | MEDIUM |

**Total**: ~76-100 hours for full completion

---

## Quick Wins (< 2 hours each)

- [ ] Fix strcat() in ble_mitm_relay.cpp
- [ ] Add low battery warning
- [ ] Create first tool documentation (WiFi KRACK)
- [ ] Add validation to 3 more attack tools
- [ ] Create GitHub issue template

---

## Tracking

- **Last Updated**: 2025-09-23
- **Next Review**: 2025-12-23 (3 months)
- **Status**: In Progress
