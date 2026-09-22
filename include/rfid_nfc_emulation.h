#ifndef RFID_NFC_EMULATION_H
#define RFID_NFC_EMULATION_H

#include <Arduino.h>

namespace RfidNfcEmulation {

struct EmulationConfig {
  uint8_t targetUid[7];
  uint32_t durationMs;
  bool replayMode;
  bool cloneFromCapture;
};

struct EmulationResult {
  bool success;
  uint32_t emulationTime;
  uint32_t readsDetected;
  String logFile;
};

class RfidEmulator {
public:
  RfidEmulator();
  EmulationResult emulateNfcTag(const EmulationConfig& config);
  EmulationResult replayCapture(const uint8_t* data, uint32_t len);
  void stop();

private:
  bool isRunning_;
};

} // namespace RfidNfcEmulation

#endif
