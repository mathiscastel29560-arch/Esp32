#ifndef CHANNEL_ANALYZER_H
#define CHANNEL_ANALYZER_H

#include <Arduino.h>
#include <vector>
#include <map>

namespace ChannelAnalyzer {

struct ChannelScan {
  uint8_t channel;
  int32_t rssi;
  uint32_t networkCount;
  uint32_t packetCount;
  uint32_t interferenceLevel;
};

struct AnalysisConfig {
  bool scan2_4GHz;
  bool scan5GHz;
  uint32_t scanDurationPerChannelMs;
  bool trackInterference;
  bool analyzeQuality;
};

struct AnalysisResult {
  bool success;
  std::vector<ChannelScan> channelData;
  uint8_t bestChannel;
  uint8_t worstChannel;
  uint32_t totalNetworksFound;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class Analyzer {
public:
  Analyzer();
  AnalysisResult analyzeChannels(const AnalysisConfig& config);
  AnalysisResult scanChannel(uint8_t channel, uint32_t durationMs);
  std::vector<ChannelScan> identifyBestChannels();
  uint32_t calculateInterference(const std::vector<ChannelScan>& scans);
  String generateReport(const AnalysisResult& result);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;
  std::map<uint8_t, ChannelScan> channelStats_;

  void updateChannelStats(uint8_t channel, int32_t rssi);
  void logAnalysis(const ChannelScan& scan);
};

} // namespace ChannelAnalyzer

#endif
