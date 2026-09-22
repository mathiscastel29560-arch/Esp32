#include "gpio_debugger.h"
#include <LittleFS.h>

namespace GpioDebugger {

Debugger::Debugger() : isRunning_(false) {}

DebugResult Debugger::scanDebugInterfaces(const DebugConfig& config) {
  DebugResult result;
  result.success = false;
  result.interfaceFound = false;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Scan GPIO pins for UART/JTAG/SWD signatures
  const uint8_t testPins[] = {0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17};

  while (isRunning_ && (millis() - startTime) < config.scanTimeoutMs) {
    for (uint8_t pin : testPins) {
      pinMode(pin, INPUT);
      uint8_t value = digitalRead(pin);

      // Simulate debug interface detection
      if (config.interface == UART) {
        // Look for TX/RX patterns (alternating 1s and 0s)
        if ((esp_random() % 100) < 15) {
          result.interfaceFound = true;
          result.deviceInfo = "UART found on GPIO" + String(pin);
          break;
        }
      } else if (config.interface == JTAG) {
        // Look for TCK/TMS patterns
        if ((esp_random() % 100) < 10) {
          result.interfaceFound = true;
          result.deviceInfo = "JTAG found on GPIO" + String(pin);
          break;
        }
      } else if (config.interface == SWD) {
        // Look for SWCLK/SWDIO patterns
        if ((esp_random() % 100) < 12) {
          result.interfaceFound = true;
          result.deviceInfo = "SWD found on GPIO" + String(pin);
          break;
        }
      }

      delay(50);
    }

    if (result.interfaceFound) break;
    delay(100);
  }

  result.registersRead = ((esp_random() % 90) + 10);
  result.success = result.interfaceFound;
  result.logFile = "/logs/handshakes/gpio_debug.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/gpio_debug.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/gpio_debug.csv", "a");
  }
  if (logFile) {
    const char* ifaceNames[] = {"UART", "JTAG", "SWD"};
    logFile.printf("%lu,%s,%u\n", millis(), ifaceNames[config.interface], result.registersRead);
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

DebugResult Debugger::dumpFirmware() {
  DebugResult result;
  result.success = false;

  // Simulate firmware extraction via JTAG/SWD
  result.registersRead = ((esp_random() % 4000) + 1000);
  result.success = true;
  result.logFile = "/logs/handshakes/firmware_dump.csv";

  return result;
}

void Debugger::stop() {
  isRunning_ = false;
}

} // namespace GpioDebugger
