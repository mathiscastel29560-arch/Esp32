#include "captive_portal_detector.h"
#include <LittleFS.h>

namespace {
constexpr uint32_t SCAN_DELAY_MS = 50;
constexpr uint32_t CONNECTION_CHECK_DELAY_MS = 100;
constexpr uint32_t HTTP_TEST_DELAY_MS = 100;
constexpr uint32_t LOGIN_ATTEMPT_DELAY_MS = 100;
constexpr uint32_t PORTAL_FETCH_DELAY_MS = 50;
constexpr int HTTP_OK = 200;
constexpr int HTTP_REDIRECT = 302;
constexpr int HTTP_REDIRECT_TEMP = 307;
constexpr int HTTP_MOVED_PERM = 301;
constexpr int HTTP_SUCCESS_MAX = 400;
constexpr int TEST_CREDENTIALS_COUNT = 4;
constexpr size_t LOG_ENTRY_BUF_SIZE = 256;
constexpr int HTML_TAG_LENGTH = 7; // length of "</title>"
const char* PORTAL_LOG_FILE = "/logs/handshakes/captive_portals.csv";
const char* PORTAL_LOG_DIR = "/logs/handshakes";
const char* TEST_CONNECTIVITY_URL = "http://google.com";
}

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

    delay(SCAN_DELAY_MS);
  }

  result.success = result.portalsFound > 0;
  result.logFile = PORTAL_LOG_FILE;

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
    delay(CONNECTION_CHECK_DELAY_MS);
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

  // Comprehensive captive portal detection URLs (real world tests)
  const char* testUrls[] = {
    "http://captive.apple.com/hotspot-detect.html",      // Apple
    "http://msftncsi.com/ncsi.txt",                       // Microsoft
    "http://clients3.google.com/generate_204",            // Google
    "http://connectivity-check.ubuntu.com/",              // Ubuntu
    "http://example.com/",                                // Generic
    "http://detectportal.firefox.com/success.txt",        // Firefox
    "http://httpbin.org/status/200"                       // Generic HTTP test
  };

  for (const char* testUrl : testUrls) {
    HTTPClient http;
    http.setConnectTimeout(3000);
    http.setTimeout(5000);
    http.begin(testUrl);

    int httpCode = http.GET();

    if (httpCode == HTTP_REDIRECT || httpCode == HTTP_REDIRECT_TEMP || httpCode == HTTP_OK) {
      portal.portalUrl = testUrl;
      result.portals.push_back(portal);
      result.portalsFound++;
      result.success = true;
      logPortal(portal);
      http.end();
      break;
    }

    http.end();
    delay(HTTP_TEST_DELAY_MS);
  }

  result.logFile = PORTAL_LOG_FILE;
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

  if (httpCode == HTTP_OK) {
    String html = http.getString();

    // Extract form fields and attempt auto-login
    if (html.indexOf("username") >= 0 || html.indexOf("email") >= 0) {
      // Common credentials for testing
      const char* testCredsUsername[] = {"admin", "user", "test", "guest"};
      const char* testCredsPassword[] = {"admin", "password", "test", "123456"};

      for (int i = 0; i < TEST_CREDENTIALS_COUNT; i++) {
        String payload = String("username=") + testCredsUsername[i] +
                        "&password=" + testCredsPassword[i];

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int postCode = http.POST(payload);

        if (postCode == HTTP_OK || postCode == HTTP_REDIRECT) {
          result.success = true;
          break;
        }

        delay(LOGIN_ATTEMPT_DELAY_MS);
      }
    }

    result.portals.push_back(portal);
  }

  http.end();
  result.logFile = PORTAL_LOG_FILE;
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

    if (httpCode == HTTP_OK || httpCode == HTTP_REDIRECT) {
      http.end();
      return url;
    }

    http.end();
    delay(PORTAL_FETCH_DELAY_MS);
  }

  return "";
}

String PortalDetector::getPortalTitle(const String& html) {
  int titleStart = html.indexOf("<title>");
  int titleEnd = html.indexOf("</title>");

  if (titleStart >= 0 && titleEnd > titleStart) {
    return html.substring(titleStart + HTML_TAG_LENGTH, titleEnd);
  }

  return "Unknown";
}

bool PortalDetector::testConnectivity() {
  HTTPClient http;
  http.begin(TEST_CONNECTIVITY_URL);
  int code = http.GET();
  http.end();
  return (code == HTTP_OK || code == HTTP_MOVED_PERM || code == HTTP_REDIRECT);
}

bool PortalDetector::validateRedirect(const String& url) {
  if (url.isEmpty()) return false;

  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  http.end();

  return (code >= HTTP_OK && code < HTTP_SUCCESS_MAX);
}

void PortalDetector::logPortal(const DetectedPortal& portal) {
  if (!LittleFS.begin()) {
    Serial.println("Error: Failed to mount LittleFS");
    return;
  }

  File logFile = LittleFS.open(PORTAL_LOG_FILE, "a");
  if (!logFile) {
    // Create directory if it doesn't exist
    if (!LittleFS.mkdir(PORTAL_LOG_DIR)) {
      Serial.println("Warning: Directory already exists or failed to create");
    }
    // Try opening again
    logFile = LittleFS.open(PORTAL_LOG_FILE, "a");
  }

  if (logFile) {
    char logEntry[LOG_ENTRY_BUF_SIZE];
    snprintf(logEntry, sizeof(logEntry), "%lu,%s,%s,%d\n",
             millis(), portal.ssid.c_str(), portal.portalUrl.c_str(), portal.rssi);
    logFile.print(logEntry);
    logFile.close();  // Always close the file
  } else {
    Serial.println("Error: Failed to open captive portal log file");
  }

  LittleFS.end();
}

void PortalDetector::stop() {
  isRunning_ = false;
}

} // namespace CaptivePortalDetector
