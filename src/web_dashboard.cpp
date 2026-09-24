#include "web_dashboard.h"
#include <WiFi.h>

namespace WebDashboard {

static WebServer* server = nullptr;
static DashboardConfig config;
static SystemMetrics currentMetrics;
static bool serverRunning = false;

// HTML dashboard template (simplified HTML-only for compilation safety)
const char DASHBOARD_HTML[] = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Security Platform Dashboard</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #1a1a1a, #2d2d2d);
            color: #00ff00;
            padding: 20px;
        }
        .container { max-width: 1200px; margin: 0 auto; }
        .header { text-align: center; margin-bottom: 30px; }
        h1 { color: #00ff00; text-shadow: 0 0 10px #00ff00; }
        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .metric-card {
            background: rgba(0, 255, 0, 0.05);
            border: 2px solid #00ff00;
            border-radius: 8px;
            padding: 15px;
            box-shadow: 0 0 15px rgba(0, 255, 0, 0.2);
        }
        .metric-label { font-size: 12px; color: #00aa00; }
        .metric-value { font-size: 24px; font-weight: bold; margin: 10px 0; }
        .progress-bar {
            width: 100%;
            height: 20px;
            background: rgba(0, 0, 0, 0.5);
            border: 1px solid #00ff00;
            border-radius: 4px;
            overflow: hidden;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #00ff00, #00aa00);
            transition: width 0.3s;
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
            animation: pulse 2s infinite;
        }
        .status-online { background: #00ff00; }
        .status-offline { background: #ff0000; }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .tool-list { margin-top: 20px; }
        .tool-item {
            background: rgba(0, 100, 0, 0.2);
            border-left: 3px solid #00ff00;
            padding: 10px;
            margin: 5px 0;
            border-radius: 4px;
        }
        button {
            background: #00ff00;
            color: #000;
            border: none;
            padding: 10px 20px;
            margin: 5px;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            transition: all 0.3s;
        }
        button:hover { background: #00aa00; box-shadow: 0 0 10px #00ff00; }
        .log-container {
            background: rgba(0, 0, 0, 0.8);
            border: 1px solid #00ff00;
            border-radius: 4px;
            padding: 10px;
            max-height: 300px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 12px;
        }
        .log-entry { margin: 5px 0; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🛡️ ESP32 Security Platform Dashboard</h1>
            <p>Real-time Monitoring & Tool Control</p>
        </div>

        <div class="metrics-grid" id="metrics-container">
            <div class="metric-card">
                <div class="metric-label">Device Status</div>
                <div class="metric-value">
                    <span class="status-indicator status-online"></span>
                    <span id="device-status">Online</span>
                </div>
            </div>

            <div class="metric-card">
                <div class="metric-label">Memory Usage</div>
                <div class="metric-value"><span id="heap-percent">0</span>%</div>
                <div class="progress-bar">
                    <div class="progress-fill" id="heap-bar" style="width: 0%"></div>
                </div>
            </div>

            <div class="metric-card">
                <div class="metric-label">WiFi Signal</div>
                <div class="metric-value"><span id="wifi-signal">0</span>%</div>
                <div class="progress-bar">
                    <div class="progress-fill" id="wifi-bar" style="width: 0%"></div>
                </div>
            </div>

            <div class="metric-card">
                <div class="metric-label">Battery Level</div>
                <div class="metric-value"><span id="battery">0</span>%</div>
                <div class="progress-bar">
                    <div class="progress-fill" id="battery-bar" style="width: 0%"></div>
                </div>
            </div>

            <div class="metric-card">
                <div class="metric-label">Uptime</div>
                <div class="metric-value" id="uptime">0s</div>
            </div>

            <div class="metric-card">
                <div class="metric-label">CPU Temperature</div>
                <div class="metric-value"><span id="cpu-temp">0</span>°C</div>
            </div>
        </div>

        <div class="tool-list">
            <h3>Active Tools</h3>
            <div id="tools-container"></div>
        </div>

        <div style="margin-top: 20px;">
            <button onclick="refreshMetrics()">🔄 Refresh</button>
            <button onclick="toggleAutoUpdate()">⏸️ Auto Update</button>
        </div>

        <div class="log-container" id="log-container">
            <div class="log-entry">Dashboard initialized...</div>
        </div>
    </div>

    <script>
        let autoUpdate = true;
        const updateInterval = 2000;

        async function refreshMetrics() {
            try {
                const response = await fetch('/api/metrics');
                const metrics = await response.json();

                document.getElementById('heap-percent').textContent = metrics.heapUsagePercent;
                document.getElementById('heap-bar').style.width = metrics.heapUsagePercent + '%';

                document.getElementById('wifi-signal').textContent = metrics.wifiSignal;
                document.getElementById('wifi-bar').style.width = metrics.wifiSignal + '%';

                document.getElementById('battery').textContent = metrics.batteryPercent;
                document.getElementById('battery-bar').style.width = metrics.batteryPercent + '%';

                document.getElementById('cpu-temp').textContent = metrics.cpuTemp.toFixed(1);

                const uptime = metrics.uptime;
                const hours = Math.floor(uptime / 3600);
                const minutes = Math.floor((uptime % 3600) / 60);
                const seconds = uptime % 60;
                document.getElementById('uptime').textContent = `${hours}h ${minutes}m ${seconds}s`;

                addLog('Metrics updated');
            } catch (e) {
                addLog('Error fetching metrics: ' + e);
            }
        }

        function toggleAutoUpdate() {
            autoUpdate = !autoUpdate;
            if (autoUpdate) {
                document.querySelector('button:nth-of-type(2)').textContent = '⏸️ Auto Update';
            } else {
                document.querySelector('button:nth-of-type(2)').textContent = '▶️ Auto Update';
            }
        }

        function addLog(message) {
            const logContainer = document.getElementById('log-container');
            const entry = document.createElement('div');
            entry.className = 'log-entry';
            const timestamp = new Date().toLocaleTimeString();
            entry.textContent = `[${timestamp}] ${message}`;
            logContainer.appendChild(entry);
            logContainer.scrollTop = logContainer.scrollHeight;

            if (logContainer.children.length > 100) {
                logContainer.removeChild(logContainer.firstChild);
            }
        }

        setInterval(() => {
            if (autoUpdate) {
                refreshMetrics();
            }
        }, updateInterval);

        refreshMetrics();
    </script>
</body>
</html>
)html";

void initDashboard(const DashboardConfig& cfg) {
    config = cfg;
    currentMetrics = {0, 0, 0, 0, 0, 0, 0};

    if (server == nullptr) {
        server = new WebServer(config.port);

        server->on("/", handleDashboardRequest);
        server->on("/api/metrics", handleMetricsJSON);
        server->on("/api/tools", handleToolStatusJSON);
    }
}

void startServer() {
    if (server == nullptr) {
        DashboardConfig defaultConfig{80, "", "", false};
        initDashboard(defaultConfig);
    }

    server->begin();
    serverRunning = true;

    Serial.printf("✓ Web Dashboard started on http://%s:%u\n",
                 WiFi.localIP().toString().c_str(), config.port);
}

void stopServer() {
    if (server != nullptr) {
        server->stop();
        serverRunning = false;
        Serial.println("✓ Web Dashboard stopped");
    }
}

bool isServerRunning() {
    return serverRunning;
}

String getServerURL() {
    if (!serverRunning) return "";
    return String("http://") + WiFi.localIP().toString() + ":" + String(config.port);
}

void handleDashboardRequest() {
    server->send(200, "text/html", DASHBOARD_HTML);
}

void handleMetricsJSON() {
    String json = "{\"freeHeap\":" + String(currentMetrics.freeHeap) +
                  ",\"totalHeap\":" + String(currentMetrics.totalHeap) +
                  ",\"heapUsagePercent\":" + String(currentMetrics.heapUsagePercent) +
                  ",\"cpuTemp\":" + String(currentMetrics.cpuTemp) +
                  ",\"wifiSignal\":" + String(currentMetrics.wifiSignal) +
                  ",\"uptime\":" + String(currentMetrics.uptime) +
                  ",\"batteryPercent\":" + String(currentMetrics.batteryPercent) +
                  ",\"timestamp\":" + String(millis()) + "}";

    server->send(200, "application/json", json);
}

void handleToolStatusJSON() {
    String json = "{\"tools\":[";
    json += "{\"name\":\"WiFi Sniffer\",\"status\":\"idle\",\"lastRun\":0},";
    json += "{\"name\":\"Spectrum Analyzer\",\"status\":\"idle\",\"lastRun\":0}";
    json += "]}";

    server->send(200, "application/json", json);
}

void updateMetrics(const SystemMetrics& metrics) {
    currentMetrics = metrics;

    if (serverRunning && server != nullptr) {
        String json = "{\"heapUsagePercent\":" + String(metrics.heapUsagePercent) +
                      ",\"wifiSignal\":" + String(metrics.wifiSignal) +
                      ",\"batteryPercent\":" + String(metrics.batteryPercent) + "}";
        broadcastMetricUpdate(json);
    }
}

void broadcastMetricUpdate(const String& metricsJSON) {
    if (server != nullptr) {
        server->sendContent(metricsJSON);
    }
}

} // namespace WebDashboard
