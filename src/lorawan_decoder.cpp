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
    if (random(0, 100) < 15) { // 15% chance per cycle
      LoRaFrame frame;
      frame.timestamp = millis();
      frame.mhdr = random(0x00, 0x80);
      frame.appEui = random(0, 0xFFFFFFFF);
      frame.devEui = random(0, 0xFFFFFFFF);
      frame.devNonce = random(0, 0xFFFF);
      frame.rssi = random(-120, -70);

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
