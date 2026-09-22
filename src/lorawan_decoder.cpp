#include "lorawan_decoder.h"
#include <LittleFS.h>

namespace LoRaWanDecoder {

LoRaDecoder::LoRaDecoder() : isRunning_(false) {}

DecoderResult LoRaDecoder::decodeLoRaWan(const DecoderConfig& config) {
  DecoderResult result;
  result.success = false;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Simulate LoRaWAN frame capture and decoding
  // LoRaWAN frame structure:
  // MHDR (1B) | MAC Header
  // AppEUI/DevEUI/etc based on frame type
  // MIC (4B) | Message Integrity Check

  uint32_t frameCount = 0;
  uint32_t devicesFound = 0;

  while (isRunning_ && (millis() - startTime) < config.scanDurationMs) {
    if ((esp_random() % 100) < 15) { // 15% chance per cycle
      LoRaFrame frame;
      frame.timestamp = millis();
      frame.mhdr = (esp_random() % 128);
      frame.appEui = (esp_random() % 4294967295);
      frame.devEui = (esp_random() % 4294967295);
      frame.devNonce = (esp_random() % 65535);
      frame.rssi = ((esp_random() % 50) + -120);

      frame.decodedData = "LoRa_";
      frame.decodedData += String(frame.devEui, HEX);

      result.frames.push_back(frame);
      frameCount++;

      if (frameCount % 5 == 0) {
        devicesFound++;
      }
    }

    delay(100);
  }

  result.framesDecoded = frameCount;
  result.devicesDiscovered = devicesFound;
  result.success = frameCount > 0;
  result.logFile = "/logs/handshakes/lorawan.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/lorawan.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/lorawan.csv", "a");
  }
  if (logFile) {
    logFile.printf("%lu,%u,%u\n", millis(), frameCount, devicesFound);
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

void LoRaDecoder::stop() {
  isRunning_ = false;
}

} // namespace LoRaWanDecoder
