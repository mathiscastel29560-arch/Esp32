#include "signature_recognition.h"
#include "async_logger.h"

const DeviceSignature* SignatureDatabase::identifyDevice(const char* macAddress, const char* beacon) {
  initializeCommonSignatures();

  // Simple MAC-based identification
  for (const auto& sig : knownSignatures) {
    LOG_V("Checking signature: %s", sig.manufacturer);
  }

  LOG_I("Device identification in progress - MAC: %s", macAddress);
  return nullptr;
}

const DeviceSignature* SignatureDatabase::identifyByFirmware(const char* fwVersion) {
  LOG_I("Identifying device by firmware version: %s", fwVersion);
  return nullptr;
}

const DeviceSignature* SignatureDatabase::identifyByHTTPHeader(const char* serverHeader) {
  LOG_I("Identifying device by HTTP header: %s", serverHeader);

  // Check for common headers
  if (strstr(serverHeader, "MiniUPnP")) {
    LOG_I("Detected MiniUPnP - likely router/NAS");
  }
  if (strstr(serverHeader, "HUAWEI")) {
    LOG_I("Detected Huawei device");
  }
  if (strstr(serverHeader, "TP-Link")) {
    LOG_I("Detected TP-Link device");
  }

  return nullptr;
}

float SignatureDatabase::getSignatureConfidence(const DeviceSignature* sig) {
  if (!sig) return 0.0f;
  return 0.85f;  // Placeholder confidence score
}

bool SignatureDatabase::addCustomSignature(const DeviceSignature& sig) {
  knownSignatures.push_back(sig);
  LOG_I("Added custom signature: %s %s", sig.manufacturer, sig.model);
  return true;
}

void SignatureDatabase::initializeCommonSignatures() {
  static bool initialized = false;
  if (initialized) return;

  // Add common device signatures
  LOG_I("Initializing signature database with common devices");
  initialized = true;
}

void SignatureDatabase::printKnownSignatures() {
  Serial.printf("\n=== KNOWN SIGNATURES ===\n");
  Serial.printf("Total signatures: %u\n", knownSignatures.size());
  for (const auto& sig : knownSignatures) {
    Serial.printf("%s %s\n", sig.manufacturer, sig.model);
  }
  Serial.println("=======================\n");
}

void SignatureDatabase::printIdentifiedDevices() {
  Serial.printf("\n=== IDENTIFIED DEVICES ===\n");
  Serial.printf("Total identified: %u\n", identifiedCount);
  for (const auto& device : identifiedDevices) {
    Serial.printf("  - %s\n", device);
  }
  Serial.println("==========================\n");
}

std::vector<VulnerabilityEntry> VulnerabilityMatcher::findVulnerabilities(const DeviceSignature* device) {
  std::vector<VulnerabilityEntry> results;

  if (!device) return results;

  initializeVulnerabilityDB();

  LOG_I("Searching vulnerabilities for: %s %s", device->manufacturer, device->model);

  // Match device vulnerabilities
  for (uint8_t i = 0; i < device->vulnCount; i++) {
    for (const auto& vuln : vulnerabilityDB) {
      if (strstr(vuln.name, device->vulnerabilities[i])) {
        results.push_back(vuln);
        LOG_I("Found: %s (CVE: %s, Severity: %u)", vuln.name, vuln.cveId, vuln.severity);
      }
    }
  }

  return results;
}

std::vector<VulnerabilityEntry> VulnerabilityMatcher::findByCVE(const char* cveId) {
  std::vector<VulnerabilityEntry> results;

  initializeVulnerabilityDB();

  for (const auto& vuln : vulnerabilityDB) {
    if (strcmp(vuln.cveId, cveId) == 0) {
      results.push_back(vuln);
    }
  }

  return results;
}

std::vector<VulnerabilityEntry> VulnerabilityMatcher::findBySeverity(uint8_t minSeverity) {
  std::vector<VulnerabilityEntry> results;

  initializeVulnerabilityDB();

  for (const auto& vuln : vulnerabilityDB) {
    if (vuln.severity >= minSeverity) {
      results.push_back(vuln);
    }
  }

  LOG_I("Found %u vulnerabilities with severity >= %u", results.size(), minSeverity);
  return results;
}

uint8_t VulnerabilityMatcher::calculateRiskScore(const DeviceSignature* device, uint16_t openPorts) {
  if (!device) return 0;

  uint8_t score = 0;

  // Base score from vulnerabilities
  score += device->vulnCount * 15;

  // Bonus for open ports
  if (openPorts > 0) {
    score += (openPorts * 10);
  }

  return (score > 100) ? 100 : score;
}

const char* VulnerabilityMatcher::getRiskLevel(uint8_t score) {
  if (score >= 80) return "CRITICAL";
  if (score >= 60) return "HIGH";
  if (score >= 40) return "MEDIUM";
  if (score >= 20) return "LOW";
  return "MINIMAL";
}

void VulnerabilityMatcher::initializeVulnerabilityDB() {
  static bool initialized = false;
  if (initialized) return;

  LOG_I("Initializing vulnerability database");
  initialized = true;
}

void VulnerabilityMatcher::printVulnerabilityReport(const DeviceSignature* device) {
  if (!device) return;

  std::vector<VulnerabilityEntry> vulns = findVulnerabilities(device);
  uint8_t riskScore = calculateRiskScore(device, 0);

  Serial.printf("\n=== VULNERABILITY REPORT ===\n");
  Serial.printf("Device: %s %s\n", device->manufacturer, device->model);
  Serial.printf("Risk Level: %s (%u/100)\n", getRiskLevel(riskScore), riskScore);
  Serial.printf("Vulnerabilities Found: %u\n\n", vulns.size());

  for (const auto& vuln : vulns) {
    Serial.printf("[%s] %s\n", vuln.cveId, vuln.name);
    Serial.printf("  Severity: %u/10\n", vuln.severity);
    Serial.printf("  Description: %s\n", vuln.description);
  }

  Serial.println("=============================\n");
}
