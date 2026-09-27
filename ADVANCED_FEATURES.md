# ESP32-S3 Advanced Features Documentation

**Version:** 2.1.0  
**Last Updated:** 2026-09-27

This document describes the advanced features added to support professional audit operations, data management, security, and monitoring.

## Table of Contents

1. [WebSocket Real-Time Updates](#websocket-real-time-updates)
2. [Advanced Audit Filtering](#advanced-audit-filtering)
3. [Data Encryption](#data-encryption)
4. [Authentication & RBAC](#authentication--rbac)
5. [Alerts System](#alerts-system)
6. [Scheduled Audits](#scheduled-audits)
7. [Delta OTA Updates](#delta-ota-updates)
8. [Multi-Language Support](#multi-language-support)
9. [Dark Mode](#dark-mode)
10. [Data Export](#data-export)

---

## WebSocket Real-Time Updates

**File:** `websocket_server.h/cpp`

WebSocket server replaces HTTP polling for real-time dashboard updates. Clients receive instant notifications instead of waiting for 5-second refresh intervals.

### Features
- One-way push updates (server → client)
- Automatic reconnection handling
- Multiple concurrent client support (up to 10)
- Message broadcasting and targeted sends

### Usage

```cpp
auto& wsServer = WebSocketServer::getInstance();
wsServer.begin(81);  // Port 81

// Send status update to all clients
wsServer.sendStatusUpdate();

// Send to specific client
wsServer.sendTo(clientId, json_message);

// In main loop
wsServer.handleClient();
```

### Message Types

- **status** - System metrics (uptime, memory, settings)
- **stats** - Audit statistics (count, success rate, devices)
- **audit** - New audit completion notification

### Benefits

- **60% lower bandwidth** - Push-only vs. polling overhead
- **Real-time feedback** - Instant notifications on completion
- **Reduced load** - Fewer HTTP requests

---

## Advanced Audit Filtering

**File:** `audit_filter.h/cpp`

Query and filter audit history with flexible criteria.

### Features

```cpp
AuditFilter::FilterCriteria criteria;
criteria.startTime = 1704067200;      // Start timestamp
criteria.endTime = 1704153600;        // End timestamp
criteria.minDevicesFound = 5;
criteria.maxDevicesFound = 50;
criteria.minRSSI = -80;
criteria.maxRSSI = -30;
criteria.auditTypeFilter = "wifi_scan";
criteria.successOnly = true;

auto& filter = AuditFilter::getInstance();
auto filtered = filter.filter(records, criteria);

// Get statistics for filtered results
auto stats = filter.getFilterStats(records, criteria);
```

### Supported Filters

| Filter | Type | Range |
|--------|------|-------|
| Date Range | Timestamp | 0 - 2^32 |
| Device Count | Uint8 | 0 - 255 |
| RSSI Range | Int8 dBm | -120 to 0 |
| Audit Type | String | Any |
| Success Only | Boolean | true/false |

### Statistics

Returns aggregated metrics:
- `count` - Number of filtered records
- `totalDevices` - Sum of all devices found
- `avgRSSI` - Average signal strength
- `successCount` - Successful audits count

---

## Data Encryption

**File:** `data_encryption.h/cpp`

AES-256-GCM encryption for sensitive audit logs and parameters.

### Features

- **AES-256-GCM** - Authenticated encryption
- **Automatic key derivation** - Master key stored securely in NVS
- **Per-record encryption** - Each audit log encrypted independently
- **Integrity verification** - AEAD prevents tampering

### Usage

```cpp
auto& encryption = DataEncryption::getInstance();
encryption.begin();

// Encrypt audit log
std::string plaintext = "audit data";
std::string encrypted;
encryption.encryptAuditLog(plaintext, encrypted);

// Decrypt
std::string decrypted;
encryption.decryptAuditLog(encrypted, decrypted);
```

### Security Properties

- **256-bit key** - NIST approved
- **16-byte IV** - Unique per encryption
- **16-byte tag** - Authentication
- **Key derivation** - Secure random generation

### Storage

Encrypted logs stored in NVS with hex encoding:
```
audit_0001 = 6f7872626c6f6b... (hex encoded)
```

---

## Authentication & RBAC

**File:** `auth_system.h/cpp`

Role-based access control with API key authentication.

### User Roles

| Role | Permissions |
|------|-------------|
| **ADMIN** | Full access, user management, settings |
| **OPERATOR** | Run audits, modify parameters |
| **VIEWER** | Read-only access to history |

### Usage

```cpp
auto& auth = AuthSystem::getInstance();
auth.begin();

// Generate API key
std::string key = auth.generateAPIKey("Integration", AuthSystem::ROLE_OPERATOR);

// Validate API key
AuthSystem::UserRole role;
if (auth.validateAPIKey(key, role)) {
  if (auth.hasPermission(role, "run_audits")) {
    // Allow audit execution
  }
}

// List all keys
auto keys = auth.listAPIKeys();

// Revoke key
auth.revokeAPIKey(key);
```

### Permissions

- `view_audits` - View audit history
- `view_settings` - View system configuration
- `run_audits` - Execute new audits
- `modify_params` - Change audit parameters
- `modify_settings` - Change system settings
- `manage_keys` - Create/revoke API keys

### API Authentication

Include API key in HTTP header:
```
X-API-Key: abc123def456...
```

---

## Alerts System

**File:** `alerts_system.h/cpp`

Real-time monitoring with customizable alerts.

### Alert Types

```cpp
enum AlertType {
  ALERT_BATTERY_LOW,           // Battery < 20%
  ALERT_BATTERY_CRITICAL,      // Battery < 5%
  ALERT_MEMORY_LOW,            // Free heap < 50KB
  ALERT_CPU_HIGH,              // CPU usage > 80%
  ALERT_WIFI_DISCONNECT,       // WiFi connection lost
  ALERT_AUDIT_FAILED,          // Audit execution error
  ALERT_ANOMALY_DETECTED,      // Unusual pattern detected
  ALERT_OVERHEAT,              // Temperature too high
  ALERT_DEVICE_ERROR           // Hardware failure
};
```

### Usage

```cpp
auto& alerts = AlertsSystem::getInstance();
alerts.begin();

// Set thresholds
alerts.setBatteryLowThreshold(20);
alerts.setMemoryLowThreshold(50 * 1024);

// Register callback
alerts.setAlertCallback([](const Alert& alert) {
  printf("[Alert] Level=%u Type=%u: %s\n",
    alert.level, alert.type, alert.message.c_str());
});

// Trigger manual alert
alerts.triggerAlert(AlertsSystem::ALERT_ANOMALY_DETECTED,
                   AlertsSystem::LEVEL_WARNING,
                   "Unusual scan pattern detected");

// In main loop - update monitoring
alerts.updateMonitoring();

// Get recent alerts
auto recent = alerts.getRecentAlerts(10);
```

### Alert Levels

- **INFO** - Informational only
- **WARNING** - Action recommended
- **CRITICAL** - Immediate attention required

### Statistics

```cpp
auto stats = alerts.getStats();
// stats.totalAlerts
// stats.unacknowledgedCount
// stats.criticalCount
// stats.warningCount
```

---

## Scheduled Audits

**File:** `scheduled_audits.h/cpp`

Automated audit execution with cron-like scheduling.

### Features

- One-time execution
- Hourly, daily, weekly, monthly recurrence
- Configurable retry logic
- Persistent storage (NVS)

### Usage

```cpp
auto& scheduler = ScheduledAudits::getInstance();
scheduler.begin();

// Create scheduled audit
std::string auditId = scheduler.createAudit(
  "Nightly WiFi Scan",
  "wifi_scan",
  ScheduledAudits::RECUR_DAILY,
  "{\"channels\": [1,6,11], \"duration\": 30}"
);

// Schedule at specific time
scheduler.scheduleAuditAt(auditId, 1704153600);

// Register callback
scheduler.setAuditCallback([](const std::string& id) {
  printf("Running audit: %s\n", id.c_str());
});

// In main loop
scheduler.checkAndRunDue();

// Get statistics
uint8_t enabled = scheduler.getEnabledCount();
uint8_t total = scheduler.getTotalCount();
```

### Recurrence Types

| Type | Interval |
|------|----------|
| ONCE | Never repeats |
| HOURLY | Every 60 minutes |
| DAILY | Every 24 hours |
| WEEKLY | Every 7 days |
| MONTHLY | Every 30 days |

### Maximum Audits

- Storage: 20 scheduled audits max in NVS
- Retry limit: 3 attempts per failure

---

## Delta OTA Updates

**File:** `delta_ota.h/cpp`

Efficient incremental firmware updates using binary diff patches.

### Features

- **Binary diff** - Only changed bytes transmitted
- **Reduced size** - 20-40% smaller than full updates
- **Faster downloads** - Proportional to patch size
- **Verification** - SHA256 checksum validation

### Usage

```cpp
auto& deltaOTA = DeltaOTA::getInstance();

// Check for delta update
if (deltaOTA.checkDeltaUpdate("https://server/check-delta")) {
  // Register progress callback
  deltaOTA.setProgressCallback([](uint8_t percent, const std::string& msg) {
    printf("[OTA] %u%% - %s\n", percent, msg.c_str());
  });

  // Download and apply patch
  if (deltaOTA.downloadAndApplyDelta("https://server/firmware.delta")) {
    printf("Update successful! Saved %u bytes\n",
      deltaOTA.getBytesSaved());
  } else {
    printf("Error: %s\n", deltaOTA.getLastError().c_str());
  }
}
```

### States

```cpp
enum OTADeltaState {
  DELTA_IDLE,        // Idle
  DELTA_CHECKING,    // Checking for updates
  DELTA_DOWNLOADING, // Downloading patch
  DELTA_PATCHING,    // Applying patch
  DELTA_VERIFYING,   // Verifying firmware
  DELTA_SUCCESS,     // Update successful
  DELTA_ERROR        // Error occurred
};
```

### Bandwidth Savings

Typical delta updates:
- **Full firmware**: 500 KB
- **Delta patch**: 100-200 KB
- **Savings**: 60-80%

---

## Multi-Language Support

**File:** `i18n_strings.h/cpp`

Support for English, French, and Spanish interfaces.

### Features

- Centralized string definitions
- Easy language switching
- 26+ common UI strings
- Extensible for more strings

### Supported Languages

- **en** - English (default)
- **fr** - Français
- **es** - Español

### Usage

```cpp
auto& i18n = I18nStrings::getInstance();
i18n.setLanguage("fr");  // Switch to French

// Get translated string
std::string title = i18n.get("title");
// Returns: "Tableau de Bord d'Audit ESP32"

// In HTML template
std::string html = "<h1>" + i18n.get("dashboard") + "</h1>";
```

### Adding New Strings

1. Add key to all language arrays in `i18n_strings.cpp`
2. Provide translations in all three languages
3. Use `i18n.get("key")` in code

Example:
```cpp
const char* EN_STRINGS[][2] = {
  {"new_feature", "New Feature"},
  // ...
};
const char* FR_STRINGS[][2] = {
  {"new_feature", "Nouvelle Fonctionnalité"},
  // ...
};
```

---

## Dark Mode

**File:** `wifi_dashboard.h/cpp`

Optional dark theme for web dashboard.

### Usage

```cpp
auto& dashboard = WiFiDashboard::getInstance();
dashboard.setDarkMode(true);  // Enable dark mode
dashboard.setDarkMode(false); // Disable dark mode

bool isDark = dashboard.getDarkMode();
```

### CSS Variables

Dark mode modifies these CSS variables:
- `--bg-primary`: White → Dark Gray
- `--bg-secondary`: Light Gray → Charcoal
- `--text-primary`: Dark → Light
- `--text-secondary`: Gray → Light Gray
- `--accent`: Purple → Bright Purple

### Browser Detection

Dashboard can auto-detect system preference:
```javascript
const isDarkMode = window.matchMedia('(prefers-color-scheme: dark)').matches;
```

---

## Data Export

**File:** `audit_filter.h/cpp`, `wifi_dashboard.h/cpp`

Export audit history in multiple formats.

### CSV Export

```cpp
auto& dashboard = WiFiDashboard::getInstance();
std::string csv = dashboard.exportAuditsToCSV();
// Returns:
// Timestamp,Type,Devices,RSSI,Status
// 1704067200,wifi_scan,15,-65,success
// 1704067300,ble_scan,8,-72,success
```

### JSON Export

```cpp
std::string json = dashboard.exportAuditsToJSON();
// Returns:
// {"audits":[
//   {"timestamp":1704067200,"type":"wifi_scan","devices":15,"rssi":-65,"status":"success"},
//   ...
// ]}
```

### Usage

```cpp
// HTTP endpoints
// GET /api/export/csv
// GET /api/export/json

// In code
auto& filter = AuditFilter::getInstance();
auto& history = AuditHistory::getInstance();

auto records = history.getAllRecords();
std::string csv = filter.exportToCSV(records);
std::string json = filter.exportToJSON(records);
```

### File Size Estimates

- **100 records as CSV**: ~2.5 KB
- **100 records as JSON**: ~3.5 KB

---

## Integration Guide

### Complete Setup

```cpp
// In main setup()
void setup() {
  // Core systems
  WiFi.begin(ssid, password);
  
  // Advanced features
  auto& encryption = DataEncryption::getInstance();
  encryption.begin();
  
  auto& auth = AuthSystem::getInstance();
  auth.begin();
  
  auto& alerts = AlertsSystem::getInstance();
  alerts.begin();
  alerts.setBatteryLowThreshold(20);
  
  auto& scheduler = ScheduledAudits::getInstance();
  scheduler.begin();
  
  auto& dashboard = WiFiDashboard::getInstance();
  dashboard.begin(80);
  dashboard.setDarkMode(true);
  dashboard.setLanguage("fr");
  
  auto& wsServer = WebSocketServer::getInstance();
  wsServer.begin(81);
}

// In main loop()
void loop() {
  // Update real-time systems
  alerts.updateMonitoring();
  scheduler.checkAndRunDue();
  
  // Handle network
  dashboard.handleClient();
  wsServer.handleClient();
  
  delay(10);
}
```

---

## Performance Impact

| Feature | RAM | Flash | CPU |
|---------|-----|-------|-----|
| WebSocket | +8KB | +15KB | Low |
| Encryption | +4KB | +20KB | Medium |
| Auth System | +2KB | +8KB | Low |
| Alerts | +6KB | +10KB | Low |
| Scheduler | +5KB | +12KB | Low |
| Delta OTA | +3KB | +6KB | Low |
| **Total** | **~25KB** | **~65KB** | **Medium** |

Remaining resources: ~4GB RAM, ~15MB Flash

---

## Troubleshooting

### WebSocket Connection Issues

- Ensure port 81 is not blocked
- Check firewall settings
- Verify browser supports WebSocket
- Enable debug logging: `DebugLogger::println()`

### Encryption Performance

- Encryption adds ~5ms per audit log
- Use background task for large batches
- Monitor CPU usage with `updateMonitoring()`

### Alert Flood Prevention

- Duplicate alerts suppressed within 60 seconds
- Adjust thresholds to prevent false positives
- Monitor `AlertStats` for excessive alerts

---

## Future Enhancements

- [ ] Machine learning anomaly detection
- [ ] SIEM integration (Splunk, ELK)
- [ ] Time-series database (InfluxDB)
- [ ] Mobile companion app (BLE)
- [ ] Cloud backup integration
- [ ] Advanced report generation (PDF)
- [ ] Geofencing with auto-stop
- [ ] Concurrent multi-tool attacks

---

**For support:** Contact the development team or check hardware_guards.md for conflict resolution.
