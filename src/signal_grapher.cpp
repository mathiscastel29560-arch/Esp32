#include "signal_grapher.h"
#include "debug_logger.h"
#include <cmath>
#include <algorithm>

SignalGrapher::SignalGrapher() {
  memset(readings, 0, sizeof(readings));
}

void SignalGrapher::addRSSIReading(int8_t rssi) {
  if (readingCount < MAX_READINGS) {
    readings[readingCount].timestamp = millis();
    readings[readingCount].rssi = rssi;
    readings[readingCount].signalPercent = rssiToPercent(rssi);
    readingCount++;
  } else {
    // Ring buffer: overwrite oldest
    readings[nextIndex].timestamp = millis();
    readings[nextIndex].rssi = rssi;
    readings[nextIndex].signalPercent = rssiToPercent(rssi);
    nextIndex = (nextIndex + 1) % MAX_READINGS;
  }
}

void SignalGrapher::addWiFiDevice(const char* ssid, int8_t rssi, uint16_t channel) {
  // Check if already exists
  for (auto& dev : wifiDevices) {
    if (strcmp(dev.ssid, ssid) == 0) {
      dev.rssi = rssi;
      dev.lastSeen = millis();
      return;
    }
  }

  // New device
  WiFiRecord rec;
  strncpy(rec.ssid, ssid, sizeof(rec.ssid) - 1);
  rec.ssid[sizeof(rec.ssid) - 1] = '\0';
  rec.rssi = rssi;
  rec.channel = channel;
  rec.lastSeen = millis();

  wifiDevices.push_back(rec);
}

void SignalGrapher::addBLEDevice(const char* address, int8_t rssi) {
  for (auto& dev : bleDevices) {
    if (strcmp(dev.address, address) == 0) {
      dev.rssi = rssi;
      dev.lastSeen = millis();
      return;
    }
  }

  BLERecord rec;
  strncpy(rec.address, address, sizeof(rec.address) - 1);
  rec.address[sizeof(rec.address) - 1] = '\0';
  rec.rssi = rssi;
  rec.lastSeen = millis();

  bleDevices.push_back(rec);
}

std::vector<SignalGrapher::DataPoint> SignalGrapher::getRecentReadings(uint16_t count) const {
  std::vector<DataPoint> result;

  uint16_t start = (readingCount > count) ? (readingCount - count) : 0;
  for (uint16_t i = start; i < readingCount; i++) {
    result.push_back(readings[i]);
  }
  return result;
}

void SignalGrapher::printASCIIGraph(const char* title, uint16_t width, uint16_t height) const {
  if (readingCount == 0) {
    Serial.println("No data to graph.");
    return;
  }

  Serial.printf("\n╔════════ GRAPH: %s ════════╗\n", title);

  int8_t maxRssi = getMaxRSSI();
  int8_t minRssi = getMinRSSI();
  int8_t range = maxRssi - minRssi;
  if (range == 0) range = 1;

  // Get recent readings
  auto recent = getRecentReadings(width);

  // Draw graph from top to bottom
  for (int h = height; h > 0; h--) {
    Serial.print("║ ");

    int threshold = minRssi + (range * h / height);

    for (size_t w = 0; w < recent.size(); w++) {
      if (recent[w].rssi >= threshold) {
        Serial.print("█");
      } else {
        Serial.print("░");
      }
    }

    Serial.printf(" %d dBm\n", threshold);
  }

  Serial.print("║ ");
  for (size_t i = 0; i < recent.size(); i++) {
    Serial.print("-");
  }
  Serial.println(" ┘");

  // Legend
  Serial.printf("║ Min: %d dBm | Max: %d dBm | Avg: %d dBm | Count: %u\n",
    minRssi, maxRssi, getAvgRSSI(), readingCount);
  Serial.println("╚════════════════════════════╝");
  Serial.println("");
}

void SignalGrapher::printSignalHistogram() const {
  if (readingCount == 0) {
    Serial.println("No data available.");
    return;
  }

  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     SIGNAL STRENGTH DISTRIBUTION       ║");
  Serial.println("╚════════════════════════════════════════╝");

  // Buckets: -30 to -100 dBm
  int buckets[8] = {0};  // -30, -40, -50, -60, -70, -80, -90, -100

  for (uint16_t i = 0; i < readingCount; i++) {
    int8_t rssi = readings[i].rssi;
    int bucket = (-rssi - 30) / 10;
    if (bucket >= 0 && bucket < 8) buckets[bucket]++;
  }

  // Print histogram
  for (int i = 0; i < 8; i++) {
    int8_t rssiLevel = -30 - (i * 10);
    float percent = (float)buckets[i] * 100.0f / readingCount;

    Serial.printf("%d dBm │", rssiLevel);

    // Bar
    int barLen = (int)(percent / 2);
    for (int j = 0; j < barLen; j++) Serial.print("█");
    for (int j = barLen; j < 25; j++) Serial.print(" ");

    Serial.printf("│ %3.0f%% (%d)\n", percent, buckets[i]);
  }

  Serial.println("");
}

void SignalGrapher::printChannelHeatmap() const {
  if (wifiDevices.empty()) {
    Serial.println("No WiFi devices detected.");
    return;
  }

  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     WiFi CHANNEL HEATMAP               ║");
  Serial.println("╚════════════════════════════════════════╝");

  // Channels 1-13
  int channelStats[14] = {0};  // index 1-13
  int channelMax[14] = {-120};

  for (auto& dev : wifiDevices) {
    if (dev.channel >= 1 && dev.channel <= 13) {
      channelStats[dev.channel]++;
      if (dev.rssi > channelMax[dev.channel]) {
        channelMax[dev.channel] = dev.rssi;
      }
    }
  }

  Serial.print("Ch │");
  for (int i = 1; i <= 13; i++) {
    Serial.printf(" %2d", i);
  }
  Serial.println(" │");

  Serial.print("───┼");
  for (int i = 1; i <= 13; i++) Serial.print("────");
  Serial.println("┤");

  Serial.print("# │");
  for (int i = 1; i <= 13; i++) {
    Serial.printf(" %2d", channelStats[i]);
  }
  Serial.println(" │");

  Serial.print("dBm│");
  for (int i = 1; i <= 13; i++) {
    Serial.printf(" %2d", channelMax[i]);
  }
  Serial.println(" │");

  Serial.println("");
}

int8_t SignalGrapher::getMaxRSSI() const {
  int8_t max = -120;
  for (uint16_t i = 0; i < readingCount; i++) {
    if (readings[i].rssi > max) max = readings[i].rssi;
  }
  return max;
}

int8_t SignalGrapher::getMinRSSI() const {
  int8_t min = 0;
  for (uint16_t i = 0; i < readingCount; i++) {
    if (readings[i].rssi < min) min = readings[i].rssi;
  }
  return min;
}

int8_t SignalGrapher::getAvgRSSI() const {
  if (readingCount == 0) return 0;

  int32_t sum = 0;
  for (uint16_t i = 0; i < readingCount; i++) {
    sum += readings[i].rssi;
  }
  return sum / readingCount;
}

void SignalGrapher::clearData() {
  readingCount = 0;
  nextIndex = 0;
  wifiDevices.clear();
  bleDevices.clear();
  memset(readings, 0, sizeof(readings));
}

char SignalGrapher::getRSSIChar(int8_t rssi) const {
  if (rssi >= -30) return '█';        // Excellent
  if (rssi >= -50) return '▓';        // Good
  if (rssi >= -70) return '▒';        // Fair
  if (rssi >= -85) return '░';        // Weak
  return '·';                          // Very weak
}

uint8_t SignalGrapher::rssiToPercent(int8_t rssi) const {
  // Convert -30 (excellent) to -100 (useless) to 0-100%
  if (rssi >= -30) return 100;
  if (rssi <= -100) return 0;
  return (uint8_t)(((rssi + 100) * 100) / 70);
}

const char* SignalGrapher::getRSSILabel(int8_t rssi) const {
  if (rssi >= -30) return "Excellent";
  if (rssi >= -50) return "Good";
  if (rssi >= -70) return "Fair";
  if (rssi >= -85) return "Weak";
  return "Very Weak";
}

std::string SignalGrapher::getSummaryString() const {
  char summary[128];
  snprintf(summary, sizeof(summary),
    "Signal: %d dBm avg | Range: %d to %d | Readings: %u",
    getAvgRSSI(), getMinRSSI(), getMaxRSSI(), readingCount
  );
  return std::string(summary);
}
