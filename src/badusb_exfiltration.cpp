#include "badusb_exfiltration.h"
#include <LittleFS.h>

namespace BadUsbExfiltration {

BadUsb::BadUsb() : isRunning_(false) {}

UsbResult BadUsb::executePayload(const UsbConfig& config) {
  UsbResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  uint32_t screenshotCount = 0;
  uint32_t keystrokeCount = 0;
  uint32_t exfiltratedBytes = 0;

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    if (config.captureScreenshots && (esp_random() % 100) < 10) {
      // Simulate screenshot capture
      screenshotCount++;
      exfiltratedBytes += ((esp_random() % 400000) + 100000); // Simulated screenshot size
    }

    if (config.logKeypresses && (esp_random() % 100) < 30) {
      // Simulate keylogger
      keystrokeCount += ((esp_random() % 9) + 1);
    }

    if (config.exfiltrateData && keystrokeCount > 0) {
      // Exfiltrate via WiFi/BLE
      exfiltratedBytes += keystrokeCount * 2;
    }

    delay(100);
  }

  result.screenshotsCaptured = screenshotCount;
  result.keystrokesLogged = keystrokeCount;
  result.bytesExfiltrated = exfiltratedBytes;
  result.success = true;
  result.logFile = "/logs/handshakes/badusb.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/badusb.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/badusb.csv", "a");
  }
  if (logFile) {
    logFile.printf("%lu,%u,%u,%u\n", millis(), screenshotCount, keystrokeCount, exfiltratedBytes);
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

UsbResult BadUsb::captureScreenshot() {
  UsbResult result;
  result.success = false;

  // Simulate screenshot capture
  result.screenshotsCaptured = 1;
  result.bytesExfiltrated = ((esp_random() % 400000) + 100000);
  result.success = true;

  return result;
}

void BadUsb::stop() {
  isRunning_ = false;
}

} // namespace BadUsbExfiltration
