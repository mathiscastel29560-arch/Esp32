#ifndef CAPTIVE_PORTAL_DETECTOR_H
#define CAPTIVE_PORTAL_DETECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>

namespace CaptivePortalDetector {

struct PortalConfig {
  char targetSsid[32];
  uint32_t scanTimeoutMs;
  uint32_t connectTimeoutMs;
  bool autoInteract;
  bool autoLogin;
};

struct DetectedPortal {
  String ssid;
  String portalUrl;
  String portalTitle;
  uint32_t rssi;
  String gatewaySsid;
  String redirectUrl;
  bool requiresAuth;
};

struct PortalResult {
  bool success;
  std::vector<DetectedPortal> portals;
  uint32_t portalsFound;
  String logFile;
  String error;
};

class PortalDetector {
public:
  PortalDetector();
  PortalResult scanNetworks(const PortalConfig& config);
  PortalResult detectPortal(const char* ssid, uint32_t timeout);
  PortalResult interactWithPortal(const DetectedPortal& portal);
  bool validateRedirect(const String& url);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  String fetchCaptivePortalUrl(const char* ssid);
  String getPortalTitle(const String& html);
  bool testConnectivity();
  void logPortal(const DetectedPortal& portal);
};

} // namespace CaptivePortalDetector

#endif
