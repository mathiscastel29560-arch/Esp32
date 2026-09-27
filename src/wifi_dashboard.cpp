#include "wifi_dashboard.h"
#include "debug_logger.h"
#include "audit_history.h"
#include "audit_statistics.h"
#include "system_settings.h"
#include <WiFi.h>

bool WiFiDashboard::begin(uint16_t port) {
  if (running) return true;

  serverPort = port;
  server = new WebServer(port);

  // Setup routes
  server->on("/", std::bind(&WiFiDashboard::handleRoot, this));
  server->on("/api/status", std::bind(&WiFiDashboard::handleAPIStatus, this));
  server->on("/api/history", std::bind(&WiFiDashboard::handleAPIHistory, this));
  server->on("/api/stats", std::bind(&WiFiDashboard::handleAPIStats, this));
  server->on("/api/audits", std::bind(&WiFiDashboard::handleAPIAudits, this));
  server->on("/api/control", std::bind(&WiFiDashboard::handleAPIControl, this));
  server->onNotFound(std::bind(&WiFiDashboard::handleNotFound, this));

  server->begin();
  running = true;

  DebugLogger::printf("[WiFiDashboard] Server started on port %u\n", port);
  printAccessInfo();

  return true;
}

void WiFiDashboard::stop() {
  if (server) {
    server->stop();
    delete server;
    server = nullptr;
  }
  running = false;
  DebugLogger::println("[WiFiDashboard] Server stopped");
}

void WiFiDashboard::handleClient() {
  if (running && server) {
    server->handleClient();
  }
}

std::string WiFiDashboard::getURL() const {
  IPAddress ip = WiFi.localIP();
  char url[64];
  snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%u",
    ip[0], ip[1], ip[2], ip[3], serverPort);
  return std::string(url);
}

void WiFiDashboard::printAccessInfo() const {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║        WiFi DASHBOARD ACCESS           ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.printf("🌐 URL: %s\n", getURL().c_str());
  Serial.printf("📍 SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("📶 Signal: %d dBm\n", WiFi.RSSI());
  Serial.printf("🔌 IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.println("\nOpen this URL in your browser to access the dashboard.");
  Serial.println("Features:");
  Serial.println("  • Real-time system status");
  Serial.println("  • Audit history & statistics");
  Serial.println("  • Device control & parameters");
  Serial.println("  • Live signal monitoring");
  Serial.println("");
}

void WiFiDashboard::handleRoot() {
  server->send(200, "text/html", generateHTML().c_str());
}

void WiFiDashboard::handleAPIStatus() {
  server->send(200, "application/json", generateStatusJSON().c_str());
}

void WiFiDashboard::handleAPIHistory() {
  server->send(200, "application/json", generateHistoryJSON().c_str());
}

void WiFiDashboard::handleAPIStats() {
  server->send(200, "application/json", generateStatsJSON().c_str());
}

void WiFiDashboard::handleAPIAudits() {
  auto& history = AuditHistory::getInstance();
  auto records = history.getAllRecords();

  String json = "{\"audits\":[";
  for (size_t i = 0; i < records.size(); i++) {
    if (i > 0) json += ",";
    json += "{\"timestamp\":" + String(records[i].timestamp);
    json += ",\"type\":" + String(records[i].auditType);
    json += ",\"devices\":" + String(records[i].devicesFound);
    json += ",\"rssi\":" + String(records[i].maxRSSI);
    json += ",\"status\":" + String(records[i].status);
    json += "}";
  }
  json += "]}";

  server->send(200, "application/json", json);
}

void WiFiDashboard::handleAPIControl() {
  if (server->method() == HTTP_POST) {
    String param = server->arg("action");
    String value = server->arg("value");

    auto& sys = SystemSettings::getInstance();

    if (param == "brightness") {
      sys.setBrightness(value.toInt());
      sys.saveToNVS();
    } else if (param == "volume") {
      sys.setVolume(value.toInt());
      sys.saveToNVS();
    }

    server->send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
  }
}

void WiFiDashboard::handleNotFound() {
  server->send(404, "application/json", "{\"error\":\"Not found\"}");
}

std::string WiFiDashboard::generateStatusJSON() const {
  auto& sys = SystemSettings::getInstance();

  uint32_t uptime = millis() / 1000;
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t freeSkram = ESP.getFreePsram();

  char json[512];
  snprintf(json, sizeof(json),
    "{"
    "\"uptime\":%u,"
    "\"freeHeap\":%u,"
    "\"freePSRAM\":%u,"
    "\"brightness\":%u,"
    "\"volume\":%u,"
    "\"contrast\":%u,"
    "\"batteryMode\":\"%s\","
    "\"cpuFreq\":\"%s\""
    "}",
    uptime, freeHeap, freeSkram,
    sys.getBrightness(), sys.getVolume(), sys.getContrast(),
    sys.getBatterySavingModeString(),
    sys.getCPUFrequencyString()
  );

  return std::string(json);
}

std::string WiFiDashboard::generateHistoryJSON() const {
  auto& history = AuditHistory::getInstance();
  String json = "{\"count\":" + String(history.getRecordCount()) + "}";
  return json.c_str();
}

std::string WiFiDashboard::generateStatsJSON() const {
  auto& stats = AuditStatistics::getInstance();
  auto auditStats = stats.calculateStats();

  char json[256];
  snprintf(json, sizeof(json),
    "{"
    "\"totalAudits\":%u,"
    "\"successRate\":%u,"
    "\"totalDevices\":%u,"
    "\"avgDevicesPerAudit\":%u,"
    "\"efficiency\":%.2f"
    "}",
    auditStats.totalAudits,
    auditStats.successPercent,
    auditStats.totalDevicesFound,
    auditStats.avgDevicesPerAudit,
    stats.getAuditEfficiency()
  );

  return std::string(json);
}

std::string WiFiDashboard::generateHTML() const {
  return R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Audit Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    .header {
      text-align: center;
      color: white;
      margin-bottom: 30px;
    }
    .header h1 {
      font-size: 2.5em;
      margin-bottom: 10px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 20px;
    }
    .card {
      background: white;
      border-radius: 10px;
      padding: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
      transition: transform 0.3s, box-shadow 0.3s;
    }
    .card:hover {
      transform: translateY(-5px);
      box-shadow: 0 15px 40px rgba(0,0,0,0.3);
    }
    .card h2 {
      font-size: 1.2em;
      color: #667eea;
      margin-bottom: 15px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .stat {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid #eee;
    }
    .stat:last-child { border-bottom: none; }
    .label { color: #666; font-weight: 500; }
    .value {
      font-weight: bold;
      color: #667eea;
      font-size: 1.1em;
    }
    .progress-bar {
      background: #eee;
      border-radius: 10px;
      height: 25px;
      overflow: hidden;
      margin: 10px 0;
    }
    .progress-fill {
      background: linear-gradient(90deg, #667eea, #764ba2);
      height: 100%;
      display: flex;
      align-items: center;
      justify-content: center;
      color: white;
      font-size: 0.8em;
      font-weight: bold;
      transition: width 0.3s;
    }
    .control {
      margin-top: 20px;
      padding-top: 20px;
      border-top: 2px solid #eee;
    }
    .control-item {
      margin: 15px 0;
    }
    .control-item label {
      display: block;
      margin-bottom: 5px;
      color: #666;
      font-weight: 500;
    }
    input[type="range"] {
      width: 100%;
      height: 8px;
      border-radius: 5px;
      background: #ddd;
      outline: none;
      -webkit-appearance: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
    }
    input[type="range"]::-moz-range-thumb {
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
    }
    .button {
      background: #667eea;
      color: white;
      border: none;
      padding: 10px 20px;
      border-radius: 5px;
      cursor: pointer;
      font-weight: bold;
      transition: background 0.3s;
      width: 100%;
      margin-top: 10px;
    }
    .button:hover {
      background: #764ba2;
    }
    .status {
      display: inline-block;
      padding: 5px 12px;
      border-radius: 20px;
      font-size: 0.9em;
      font-weight: bold;
    }
    .status.ok { background: #4caf50; color: white; }
    .status.warning { background: #ff9800; color: white; }
    .status.error { background: #f44336; color: white; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🛰️ ESP32 AUDIT DASHBOARD</h1>
      <p>Real-time monitoring and control</p>
    </div>

    <div class="grid">
      <!-- System Status -->
      <div class="card">
        <h2>📊 System Status</h2>
        <div class="stat">
          <span class="label">Uptime:</span>
          <span class="value" id="uptime">--</span>
        </div>
        <div class="stat">
          <span class="label">Free Memory:</span>
          <span class="value" id="freeHeap">--</span>
        </div>
        <div class="stat">
          <span class="label">PSRAM:</span>
          <span class="value" id="psram">--</span>
        </div>
      </div>

      <!-- Audit Statistics -->
      <div class="card">
        <h2>📈 Audit Stats</h2>
        <div class="stat">
          <span class="label">Total Audits:</span>
          <span class="value" id="totalAudits">--</span>
        </div>
        <div class="stat">
          <span class="label">Success Rate:</span>
          <span class="value" id="successRate">--</span>
        </div>
        <div class="stat">
          <span class="label">Total Devices:</span>
          <span class="value" id="totalDevices">--</span>
        </div>
        <div class="progress-bar">
          <div class="progress-fill" id="successBar" style="width: 0%">0%</div>
        </div>
      </div>

      <!-- Settings Control -->
      <div class="card">
        <h2>⚙️ Settings</h2>
        <div class="control">
          <div class="control-item">
            <label for="brightness">Brightness: <span id="brightValue">--</span>%</label>
            <input type="range" id="brightness" min="0" max="100" value="80">
          </div>
          <div class="control-item">
            <label for="volume">Volume: <span id="volValue">--</span>%</label>
            <input type="range" id="volume" min="0" max="100" value="70">
          </div>
          <button class="button" onclick="updateSettings()">💾 Save</button>
        </div>
      </div>
    </div>

    <!-- History Section -->
    <div class="card" style="margin-top: 20px;">
      <h2>📋 Recent Activity</h2>
      <div id="historyContainer" style="max-height: 300px; overflow-y: auto;">
        <p style="text-align: center; color: #999;">Loading...</p>
      </div>
    </div>
  </div>

  <script>
    // Auto-refresh every 5 seconds
    setInterval(updateDashboard, 5000);

    // Initial load
    updateDashboard();

    function updateDashboard() {
      fetchStatus();
      fetchStats();
      updateSliders();
    }

    function fetchStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('uptime').textContent = formatTime(data.uptime);
          document.getElementById('freeHeap').textContent = (data.freeHeap / 1024).toFixed(1) + ' KB';
          document.getElementById('psram').textContent = (data.freePSRAM / 1024).toFixed(1) + ' KB';
        })
        .catch(e => console.error('Status error:', e));
    }

    function fetchStats() {
      fetch('/api/stats')
        .then(r => r.json())
        .then(data => {
          document.getElementById('totalAudits').textContent = data.totalAudits;
          document.getElementById('successRate').textContent = data.successRate + '%';
          document.getElementById('totalDevices').textContent = data.totalDevices;
          document.getElementById('successBar').style.width = data.successRate + '%';
          document.getElementById('successBar').textContent = data.successRate + '%';
        })
        .catch(e => console.error('Stats error:', e));
    }

    function updateSliders() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('brightness').value = data.brightness;
          document.getElementById('brightValue').textContent = data.brightness;
          document.getElementById('volume').value = data.volume;
          document.getElementById('volValue').textContent = data.volume;
        });
    }

    document.getElementById('brightness').addEventListener('input', (e) => {
      document.getElementById('brightValue').textContent = e.target.value;
    });

    document.getElementById('volume').addEventListener('input', (e) => {
      document.getElementById('volValue').textContent = e.target.value;
    });

    function updateSettings() {
      const brightness = document.getElementById('brightness').value;
      const volume = document.getElementById('volume').value;

      fetch('/api/control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'action=brightness&value=' + brightness
      }).then(() => {
        fetch('/api/control', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'action=volume&value=' + volume
        }).then(() => {
          alert('⚙️ Settings saved!');
          updateDashboard();
        });
      });
    }

    function formatTime(seconds) {
      const h = Math.floor(seconds / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      const s = seconds % 60;
      return `${h}h ${m}m ${s}s`;
    }
  </script>
</body>
</html>
)HTML";
}
