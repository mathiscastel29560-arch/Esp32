# Phase 2: Logging & Persistence System

## Overview

Phase 2 implements a comprehensive logging, auditing, and result persistence system for all tools.

**Components:**
1. **AuditLog** - Event-based audit trail (1 event per tool start/stop)
2. **ToolResultPersistence** - Structured result storage (JSON/CSV)
3. **HighSpeedLogBuffer** - PSRAM buffer for intensive operations (1000+ events/sec)
4. **LogViewerMenu** - Menu interface for reviewing logs
5. **OperationLogger** - Unified wrapper for all logging needs

## Quick Start

### Simple Tool Logging

```cpp
#include "audit_log.h"
#include "tool_result_persistence.h"

ScanResult scanDevices(uint32_t durationMs) {
    // Log start
    AuditLog::instance().logToolStart("MyScanner", "duration=5000");
    
    ScanResult result = performScan();
    
    // Store result
    String json = "{\"devices\":" + String(result.count) + "}";
    ToolResultPersistence::instance().storeToolResult("MyScanner", json.c_str());
    
    // Log completion
    AuditLog::instance().logToolStop("MyScanner", result.success, "scan_complete");
    
    return result;
}
```

### High-Speed Operation Logging

For packet-by-packet jamming, WiFi sniffing, or RF scanning:

```cpp
#include "operation_logger.h"

JamResult generateJamming(uint32_t durationMs, const String &type) {
    // Initialize PSRAM buffer (256KB default)
    OperationLogger::initBuffer(256);
    
    OperationLogger::start("JammingTool", "type=" + type);
    
    uint32_t packets_jammed = 0;
    
    while (jamming_active) {
        // For each packet/event, log to PSRAM (non-blocking)
        if (packets_jammed % 100 == 0) {
            String event = "packets=" + String(packets_jammed);
            OperationLogger::logEvent("JammingTool", event.c_str());
        }
        packets_jammed++;
    }
    
    // Flush all events to disk and persist result
    String result_json = "{\"packets\":" + String(packets_jammed) + "}";
    OperationLogger::complete("JammingTool", result_json.c_str());
    
    return result;
}
```

### Device Discovery Logging

```cpp
#include "tool_result_persistence.h"

void onDeviceFound(const BLEDevice &device) {
    // Log each discovery
    String device_json = "{\"name\":\"" + device.name +
                        "\",\"rssi\":" + String(device.rssi) + "}";
    ToolResultPersistence::instance().storeDeviceResult("BLEScanner", device_json.c_str());
}
```

## File Organization

### Audit Logs
```
/logs/audit/
├── audit_0.csv      (Day 0 audit trail)
├── audit_1.csv      (Day 1 audit trail)
└── audit_7.csv      (Week old logs, marked for cleanup)
```

Format: `timestamp,event_type,module,free_heap,details`

Example:
```
1234567,3,ZwaveScanner,87012,5_nodes
1234568,5,ZwaveScanner,86980,device=1:ControllerStatic:Aeotec
```

### Tool Results
```
/results/
├── tools/
│   ├── ZwaveScanner_1234567.json
│   ├── WPA2Cracker_1234568.json
│   └── ...
├── devices/
│   ├── SmartLockScanner_1234569.json
│   └── ...
├── attacks/
│   ├── BLESpoof_1234570.json
│   └── ...
└── buffer_*.log      (PSRAM buffer flush files)
```

### High-Speed Buffer
```
/results/
└── buffer_1234567.log  (PSRAM circular buffer dump)

Format per line: [timestamp:tool:data]
```

## Cleanup Policies

### Automatic Retention
```cpp
// Keep last 7 days of logs
AuditLog::instance().clearOldLogs(7);
ToolResultPersistence::instance().clearOldResults(7);
```

### Manual Export
```cpp
// Export audit logs as CSV
ToolResultPersistence::instance().exportResultsToCSV(
    "/export/audit.csv", "tools");
```

## Menu Integration

Access from main menu:
```
Log & Results Manager
├── 1. View Audit Logs
├── 2. View Tool Results  
├── 3. View Device Discoveries
├── 4. View Attack Results
├── 5. Storage Statistics
├── 6. Cleanup Old Logs
└── 7-8. Export to CSV
```

## Performance Characteristics

| Operation | Overhead | Speed |
|-----------|----------|-------|
| logToolStart | ~1ms | AuditLog direct write |
| logEvent (PSRAM) | <100µs | Non-blocking |
| storeToolResult | ~5-10ms | LittleFS write |
| exportToCSV | Variable | Depends on file size |

## Storage Requirements

| Component | Size |
|-----------|------|
| Audit logs (7 days) | ~50-100 KB |
| Tool results (7 days) | ~100-200 KB |
| Device results (7 days) | ~50-100 KB |
| PSRAM buffer | 256 KB (configurable) |
| Total | ~500-700 KB |

## Troubleshooting

### PSRAM buffer full
- Lower buffer size: `HighSpeedLogBuffer::instance().begin(128);`
- Increase flush frequency in tool code
- Use audit log instead for less critical events

### Storage full
- Export and delete old results
- Reduce retention period: `clearOldLogs(1);`
- Disable persistence for non-critical tools

### Audit log corrupted
- Check /logs/audit/ directory
- Use LogViewerMenu to export and verify
- Manual cleanup: `rm /logs/audit/audit_*.csv`

## Best Practices

1. **Use OperationLogger** for complex operations (DRY principle)
2. **Enable PSRAM buffer** for packet-level logging (jamming, sniffing)
3. **Export regularly** from LogViewerMenu before storage fills
4. **Disable persistence** for memory-constrained scenarios
5. **Batch events** before logging (log every 100 packets, not every packet)

## Examples by Tool Type

### Scanner Tool
```cpp
AuditLog::instance().logToolStart("Scanner", "");
// ... scan ...
for (device in devices) {
    ToolResultPersistence::instance().storeDeviceResult("Scanner", device_json);
}
OperationLogger::complete("Scanner", summary_json);
```

### Attack Tool
```cpp
OperationLogger::start("Attacker", "target=X");
OperationLogger::initBuffer(256);
for (i = 0; i < packets; i++) {
    OperationLogger::logEvent("Attacker", event_str);
}
OperationLogger::complete("Attacker", result_json);
```

### Device Discovery
```cpp
ToolResultPersistence::instance().storeDeviceResult("Tool", json);
```

## Future Enhancements

- [ ] SQL-like querying interface
- [ ] Compressed log storage (gzip)
- [ ] Real-time dashboard
- [ ] Automated report generation
- [ ] Log synchronization to external storage
- [ ] Encrypted audit trail
