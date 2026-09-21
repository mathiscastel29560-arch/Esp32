# API Testing & Verification Guide

**Date:** 2026-09-21  
**Status:** PHASE 1 - ESP32 Verification

## Overview

This guide provides step-by-step instructions for testing all ESP32 API endpoints with the production web application at: https://claude.ai/artifact/JhLoBafP57aco36kFrX8wW

## Prerequisites

1. ESP32-S3 device compiled and running (firmware version verified: 61.9% flash usage)
2. Device in SoftAP mode (check AUDIT_COMPLET.md for WiFi config)
3. Web browser on iPhone/mobile device connected to `esp32-audit` WiFi
4. Production app loaded at `http://192.168.4.1:8080/`

## API Endpoints - Testing Checklist

### 1. System Endpoints

#### GET /api/status
- **Expected Response:** JSON with time, GPS status, battery %, connected clients
- **Test:** Click "Test Connexion" in Settings tab
- **Success Criteria:** 
  - ✅ Returns JSON with all fields
  - ✅ Battery % is reasonable (0-100)
  - ✅ apClients count is >= 1 (your device)
  - ✅ Status badge turns GREEN

#### GET / (Root)
- **Expected Response:** HTML control panel
- **Test:** Click "Statut ESP32" in Settings → should fetch and display status
- **Success Criteria:** 
  - ✅ Dashboard loads without errors
  - ✅ Status details show in dashboard

### 2. WiFi Attack Endpoints

#### GET /api/wifi/scan
- **Status:** ✅ Implemented
- **Usage:** (Currently in web app as "Scan WiFi" utility)
- **Test:** Add utility button to Attacks tab
- **Expected:** Returns array of nearby WiFi networks with SSID, BSSID, RSSI, channel

#### POST /api/wifi/deauth
- **Required Parameters:**
  - `bssid` - Target AP MAC (e.g., "AA:BB:CC:DD:EE:FF")
  - `client` - Target client MAC or broadcast "FF:FF:FF:FF:FF:FF"
  - `channel` - WiFi channel (1-11)
  - `frames` - Number of deauth frames (default: 30)
- **Test Steps:**
  1. Go to Attacks tab
  2. Select "WiFi Deauth" from dropdown
  3. Enter a target BSSID (scan first to get real MACs)
  4. Set duration (10-30 seconds)
  5. Click "Lancer l'Attaque"
- **Success Criteria:**
  - ✅ Request sent (app shows "✅ Attaque envoyée")
  - ✅ Result recorded in Results tab
  - ✅ Device logs show deauth frames sent

#### POST /api/wifi/beacon/start
- **Required Parameters:**
  - `ssids` - Comma-separated network names (e.g., "FreeWiFi,Starbucks,Airport")
  - `hop` - Boolean, enable channel hopping (default: true)
- **Test Steps:**
  1. Attacks tab → Select "Beacon Spam"
  2. Enter multiple SSIDs in target field (comma-separated)
  3. Click attack button
- **Success Criteria:**
  - ✅ JSON response with `"ok":true`
  - ✅ Beacon frames visible in packet sniffer (if testing with another device)
  - ✅ Status shows "active" when running

#### POST /api/wifi/beacon/stop
- **Test:** Open another tab, set IP/port, call this endpoint
- **Success Criteria:** Beacon spam stops, status returns `"active":false`

#### GET /api/wifi/beacon/status
- **Test:** Check status without stopping
- **Expected:** Returns `{"active": true/false}`

### 3. BLE Attack Endpoints

#### GET /api/ble/scan?seconds=5
- **Status:** ✅ Implemented
- **Expected Response:** Array of BLE devices with address, name, RSSI, manufacturer
- **Test:** (Add to web app utilities section)
- **Success Criteria:**
  - ✅ Returns valid BLE device array
  - ✅ Contains nearby BLE devices
  - ✅ RSSI values in range (-100 to -20)

#### POST /api/ble/spam/start
- **Test:** Attacks tab → "BLE Spam" → Click button
- **Expected:** `{"ok": true}`
- **Success Criteria:**
  - ✅ BLE spam detection starts
  - ✅ Web app shows attack launched
  - ✅ Monitor logs show scanning active

#### POST /api/ble/spam/stop
- **Test:** Call endpoint (add stop button in web app)
- **Expected:** `{"ok": true}`

#### GET /api/ble/spam/check
- **Test:** While spam is running, check for alerts
- **Expected:** JSON with `active`, `type`, `distinctMacs`, `strongestRssi`
- **Success Criteria:**
  - ✅ Detects spam if generated
  - ✅ Counts MACs correctly
  - ✅ RSSI readings accurate

#### POST /api/ble/gatt-audit?address=XX:XX:XX:XX:XX:XX
- **Test:** 
  1. Get BLE device address from scan
  2. Attacks tab → "BLE GATT Audit"
  3. Enter BLE device address
  4. Click attack
- **Expected Response:** JSON with characteristics, security findings
- **Success Criteria:**
  - ✅ Connects to device
  - ✅ Enumerates GATT services
  - ✅ Reports readable/writable characteristics

#### POST /api/ble/fuzz?address=XX:XX:XX:XX:XX:XX
- **Test:** Attacks tab → "BLE Fuzz" → Similar to GATT audit
- **Expected:** JSON with fuzzing statistics (oversized writes, protocol violations)
- **Success Criteria:**
  - ✅ Device handled fuzz gracefully
  - ✅ JSON response with metrics

### 4. RF Scanning Endpoints

#### GET /api/nrf24/scan?samples=50
- **Status:** ✅ Implemented
- **Expected:** Array of channel activity levels
- **Test:** Settings → Add utility button "NRF24 Scan"
- **Success Criteria:** Returns 126 values (NRF24 channel activity)

#### GET /api/subghz/rssi?freq=433.92
- **Status:** ✅ Implemented
- **Expected:** `{"rssi": -45}` (example)
- **Test:** Settings → Add utility "Sub-GHz RSSI"
- **Success Criteria:** Returns integer RSSI value

#### POST /api/subghz/record?freq=433.92&ms=8000
- **Status:** ✅ Implemented
- **Expected:** `{"pulses": 1234, "file": "/path/to/capture.cap"}`
- **Test:** Attacks tab → "Sub-GHz Record"
- **Success Criteria:**
  - ✅ Records signal pulses
  - ✅ File saved to LittleFS
  - ✅ Pulse count > 0

#### POST /api/subghz/replay
- **Status:** ✅ Implemented
- **Expected:** `{"ok": true}` or `{"ok": false, "reason": "no capture yet"}`
- **Test:** After recording, call replay
- **Success Criteria:** Replayed signal (requires TX arm)

### 5. Evil Portal Endpoints

#### POST /api/portal/start?ssid=FAKE&maxMs=600000
- **Status:** ✅ Implemented
- **Expected:** `{"ok": true}` (requires TX arm)
- **Test:** Settings → Add "Evil Portal" utility
- **Success Criteria:**
  - ✅ Fake AP appears in WiFi networks
  - ✅ Credentials captured when clients connect
  - ✅ Max timeout respected (10 min default)

#### POST /api/portal/stop
- **Expected:** `{"ok": true}`

#### GET /api/portal/status
- **Expected:** `{"active": true/false}`

#### GET /api/portal/log
- **Expected:** CSV file with captured credentials
- **Test:** Download and verify format

### 6. Infrared Endpoints

#### POST /api/ir/power
- **Status:** ✅ Implemented
- **Expected:** `{"ok": true}`
- **Test:** Point ESP32 at TV, click button
- **Success Criteria:** TV turns off/on

#### POST /api/ir/learn?ms=5000
- **Expected:** `{"ok": true, "pulses": 123}`
- **Test:** IR receiver ready, click learn
- **Success Criteria:** Captures IR signal within 5 seconds

#### POST /api/ir/replay
- **Expected:** `{"ok": true}`
- **Test:** Replay captured signal
- **Success Criteria:** Device responds to IR

### 7. Wardriving Endpoints

#### POST /api/wardrive/snapshot
- **Status:** ✅ Implemented
- **Expected:** `{"rows": 10, "total": 100}`
- **Test:** (Only works with GPS fix)
- **Success Criteria:**
  - ✅ Captures GPS + WiFi networks
  - ✅ Row count increases

#### GET /api/wardrive/log
- **Expected:** CSV file with GPS coordinates and nearby networks
- **Test:** Download and verify Kismet format compatibility

## Web App Testing

### Dashboard Tab
- [ ] Connection status displays correctly
- [ ] Battery % updates every 10 seconds
- [ ] Client count accurate
- [ ] Recent results display last 5 attacks
- [ ] Success rate calculation correct

### Attacks Tab
- [ ] Dropdown shows all 7 attack types
- [ ] Can select target/parameters
- [ ] Response message displays
- [ ] Results recorded

### Results Tab
- [ ] Filter buttons work (All, Success, Failed)
- [ ] Results list displays in reverse chronological order
- [ ] Pagination or scroll works

### Settings Tab
- [ ] IP/Port inputs save to localStorage
- [ ] Connect button updates status
- [ ] All utilities available
- [ ] Reset function clears data

## Error Scenarios

### Network Errors
- [ ] Device disconnected - shows "OFFLINE" immediately
- [ ] Wrong IP/Port - shows error message
- [ ] Timeout (10+ seconds) - handled gracefully

### Attack Errors
- [ ] Invalid target MAC - shows error
- [ ] Deauth blocked (TX not armed) - shows in response
- [ ] Beacon spam with empty SSID - handled

### Storage Errors
- [ ] localStorage full - app handles gracefully
- [ ] Restore from corrupted data - defaults to empty

## Performance Metrics

### Connection Time
- [ ] `/api/status` responds in < 500ms
- [ ] Attack launch in < 2000ms
- [ ] Dashboard refresh < 1000ms

### Memory Usage
- [ ] RAM stable at 25.1% after 1 hour
- [ ] No memory leaks on repeated attacks
- [ ] Battery drain reasonable (~5% per hour in SoftAP mode)

### Concurrency
- [ ] Multiple clients connected - all receive status updates
- [ ] Simultaneous attacks - safely queued or rejected

## Commit Summary

**Files Modified:**
- `include/web_ctrl.h` - Added comprehensive API documentation
- `AUDIT_COMPLET.md` - Audit report with compilation details

**Build Status:** ✅ SUCCESS (35.27s, no errors or warnings)

**Memory Status:**
- RAM: 25.1% (82KB/328KB) - EXCELLENT
- Flash: 61.9% (1.9MB/3.1MB) - GOOD

**Next Phase:** After verification, proceed to PHASE 2 (Web App Integration Testing)
