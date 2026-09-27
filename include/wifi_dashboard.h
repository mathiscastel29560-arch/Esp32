#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <WebServer.h>

// ============= WiFi DASHBOARD SERVER =============
class WiFiDashboard {
public:
  static WiFiDashboard& getInstance() {
    static WiFiDashboard instance;
    return instance;
  }

  // Initialize dashboard server
  bool begin(uint16_t port = 80);

  // Stop server
  void stop();

  // Update handler (call in main loop)
  void handleClient();

  // Check if running
  bool isRunning() const { return running; }

  // Get server URL
  std::string getURL() const;

  // Get access details
  void printAccessInfo() const;

private:
  WiFiDashboard() = default;

  bool running = false;
  uint16_t serverPort = 80;
  WebServer* server = nullptr;

  // HTTP handlers
  void handleRoot();
  void handleAPIStatus();
  void handleAPIHistory();
  void handleAPIStats();
  void handleAPIAudits();
  void handleAPIControl();
  void handleNotFound();

  // HTML/CSS/JS generation
  std::string generateHTML() const;
  std::string generateCSS() const;
  std::string generateJS() const;

  // JSON generation
  std::string generateStatusJSON() const;
  std::string generateHistoryJSON() const;
  std::string generateStatsJSON() const;
};

#endif // WIFI_DASHBOARD_H
