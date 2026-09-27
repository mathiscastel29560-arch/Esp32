#ifndef ANOMALY_DETECTOR_H
#define ANOMALY_DETECTOR_H

#include <Arduino.h>
#include <vector>

// ============= ANOMALY DETECTION SYSTEM =============

enum class DetectionThreat {
  NONE = 0,
  MINOR = 1,           // 1-2 suspicious indicators
  MODERATE = 2,        // 3-4 indicators (consider evasion)
  SEVERE = 3,          // 5+ indicators (abort and evade)
  CRITICAL = 4         // IDS/IPS actively blocking
};

class AnomalyDetector {
public:
  static AnomalyDetector& getInstance() {
    static AnomalyDetector instance;
    return instance;
  }

  void recordPacketTimeout(uint32_t expectedMs, uint32_t actualMs);
  void recordUnexpectedReset();
  void recordAckMismatch(uint16_t expectedSeq, uint16_t receivedSeq);
  void recordRateLimitResponse();
  void recordMACBlacklisting();
  void recordChannelBlocking();
  void recordPacketDrop(uint16_t sequenceGap);

  DetectionThreat getCurrentThreat() const;
  bool shouldEvade() const { return getCurrentThreat() >= DetectionThreat::MODERATE; }
  bool shouldAbort() const { return getCurrentThreat() >= DetectionThreat::SEVERE; }

  uint16_t getAnomalyScore() const { return anomalyScore; }
  float getDetectionProbability() const;

  void reset();
  void printStatus();

private:
  AnomalyDetector() : anomalyScore(0), lastUpdateTime(0) {}

  struct AnomalyEvent {
    uint32_t timestamp;
    uint8_t severity;     // 1-10
    const char* type;
  };

  std::vector<AnomalyEvent> events;
  uint16_t anomalyScore;
  uint32_t lastUpdateTime;

  void updateAnomalyScore();
  void decayOldEvents();
};

// ============= STEALTH ADAPTATION SYSTEM =============

class StealthAdapter {
public:
  static StealthAdapter& getInstance() {
    static StealthAdapter instance;
    return instance;
  }

  void adaptToThreat(DetectionThreat threat);

  uint32_t getAdaptiveDelay() const { return adaptiveDelay; }
  uint8_t getChannelHopInterval() const { return channelHopInterval; }
  bool shouldRandomizeMAC() const { return randomizeMAC; }
  bool shouldReducePayloadSize() const { return reducePayloadSize; }
  bool shouldAddDecoyTraffic() const { return addDecoyTraffic; }

  void printAdaptiveConfig();

private:
  StealthAdapter() : adaptiveDelay(100), channelHopInterval(500),
                    randomizeMAC(false), reducePayloadSize(false),
                    addDecoyTraffic(false) {}

  uint32_t adaptiveDelay;
  uint8_t channelHopInterval;
  bool randomizeMAC;
  bool reducePayloadSize;
  bool addDecoyTraffic;
};

#endif
