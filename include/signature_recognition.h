#ifndef SIGNATURE_RECOGNITION_H
#define SIGNATURE_RECOGNITION_H

#include <Arduino.h>
#include <vector>

// ============= SIGNATURE DATABASE =============

struct DeviceSignature {
  const char* manufacturer;
  const char* model;
  const char* firmware;
  const char* vulnerabilities[5];
  uint8_t vulnCount;
};

class SignatureDatabase {
public:
  static SignatureDatabase& getInstance() {
    static SignatureDatabase instance;
    return instance;
  }

  // Device identification
  const DeviceSignature* identifyDevice(const char* macAddress, const char* beacon);
  const DeviceSignature* identifyByFirmware(const char* fwVersion);
  const DeviceSignature* identifyByHTTPHeader(const char* serverHeader);

  // Signature matching
  float getSignatureConfidence(const DeviceSignature* sig);
  bool addCustomSignature(const DeviceSignature& sig);

  void printKnownSignatures();
  void printIdentifiedDevices();

private:
  SignatureDatabase() : identifiedCount(0) {}

  std::vector<DeviceSignature> knownSignatures;
  std::vector<const char*> identifiedDevices;
  uint16_t identifiedCount;

  void initializeCommonSignatures();
};

// ============= VULNERABILITY MATCHER =============

struct VulnerabilityEntry {
  const char* cveId;
  const char* name;
  uint8_t severity;      // 0-10
  const char* description;
  const char* affectedVersions;
  const char* exploitation;
};

class VulnerabilityMatcher {
public:
  static VulnerabilityMatcher& getInstance() {
    static VulnerabilityMatcher instance;
    return instance;
  }

  // Find vulnerabilities for identified device
  std::vector<VulnerabilityEntry> findVulnerabilities(const DeviceSignature* device);
  std::vector<VulnerabilityEntry> findByCVE(const char* cveId);
  std::vector<VulnerabilityEntry> findBySeverity(uint8_t minSeverity);

  // Risk assessment
  uint8_t calculateRiskScore(const DeviceSignature* device, uint16_t openPorts);
  const char* getRiskLevel(uint8_t score);

  void printVulnerabilityReport(const DeviceSignature* device);

private:
  VulnerabilityMatcher() {}

  std::vector<VulnerabilityEntry> vulnerabilityDB;

  void initializeVulnerabilityDB();
};

#endif
