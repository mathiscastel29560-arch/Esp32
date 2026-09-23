# 🔐 Credentials Security & Hardening Guide

**This document addresses Vulnerability #5: Hardcoded Credentials**

## ⚠️ Current State

The firmware contains default credentials hardcoded at compile time:

```
AP_SSID:     ESP32-Audit-XXXXXX (chip ID auto-appended)
AP_PASSWORD: auditctrl123 (default, CHANGEABLE)
```

These are **intentionally exposed** for quick lab/development use, but pose a security risk if left unchanged.

## 🎯 When This is Acceptable

✅ **Safe to keep defaults:**
- Personal lab devices not connected to public networks
- Air-gapped WiFi networks (Faraday cage, shielded room)
- CTF competitions with short-lived devices
- Development/testing only

❌ **MUST change before:**
- Any real penetration testing engagement
- Devices near public networks
- Long-term field deployments
- Shared lab environments

## 🔧 How to Change Credentials

### Option 1: Change at Compile Time (Recommended)

Edit `include/config.h` before building:

```cpp
#define AP_PASSWORD "YourSecurePasswordHere"
```

Then rebuild:
```bash
pio run
pio run -t upload
```

**Pros:**
- Credentials never visible after compilation
- No runtime overhead
- Built into firmware binary

**Cons:**
- Requires recompilation to change
- Password visible in source code

### Option 2: Change at Runtime (Better for Iteration)

Create `WiFiConfig.h` to load password from EEPROM/LittleFS:

```cpp
#include <LittleFS.h>

String getAPPassword() {
    if (LittleFS.exists("/cfg/ap_pwd")) {
        File f = LittleFS.open("/cfg/ap_pwd", "r");
        String pwd = f.readString();
        f.close();
        return pwd;
    }
    return "auditctrl123";  // Fallback only
}
```

Then modify `main.cpp`:

```cpp
String password = getAPPassword();
WifiTools::begin(apSsid, password);
```

**Pros:**
- Change password without recompilation
- Can be uploaded via web panel
- Credentials stored encrypted in LittleFS

**Cons:**
- Slightly more complex
- Requires LittleFS encryption setup

### Option 3: Dynamic Generation (Most Secure)

Generate password from chip ID + timestamp:

```cpp
String generateSecurePassword() {
    uint32_t chipId = ESP.getChipId();
    uint32_t timestamp = millis() / 1000;
    char pwd[16];
    snprintf(pwd, sizeof(pwd), "%08X%08X", chipId, timestamp);
    return String(pwd);
}
```

**Pros:**
- Unique per device, per session
- No hardcoded credentials in firmware
- Requires physical access to device + web panel to find password

**Cons:**
- Can't memorize/share password easily
- Requires each session to check device screen

## 📋 Checklist Before Field Deployment

- [ ] AP_PASSWORD changed from "auditctrl123"
- [ ] New password is at least 12 characters
- [ ] New password contains mixed case + numbers + symbols
- [ ] WiFi AP only accessible from air-gapped network
- [ ] No default credentials in any code commit
- [ ] `.gitignore` includes local config files with real passwords
- [ ] Firmware binary scrubbed before sharing (use `strings firmware.bin` to verify)
- [ ] Team aware this is audit-only tool

## 🔒 Other Hardcoded Values to Review

### Default Scanner Wordlists

`src/default_creds_scanner.cpp` contains test credentials for common devices:

```cpp
const DefaultCred defaultCreds[] = {
    {"admin", "password", "HTTP"},
    {"admin", "admin", "HTTP"},
    {"root", "root", "SSH"},
    // ... 50+ entries
};
```

**Status**: These are **intentional** for testing purposes. Each entry is clearly a test value. No real credentials here.

**Recommendation**: These are fine for audit tools. Document that they're test-only and used for authorized assessments only.

### MQTT/CoAP Scanner Defaults

Files like `src/mqtt_hijacker_impl.cpp` contain test usernames/passwords for protocol scanning.

**Status**: Test/demo values only. Clearly marked in comments.

**Recommendation**: Keep as-is for tool functionality. These are needed for the tool to work.

## 🛡️ What Was NOT Changed (and why)

- **Test credentials in scanners**: These are part of the tool's function (finding weak creds)
- **IR/RF patterns**: Not credentials, just hex codes for common devices
- **API endpoints**: Test URLs for development, not secrets

## 📊 Vulnerability Impact

**Before this guide**: 
- Default password easily extracted from firmware binary: `strings esp32.bin | grep audit`
- No guidance on hardening

**After this guide**:
- Clear security posture documented
- Options provided for credential management
- Pre-deployment checklist included

## ⏭️ Future Improvements

1. **Web-based password setter** (requires auth)
2. **Encrypted LittleFS credentials storage**
3. **QR code WiFi provisioning** (WPS alternative)
4. **CI/CD check** preventing commits with default passwords

---

**Last Updated**: 2026-09-22  
**Status**: Vulnerability #5 - Partially Mitigated  
**Classification**: Security Hardening Guide - Audit Tool
