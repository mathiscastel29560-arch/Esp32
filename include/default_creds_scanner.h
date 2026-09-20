#pragma once
#include <Arduino.h>
#include <vector>

namespace DefaultCredScanner {

struct DefaultCred {
    String username;
    String password;
    String service;  // "HTTP", "SSH", "FTP", "Telnet", "MQTT"
};

struct ScanResult {
    bool found;
    uint16_t devicesScanned;
    uint16_t credentialsAttempted;
    String vulnerableDevices;  // comma-separated list
    String error;
};

std::vector<DefaultCred> getCommonCredentials();
ScanResult scanDefaultCredentials(uint16_t timeoutMs = 60000);
bool testCredential(const String &target, const String &username, const String &password, const String &service);

} // namespace DefaultCredScanner
