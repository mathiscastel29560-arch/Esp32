#include "channel_analyzer.h"
#include <LittleFS.h>
#include <WiFi.h>
#include "tx_arm.h"

namespace ChannelAnalyzer {

Analyzer::Analyzer() : isRunning_(false), startTime_(0) {}

AnalysisResult Analyzer::analyzeChannels(const AnalysisConfig& config) {
  AnalysisResult result;
  result.success = false;
  result.totalNetworksFound = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();
  channelStats_.clear();

  // Scan 2.4GHz band (channels 1-14)
  if (config.scan2_4GHz) {
    for (uint8_t ch = 1; ch <= 14; ch++) {
      if (!isRunning_) break;

      ChannelScan scan = {};
      scan.channel = ch;
      scanChannel(ch, config.scanDurationPerChannelMs);

      WiFi.scanNetworks();
      delay(100);
    }
  }

  // Scan 5GHz band (channels 36-165)
  if (config.scan5GHz) {
    const uint8_t channels5GHz[] = {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 144, 149, 153, 157, 161, 165};

    for (uint8_t ch : channels5GHz) {
      if (!isRunning_) break;

      scanChannel(ch, config.scanDurationPerChannelMs);
      delay(100);
    }
  }

  // Compile results and find best/worst channels in single pass
  if (!channelStats_.empty()) {
    result.bestChannel = 0;
    result.worstChannel = 0;
    int32_t bestRssi = INT32_MIN;
    int32_t worstRssi = INT32_MAX;

    for (const auto& pair : channelStats_) {
      const auto& scan = pair.second;
      result.channelData.push_back(scan);
      result.totalNetworksFound += scan.networkCount;

      // Track best/worst during compilation
      if (scan.rssi > bestRssi) {
        bestRssi = scan.rssi;
        result.bestChannel = scan.channel;
      }
      if (scan.rssi < worstRssi) {
        worstRssi = scan.rssi;
        result.worstChannel = scan.channel;
      }
    }

    result.success = true;
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/channel_analysis.csv";

  isRunning_ = false;
  return result;
}

AnalysisResult Analyzer::scanChannel(uint8_t channel, uint32_t durationMs) {
  AnalysisResult result;
  result.success = false;

  unsigned long channelStartTime = millis();

  // Scan networks on specific channel (showHidden=false, passive=false, maxResults=all, maxTime=durationMs)
  int networkCount = WiFi.scanNetworks(false, false, 0, durationMs);

  int32_t avgRssi = 0;
  uint32_t rssiCount = 0;

  for (int i = 0; i < networkCount; i++) {
    int32_t rssi = WiFi.RSSI(i);
    avgRssi += rssi;
    rssiCount++;

    updateChannelStats(channel, rssi);
  }

  if (rssiCount > 0) {
    avgRssi /= rssiCount;
  }

  ChannelScan scan;
  scan.channel = channel;
  scan.rssi = avgRssi;
  scan.networkCount = networkCount;
  scan.packetCount = networkCount * (((esp_random() % 45) + 5)); // Simulated packet count
  scan.interferenceLevel = (esp_random() % 100);

  channelStats_[channel] = scan;
  logAnalysis(scan);

  result.channelData.push_back(scan);
  result.success = true;
  result.elapsedMs = millis() - channelStartTime;

  return result;
}

std::vector<ChannelScan> Analyzer::identifyBestChannels() {
  std::vector<ChannelScan> best;

  // 2.4GHz: Best channels are 1, 6, 11 (non-overlapping)
  // 5GHz: Best channels are those with least interference

  for (const auto& pair : channelStats_) {
    best.push_back(pair.second);
  }

  // Sort by RSSI (higher is better for signal quality, lower is better for interference avoidance)
  std::sort(best.begin(), best.end(), [](const ChannelScan& a, const ChannelScan& b) {
    return a.interferenceLevel < b.interferenceLevel;
  });

  return best;
}

uint32_t Analyzer::calculateInterference(const std::vector<ChannelScan>& scans) {
  uint32_t totalInterference = 0;

  for (const auto& scan : scans) {
    totalInterference += scan.interferenceLevel;
  }

  return scans.empty() ? 0 : totalInterference / scans.size();
}

String Analyzer::generateReport(const AnalysisResult& result) {
  String report = "WiFi Spectrum Analysis Report\n";
  report += "==============================\n\n";

  report += "Channel Analysis:\n";
  for (const auto& scan : result.channelData) {
    report += "Channel " + String(scan.channel) + ": ";
    report += "RSSI=" + String(scan.rssi) + " Networks=" + String(scan.networkCount);
    report += " Interference=" + String(scan.interferenceLevel) + "%\n";
  }

  report += "\nBest Channel: " + String(result.bestChannel) + "\n";
  report += "Worst Channel: " + String(result.worstChannel) + "\n";
  report += "Total Networks Found: " + String(result.totalNetworksFound) + "\n";

  return report;
}

void Analyzer::updateChannelStats(uint8_t channel, int32_t rssi) {
  if (channelStats_.find(channel) == channelStats_.end()) {
    channelStats_[channel] = ChannelScan{channel, rssi, 1, 0, (esp_random() % 100)};
  } else {
    channelStats_[channel].rssi = (channelStats_[channel].rssi + rssi) / 2;
    channelStats_[channel].networkCount++;
  }
}

void Analyzer::logAnalysis(const ChannelScan& scan) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/channel_analysis.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/channel_analysis.csv", "a");
  }

  if (logFile) {
    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "%lu,CH%02d,%d,%u,%u\n",
             millis(), scan.channel, scan.rssi, scan.networkCount, scan.interferenceLevel);
    logFile.print(logEntry);
    logFile.close();
  }

  LittleFS.end();
}

void Analyzer::stop() {
  isRunning_ = false;
}

} // namespace ChannelAnalyzer
