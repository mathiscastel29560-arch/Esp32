#ifndef BADUSB_EXFILTRATION_H
#define BADUSB_EXFILTRATION_H

#include <Arduino.h>
#include <vector>

namespace BadUsbExfiltration {

struct UsbConfig {
  uint32_t durationMs;
  bool captureScreenshots;
  bool logKeypresses;
  bool exfiltrateData;
};

struct UsbResult {
  bool success;
  uint32_t screenshotsCaptured;
  uint32_t keystrokesLogged;
  uint32_t bytesExfiltrated;
  String logFile;
};

class BadUsb {
public:
  BadUsb();
  UsbResult executePayload(const UsbConfig& config);
  UsbResult captureScreenshot();
  void stop();

private:
  bool isRunning_;
};

} // namespace BadUsbExfiltration

#endif
