#pragma once
#include <Arduino.h>

namespace OBD2Scanner {

struct VehicleInfo {
    String vin;
    String make;
    String model;
    String year;
    String ecuVersion;
    bool vulnConnected;
};

struct ScanResult {
    bool success;
    VehicleInfo vehicleInfo;
    uint32_t pidsFound;
    uint32_t durationMs;
};

struct DTCInfo {
    String code;          // "P0101"
    String description;
    bool critical;
    uint32_t occurrences;
};

// Scan for OBD-II devices (CAN bus)
ScanResult scanVehicle(uint32_t durationMs = 30000);

// Read vehicle data
struct ReadResult {
    bool success;
    String parameter;
    String value;
    String unit;
};
ReadResult readParameter(const char* pidCode);

// Read Diagnostic Trouble Codes
struct DTCResult {
    bool success;
    uint32_t codeCount;
    String codes;  // Comma-separated
};
DTCResult readDiagnosticCodes();

// CAN bus analysis
struct CANAnalysis {
    bool success;
    uint32_t messagesCapture;
    uint32_t uniqueIDs;
    String mostActiveID;
};
CANAnalysis analyzeCANBus(uint32_t durationMs = 20000);

// Vulnerability check
struct VulnerabilityCheck {
    bool success;
    bool canBusUnprotected;
    bool defaultCredentials;
    bool unencryptedCommunication;
};
VulnerabilityCheck checkVulnerabilities();

}  // namespace OBD2Scanner
