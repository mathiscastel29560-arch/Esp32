#include "ir_sniffer.h"
#include <LittleFS.h>

namespace IrSniffer {

IrSniffer::IrSniffer() : isRunning_(false), startTime_(0) {}

SnifferResult IrSniffer::captureIrCodes(const SnifferConfig& config) {
  SnifferResult result;
  result.success = false;
  result.codesCapTured = 0;

  pinMode(config.rxPin, INPUT);

  isRunning_ = true;
  startTime_ = millis();

  Serial.println("IR Sniffer: Real protocol decoding active...");
  Serial.printf("Capturing on GPIO %d for %d ms\n", config.rxPin, config.captureTimeMs);

  uint32_t lastLevel = LOW;
  uint32_t lastTime = micros();
  uint32_t codeCount = 0;
  std::vector<uint16_t> timingBuffer;

  while (isRunning_ && (millis() - startTime_) < config.captureTimeMs) {
    uint32_t currentLevel = digitalRead(config.rxPin);
    uint32_t currentTime = micros();

    if (currentLevel != lastLevel) {
      uint32_t timing = currentTime - lastTime;

      if (timing > 100) {
        timingBuffer.push_back((uint16_t)((timing > 65535) ? 65535 : timing));

        if (timingBuffer.size() >= 34) {
          IrCode decodedCode = decodeIrProtocol(timingBuffer);

          if (decodedCode.protocol != "UNKNOWN") {
            decodedCode.timestamp = millis();
            result.codes.push_back(decodedCode);
            result.codesCapTured++;
            codeCount++;

            Serial.printf("  [%d] %s: addr=0x%02X cmd=0x%02X\n",
              codeCount, decodedCode.protocol.c_str(),
              decodedCode.address, decodedCode.command);

            logCode(decodedCode);
            timingBuffer.clear();
          } else if (timingBuffer.size() > 50) {
            timingBuffer.erase(timingBuffer.begin());
          }
        }
      }

      lastLevel = currentLevel;
      lastTime = currentTime;
    }

    if (config.continuousCapture) {
      delayMicroseconds(100);
    } else {
      delayMicroseconds(500);
    }
  }

  result.dominantProtocol = identifyProtocol(timingBuffer);
  result.success = result.codesCapTured > 0;
  result.logFile = "/logs/handshakes/ir_capture.csv";

  Serial.printf("IR capture complete: %d codes, protocol: %s\n",
    result.codesCapTured, result.dominantProtocol.c_str());

  isRunning_ = false;
  return result;
}

IrCode IrSniffer::decodeIrProtocol(const std::vector<uint16_t>& timings) {
  IrCode code;
  code.protocol = "UNKNOWN";
  code.address = 0;
  code.command = 0;

  if (timings.size() < 34) return code;

  if (timings[0] > 8000 && timings[0] < 10000) {
    return decodeNec(timings);
  } else if (timings[0] > 2000 && timings[0] < 2800) {
    return decodeSony(timings);
  } else if (timings.size() > 20) {
    return decodeRc5(timings);
  }

  return code;
}

IrCode IrSniffer::decodeNec(const std::vector<uint16_t>& timings) {
  IrCode code;
  code.protocol = "NEC";
  code.address = 0;
  code.command = 0;

  if (timings.size() < 34) return code;

  for (int i = 1; i < 17 && i < timings.size(); i++) {
    if (timings[i * 2] > 1000) {
      code.address |= (1 << (16 - i));
    }
  }

  for (int i = 17; i < 33 && i < timings.size(); i++) {
    if (timings[i * 2] > 1000) {
      code.command |= (1 << (32 - i));
    }
  }

  return code;
}

IrCode IrSniffer::decodeSony(const std::vector<uint16_t>& timings) {
  IrCode code;
  code.protocol = "SONY";
  code.address = 0;
  code.command = 0;

  if (timings.size() < 20) return code;

  for (int i = 1; i < 9 && i * 2 < timings.size(); i++) {
    if (timings[i * 2] > 600) {
      code.address |= (1 << (8 - i));
    }
  }

  for (int i = 9; i < 20 && i * 2 < timings.size(); i++) {
    if (timings[i * 2] > 600) {
      code.command |= (1 << (20 - i));
    }
  }

  return code;
}

IrCode IrSniffer::decodeRc5(const std::vector<uint16_t>& timings) {
  IrCode code;
  code.protocol = "RC5";
  code.address = 0;
  code.command = 0;

  if (timings.size() < 14) return code;

  for (int i = 1; i < 6 && i * 2 < timings.size(); i++) {
    if (timings[i * 2] > 900) {
      code.address |= (1 << (6 - i));
    }
  }

  for (int i = 6; i < 14 && i * 2 < timings.size(); i++) {
    if (timings[i * 2] > 900) {
      code.command |= (1 << (14 - i));
    }
  }

  return code;
}

String IrSniffer::identifyProtocol(const std::vector<uint16_t>& timings) {
  if (timings.empty()) return "UNKNOWN";

  if (timings[0] > 8000 && timings[0] < 10000) {
    return "NEC";
  } else if (timings[0] > 2000 && timings[0] < 2800) {
    return "SONY";
  } else if (timings.size() > 20) {
    return "RC5";
  }

  return "UNKNOWN";
}

void IrSniffer::logCode(const IrCode& code) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ir_capture.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ir_capture.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,%s,%02X,%02X,%u\n", code.timestamp, code.protocol.c_str(),
                   code.address, code.command, (uint32_t)code.timings.size());
    logFile.close();
  }

  LittleFS.end();
}

void IrSniffer::stop() {
  isRunning_ = false;
}

} // namespace IrSniffer
