#include "anomaly_detector.h"
#include "async_logger.h"

void AnomalyDetector::recordPacketTimeout(uint32_t expectedMs, uint32_t actualMs) {
  uint32_t variance = (actualMs > expectedMs) ? (actualMs - expectedMs) : 0;
  uint8_t severity = (variance > expectedMs * 2) ? 8 :
                     (variance > expectedMs) ? 5 : 2;

  events.push_back({millis(), severity, "TIMEOUT"});
  anomalyScore += severity;
  LOG_W("Packet timeout - expected: %ums, actual: %ums, severity: %u", expectedMs, actualMs, severity);
  updateAnomalyScore();
}

void AnomalyDetector::recordUnexpectedReset() {
  events.push_back({millis(), 9, "RESET"});
  anomalyScore += 9;
  LOG_E("Unexpected reset detected - likely IDS trigger");
  updateAnomalyScore();
}

void AnomalyDetector::recordAckMismatch(uint16_t expectedSeq, uint16_t receivedSeq) {
  events.push_back({millis(), 6, "ACK_MISMATCH"});
  anomalyScore += 6;
  LOG_W("ACK mismatch - expected: %u, received: %u", expectedSeq, receivedSeq);
  updateAnomalyScore();
}

void AnomalyDetector::recordRateLimitResponse() {
  events.push_back({millis(), 7, "RATE_LIMIT"});
  anomalyScore += 7;
  LOG_W("Rate limiting detected");
  updateAnomalyScore();
}

void AnomalyDetector::recordMACBlacklisting() {
  events.push_back({millis(), 10, "MAC_BLACKLIST"});
  anomalyScore += 10;
  LOG_E("MAC address blacklisted - immediate evasion required");
  updateAnomalyScore();
}

void AnomalyDetector::recordChannelBlocking() {
  events.push_back({millis(), 8, "CHANNEL_BLOCK"});
  anomalyScore += 8;
  LOG_W("Channel blocking detected");
  updateAnomalyScore();
}

void AnomalyDetector::recordPacketDrop(uint16_t sequenceGap) {
  uint8_t severity = (sequenceGap > 10) ? 8 : (sequenceGap > 5) ? 5 : 3;
  events.push_back({millis(), severity, "PKT_DROP"});
  anomalyScore += severity;
  LOG_W("Packet drop detected - gap: %u packets", sequenceGap);
  updateAnomalyScore();
}

DetectionThreat AnomalyDetector::getCurrentThreat() const {
  if (anomalyScore >= 30) return DetectionThreat::CRITICAL;
  if (anomalyScore >= 20) return DetectionThreat::SEVERE;
  if (anomalyScore >= 10) return DetectionThreat::MODERATE;
  if (anomalyScore >= 3) return DetectionThreat::MINOR;
  return DetectionThreat::NONE;
}

float AnomalyDetector::getDetectionProbability() const {
  float probability = (float)anomalyScore / 50.0f;
  return (probability > 1.0f) ? 1.0f : probability;
}

void AnomalyDetector::updateAnomalyScore() {
  lastUpdateTime = millis();
  decayOldEvents();
}

void AnomalyDetector::decayOldEvents() {
  uint32_t now = millis();
  uint32_t decayWindow = 30000;  // 30 second decay window

  anomalyScore = 0;
  for (auto& evt : events) {
    if (now - evt.timestamp < decayWindow) {
      float decayFactor = 1.0f - ((float)(now - evt.timestamp) / decayWindow);
      anomalyScore += (uint16_t)(evt.severity * decayFactor);
    }
  }

  // Remove very old events
  events.erase(
    std::remove_if(events.begin(), events.end(),
      [now, decayWindow](const AnomalyEvent& e) { return (now - e.timestamp) > decayWindow; }),
    events.end()
  );
}

void AnomalyDetector::reset() {
  anomalyScore = 0;
  events.clear();
  lastUpdateTime = millis();
  LOG_I("Anomaly detector reset");
}

void AnomalyDetector::printStatus() {
  Serial.printf("\n=== ANOMALY DETECTION STATUS ===\n");
  Serial.printf("Anomaly Score: %u/50\n", anomalyScore);
  Serial.printf("Detection Probability: %.1f%%\n", getDetectionProbability() * 100);
  Serial.printf("Current Threat: %u\n", (uint8_t)getCurrentThreat());
  Serial.printf("Recent Events: %u\n", events.size());
  Serial.println("================================\n");
}

void StealthAdapter::adaptToThreat(DetectionThreat threat) {
  switch (threat) {
    case DetectionThreat::NONE:
      adaptiveDelay = 100;
      channelHopInterval = 500;
      randomizeMAC = false;
      reducePayloadSize = false;
      addDecoyTraffic = false;
      break;

    case DetectionThreat::MINOR:
      adaptiveDelay = 200;
      channelHopInterval = 1000;
      randomizeMAC = true;
      reducePayloadSize = false;
      addDecoyTraffic = false;
      LOG_W("Minor threat detected - adapting: increased delays");
      break;

    case DetectionThreat::MODERATE:
      adaptiveDelay = 500;
      channelHopInterval = 500;
      randomizeMAC = true;
      reducePayloadSize = true;
      addDecoyTraffic = true;
      LOG_W("Moderate threat detected - adapting: decoy traffic enabled");
      break;

    case DetectionThreat::SEVERE:
      adaptiveDelay = 1000;
      channelHopInterval = 200;
      randomizeMAC = true;
      reducePayloadSize = true;
      addDecoyTraffic = true;
      LOG_E("Severe threat - heavy evasion enabled");
      break;

    case DetectionThreat::CRITICAL:
      adaptiveDelay = 2000;
      channelHopInterval = 100;
      randomizeMAC = true;
      reducePayloadSize = true;
      addDecoyTraffic = true;
      LOG_E("CRITICAL THREAT - maximum evasion mode");
      break;
  }
}

void StealthAdapter::printAdaptiveConfig() {
  Serial.printf("\n=== STEALTH ADAPTATION CONFIG ===\n");
  Serial.printf("Adaptive Delay: %ums\n", adaptiveDelay);
  Serial.printf("Channel Hop Interval: %ums\n", channelHopInterval);
  Serial.printf("Randomize MAC: %s\n", randomizeMAC ? "YES" : "NO");
  Serial.printf("Reduce Payload Size: %s\n", reducePayloadSize ? "YES" : "NO");
  Serial.printf("Add Decoy Traffic: %s\n", addDecoyTraffic ? "YES" : "NO");
  Serial.println("==================================\n");
}
