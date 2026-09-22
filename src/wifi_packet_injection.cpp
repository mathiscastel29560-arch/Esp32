#include "wifi_packet_injection.h"
#include <LittleFS.h>
#include <esp_wifi.h>
#include <WiFi.h>

namespace WifiPacketInjection {

PacketInjector::PacketInjector() : isRunning_(false), seqNum_(0), startTime_(0) {}

void PacketInjector::setupWifiInjectionMode(uint8_t channel) {
  // Initialize WiFi in STA mode for packet injection
  WiFi.mode(WIFI_STA);

  // Disconnect from any AP
  WiFi.disconnect(false); // false = keep RF on

  // Set to specific channel
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  // Enable promiscuous mode for monitoring
  esp_wifi_set_promiscuous(true);
}

InjectionResult PacketInjector::injectBeacon(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;
  result.packetsSent = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  // Setup WiFi for injection
  setupWifiInjectionMode(config.targetChannel);

  isRunning_ = true;
  startTime_ = millis();
  uint32_t delayMs = 1000 / config.packetsPerSec;

  uint8_t bssidBytes[6];
  sscanf(config.targetBssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &bssidBytes[0], &bssidBytes[1], &bssidBytes[2],
         &bssidBytes[3], &bssidBytes[4], &bssidBytes[5]);

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    std::vector<uint8_t> beacon = buildFrame(BEACON, bssidBytes);

    if (config.fuzzPayload) {
      auto fuzzVec = generateFuzzVector();
      beacon.insert(beacon.end(), fuzzVec.begin(), fuzzVec.end());
      result.fuzzingVectorsUsed++;
    }

    sendRawFrame(beacon.data(), beacon.size());
    result.packetsSent++;

    delay(delayMs);
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/wifi_injection.csv";
  logInjection(BEACON, result.packetsSent);
  result.success = true;

  isRunning_ = false;
  return result;
}

InjectionResult PacketInjector::injectProbe(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;
  result.packetsSent = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  // Setup WiFi for injection
  setupWifiInjectionMode(config.targetChannel);

  isRunning_ = true;
  startTime_ = millis();
  uint32_t delayMs = 1000 / config.packetsPerSec;

  uint8_t bssidBytes[6];
  sscanf(config.targetBssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &bssidBytes[0], &bssidBytes[1], &bssidBytes[2],
         &bssidBytes[3], &bssidBytes[4], &bssidBytes[5]);

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    // Alternate between Probe Request and Response
    FrameType type = (result.packetsSent % 2 == 0) ? PROBE_REQUEST : PROBE_RESPONSE;
    std::vector<uint8_t> probe = buildFrame(type, bssidBytes);

    sendRawFrame(probe.data(), probe.size());
    result.packetsSent++;

    delay(delayMs);
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/wifi_injection.csv";
  logInjection(PROBE_REQUEST, result.packetsSent);
  result.success = true;

  isRunning_ = false;
  return result;
}

InjectionResult PacketInjector::injectAuth(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;
  result.packetsSent = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  // Setup WiFi for injection
  setupWifiInjectionMode(config.targetChannel);

  isRunning_ = true;
  startTime_ = millis();
  uint32_t delayMs = 1000 / config.packetsPerSec;

  uint8_t bssidBytes[6];
  sscanf(config.targetBssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &bssidBytes[0], &bssidBytes[1], &bssidBytes[2],
         &bssidBytes[3], &bssidBytes[4], &bssidBytes[5]);

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    std::vector<uint8_t> auth = buildFrame(AUTH_REQUEST, bssidBytes);

    // Add random auth algorithm and status code
    auth.push_back((esp_random() % 2)); // auth type (open/shared)
    auth.push_back((esp_random() % 256)); // status code

    sendRawFrame(auth.data(), auth.size());
    result.packetsSent++;

    delay(delayMs);
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/wifi_injection.csv";
  logInjection(AUTH_REQUEST, result.packetsSent);
  result.success = true;

  isRunning_ = false;
  return result;
}

InjectionResult PacketInjector::injectAssoc(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;
  result.packetsSent = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  // Setup WiFi for injection
  setupWifiInjectionMode(config.targetChannel);

  isRunning_ = true;
  startTime_ = millis();

  uint8_t bssidBytes[6];
  sscanf(config.targetBssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &bssidBytes[0], &bssidBytes[1], &bssidBytes[2],
         &bssidBytes[3], &bssidBytes[4], &bssidBytes[5]);

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    std::vector<uint8_t> assoc = buildFrame(ASSOC_REQUEST, bssidBytes);

    sendRawFrame(assoc.data(), assoc.size());
    result.packetsSent++;

    delay(50); // Association requests at ~20/sec
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/wifi_injection.csv";
  logInjection(ASSOC_REQUEST, result.packetsSent);
  result.success = true;

  isRunning_ = false;
  return result;
}

InjectionResult PacketInjector::fuzzFrames(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;
  result.packetsSent = 0;
  result.fuzzingVectorsUsed = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  // Setup WiFi for injection
  setupWifiInjectionMode(config.targetChannel);

  isRunning_ = true;
  startTime_ = millis();

  uint8_t bssidBytes[6];
  sscanf(config.targetBssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &bssidBytes[0], &bssidBytes[1], &bssidBytes[2],
         &bssidBytes[3], &bssidBytes[4], &bssidBytes[5]);

  const FrameType frameTypes[] = {BEACON, PROBE_REQUEST, AUTH_REQUEST, DATA_FRAME, NULL_FRAME};

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    FrameType type = frameTypes[(esp_random() % 5)];
    std::vector<uint8_t> frame = buildFrame(type, bssidBytes);

    // Fuzz payload
    auto fuzzVec = generateFuzzVector();
    frame.insert(frame.end(), fuzzVec.begin(), fuzzVec.end());

    sendRawFrame(frame.data(), frame.size());
    result.packetsSent++;
    result.fuzzingVectorsUsed++;

    delay(50);
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/wifi_fuzzing.csv";
  result.success = true;

  isRunning_ = false;
  return result;
}

std::vector<uint8_t> PacketInjector::buildFrame(FrameType type, const uint8_t* bssid) {
  std::vector<uint8_t> frame;

  // 802.11 MAC header
  uint16_t frameControl = 0x0000;

  // Set frame type and subtype
  switch (type) {
    case BEACON:
      frameControl = 0x8000; // Beacon
      break;
    case PROBE_REQUEST:
      frameControl = 0x4000; // Probe Request
      break;
    case PROBE_RESPONSE:
      frameControl = 0x5000; // Probe Response
      break;
    case AUTH_REQUEST:
      frameControl = 0xB000; // Auth
      break;
    case ASSOC_REQUEST:
      frameControl = 0x0000; // Association Request
      break;
    case DATA_FRAME:
      frameControl = 0x0800; // Data
      break;
    case NULL_FRAME:
      frameControl = 0x0400; // Null data
      break;
    default:
      frameControl = 0x8000;
  }

  // Frame control
  frame.push_back(frameControl & 0xFF);
  frame.push_back((frameControl >> 8) & 0xFF);

  // Duration
  frame.push_back(0x00);
  frame.push_back(0x00);

  // Receiver address (BSSID)
  for (int i = 0; i < 6; i++) {
    frame.push_back(bssid[i]);
  }

  // Transmitter address
  for (int i = 0; i < 6; i++) {
    frame.push_back((esp_random() % 256));
  }

  // BSSID
  for (int i = 0; i < 6; i++) {
    frame.push_back(bssid[i]);
  }

  // Sequence control
  frame.push_back(seqNum_ & 0xFF);
  frame.push_back((seqNum_ >> 8) & 0xFF);
  seqNum_ += 16; // Increment by 16 (sequence number in upper 12 bits)

  return frame;
}

std::vector<uint8_t> PacketInjector::generateFuzzVector() {
  std::vector<uint8_t> fuzz;
  uint32_t fuzzLen = ((esp_random() % 90) + 10);

  for (uint32_t i = 0; i < fuzzLen; i++) {
    fuzz.push_back((esp_random() % 256));
  }

  return fuzz;
}

void PacketInjector::sendRawFrame(const uint8_t* frame, uint32_t len) {
  if (!frame || len == 0) return;

  // Use esp_wifi_80211_tx to send raw frame
  esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*)frame, len, false);
}

void PacketInjector::logInjection(FrameType type, uint32_t count) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/wifi_injection.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/wifi_injection.csv", "a");
  }

  if (logFile) {
    const char* typeNames[] = {"BEACON", "PROBE_REQ", "PROBE_RESP", "AUTH_REQ", "AUTH_RESP", "ASSOC_REQ", "ASSOC_RESP", "DATA", "NULL"};
    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "%lu,%s,%u\n",
             millis(), typeNames[type], count);
    logFile.print(logEntry);
    logFile.close();
  }

  LittleFS.end();
}

void PacketInjector::stop() {
  isRunning_ = false;
}

} // namespace WifiPacketInjection
