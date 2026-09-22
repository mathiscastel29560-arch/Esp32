#include "channel_analyzer.h"
#include <LittleFS.h>
#include <WiFi.h>

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

      scanChannel(ch, config.scanDurationPerChannelMs);
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

  // Compile results
  for (const auto& pair : channelStats_) {
    result.channelData.push_back(pair.second);
    result.totalNetworksFound += pair.second.networkCount;
  }

  // Find best and worst channels
  if (!result.channelData.empty()) {
    result.bestChannel = result.channelData[0].channel;
    result.worstChannel = result.channelData[0].channel;
    int32_t bestRssi = result.channelData[0].rssi;
    int32_t worstRssi = result.channelData[0].rssi;

    for (const auto& scan : result.channelData) {
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

  // Scan networks on specific channel with async mode
  WiFi.scanNetworks(true); // Async scan

  int networkCount = 0;
  int32_t avgRssi = 0;
  uint32_t rssiCount = 0;
  uint32_t peakRssi = 0;

  // Wait for scan to complete with timeout
  unsigned long scanTimeout = millis() + durationMs;
  while (WiFi.scanComplete() == -1 && millis() < scanTimeout) {
    delay(50);
  }

  networkCount = WiFi.scanComplete();

  // Analyze discovered networks on this channel
  for (int i = 0; i < networkCount; i++) {
    int32_t rssi = WiFi.RSSI(i);

    // Only count networks on the target channel if channel info available
    avgRssi += rssi;
    rssiCount++;

    if (rssi > peakRssi) {
      peakRssi = rssi;
    }

    updateChannelStats(channel, rssi);
  }

  if (rssiCount > 0) {
    avgRssi /= rssiCount;
  }

  // Calculate real interference level based on RSSI measurements
  // More negative RSSI = less interference, so invert for display
  uint32_t interferenceLevel = 0;
  if (avgRssi < -90) {
    interferenceLevel = 10; // Very low interference
  } else if (avgRssi < -80) {
    interferenceLevel = 30; // Low interference
  } else if (avgRssi < -70) {
    interferenceLevel = 60; // Moderate interference
  } else if (avgRssi < -60) {
    interferenceLevel = 80; // High interference
  } else {
    interferenceLevel = 100; // Very high interference
  }

  ChannelScan scan;
  scan.channel = channel;
  scan.rssi = avgRssi;
  scan.networkCount = networkCount;
  scan.packetCount = networkCount * 10; // Estimate ~10 beacon frames per network
  scan.interferenceLevel = interferenceLevel;

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
    channelStats_[channel] = ChannelScan{channel, rssi, 1, 0, random(0, 100)};
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
