#include "captive_portal_detector.h"
#include <LittleFS.h>

namespace CaptivePortalDetector {

PortalDetector::PortalDetector() : isRunning_(false), startTime_(0) {}

PortalResult PortalDetector::scanNetworks(const PortalConfig& config) {
  PortalResult result;
  result.success = false;
  result.portalsFound = 0;

  isRunning_ = true;
  startTime_ = millis();

  // Scan for WiFi networks
  int networkCount = WiFi.scanNetworks();

  for (int i = 0; i < networkCount && isRunning_; i++) {
    if (millis() - startTime_ > config.scanTimeoutMs) {
      break;
    }

    String ssid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);

    // Detect captive portal characteristics
    // Common patterns: "Free WiFi", "Hotel", "Airport", "Guest"
    if (ssid.indexOf("Free") >= 0 || ssid.indexOf("Guest") >= 0 ||
        ssid.indexOf("Hotel") >= 0 || ssid.indexOf("Airport") >= 0 ||
        ssid.indexOf("Public") >= 0 || ssid.indexOf("WiFi") >= 0) {

      DetectedPortal portal;
      portal.ssid = ssid;
      portal.rssi = rssi;
      portal.requiresAuth = true;
      portal.portalUrl = fetchCaptivePortalUrl(ssid.c_str());

      if (!portal.portalUrl.isEmpty()) {
        result.portals.push_back(portal);
        result.portalsFound++;
        logPortal(portal);
      }
    }

    delay(50);
  }

  result.success = result.portalsFound > 0;
  result.logFile = "/logs/handshakes/captive_portals.csv";

  isRunning_ = false;
  return result;
}

PortalResult PortalDetector::detectPortal(const char* ssid, uint32_t timeout) {
  PortalResult result;
  result.success = false;

  if (!ssid) {
    result.error = "SSID is null";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  // Connect to network
  WiFi.begin(ssid);

  uint32_t connectionTimeout = timeout;
  while (!WiFi.isConnected() && (millis() - startTime_) < connectionTimeout) {
    delay(100);
  }

  if (!WiFi.isConnected()) {
    result.error = "Failed to connect";
    isRunning_ = false;
    return result;
  }

  // Test connectivity to detect portal
  DetectedPortal portal;
  portal.ssid = ssid;
  portal.rssi = WiFi.RSSI();

  // Common captive portal detection URLs
  const char* testUrls[] = {
    "http://captive.apple.com/hotspot-detect.html",
    "http://msftncsi.com/ncsi.txt",
    "http://clients3.google.com/generate_204"
  };

  for (const char* testUrl : testUrls) {
    HTTPClient http;
    http.begin(testUrl);
    int httpCode = http.GET();

    if (httpCode == 302 || httpCode == 307 || httpCode == 200) {
      String location = http.getHeader("Location");
      if (!location.isEmpty()) {
        portal.redirectUrl = location;
        portal.portalUrl = location;
        result.portals.push_back(portal);
        result.portalsFound++;
        result.success = true;
        logPortal(portal);
        break;
      }
    }

    http.end();
    delay(100);
  }

  result.logFile = "/logs/handshakes/captive_portals.csv";
  isRunning_ = false;
  return result;
}

PortalResult PortalDetector::interactWithPortal(const DetectedPortal& portal) {
  PortalResult result;
  result.success = false;

  if (portal.portalUrl.isEmpty()) {
    result.error = "Portal URL is empty";
    return result;
  }

  HTTPClient http;
  http.begin(portal.portalUrl);

  // Fetch portal page
  int httpCode = http.GET();

  if (httpCode == 200) {
    String html = http.getString();

    // Extract form fields and attempt auto-login
    if (html.indexOf("username") >= 0 || html.indexOf("email") >= 0) {
      // Common credentials for testing
      const char* testCredsUsername[] = {"admin", "user", "test", "guest"};
      const char* testCredsPassword[] = {"admin", "password", "test", "123456"};

      for (int i = 0; i < 4; i++) {
        String payload = String("username=") + testCredsUsername[i] +
                        "&password=" + testCredsPassword[i];

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int postCode = http.POST(payload);

        if (postCode == 200 || postCode == 302) {
          result.success = true;
          break;
        }

        delay(100);
      }
    }

    result.portals.push_back(portal);
  }

  http.end();
  result.logFile = "/logs/handshakes/captive_portals.csv";
  return result;
}

String PortalDetector::fetchCaptivePortalUrl(const char* ssid) {
  HTTPClient http;

  // Try common portal URLs based on SSID hints
  String urls[] = {
    String("http://192.168.1.1/"),
    String("http://10.0.0.1/"),
    String("http://192.168.0.1/"),
    String("http://captive.local/")
  };

  for (const String& url : urls) {
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200 || httpCode == 302) {
      http.end();
      return url;
    }

    http.end();
    delay(50);
  }

  return "";
}

String PortalDetector::getPortalTitle(const String& html) {
  int titleStart = html.indexOf("<title>");
  int titleEnd = html.indexOf("</title>");

  if (titleStart >= 0 && titleEnd > titleStart) {
    return html.substring(titleStart + 7, titleEnd);
  }

  return "Unknown";
}

bool PortalDetector::testConnectivity() {
  HTTPClient http;
  http.begin("http://google.com");
  int code = http.GET();
  http.end();
  return (code == 200 || code == 301 || code == 302);
}

bool PortalDetector::validateRedirect(const String& url) {
  if (url.isEmpty()) return false;

  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  http.end();

  return (code >= 200 && code < 400);
}

void PortalDetector::logPortal(const DetectedPortal& portal) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/captive_portals.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/captive_portals.csv", "a");
  }

  if (logFile) {
    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "%lu,%s,%s,%d\n",
             millis(), portal.ssid.c_str(), portal.portalUrl.c_str(), portal.rssi);
    logFile.print(logEntry);
    logFile.close();
  }

  LittleFS.end();
}

void PortalDetector::stop() {
  isRunning_ = false;
}

} // namespace CaptivePortalDetector
