#ifndef WIFI_PACKET_INJECTION_H
#define WIFI_PACKET_INJECTION_H

#include <Arduino.h>
#include <vector>
#include <cstring>

namespace WifiPacketInjection {

enum FrameType {
  BEACON,
  PROBE_REQUEST,
  PROBE_RESPONSE,
  AUTH_REQUEST,
  AUTH_RESPONSE,
  ASSOC_REQUEST,
  ASSOC_RESPONSE,
  DATA_FRAME,
  NULL_FRAME
};

struct InjectionConfig {
  FrameType frameType;
  char targetBssid[18];
  char sourceMac[18];
  uint8_t targetChannel;
  uint32_t packetsPerSec;
  uint32_t durationMs;
  bool fuzzPayload;
  bool randomizeSequence;
};

struct InjectionResult {
  bool success;
  uint32_t packetsSent;
  uint32_t fuzzingVectorsUsed;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class PacketInjector {
public:
  PacketInjector();
  InjectionResult injectBeacon(const InjectionConfig& config);
  InjectionResult injectProbe(const InjectionConfig& config);
  InjectionResult injectAuth(const InjectionConfig& config);
  InjectionResult injectAssoc(const InjectionConfig& config);
  InjectionResult fuzzFrames(const InjectionConfig& config);
  std::vector<uint8_t> buildFrame(FrameType type, const uint8_t* bssid);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  uint32_t seqNum_;
  unsigned long startTime_;

  void sendRawFrame(const uint8_t* frame, uint32_t len);
  std::vector<uint8_t> generateFuzzVector();
  void logInjection(FrameType type, uint32_t count);
};

} // namespace WifiPacketInjection

#endif
