#include "ir_sniffer.h"
#include <LittleFS.h>

namespace IrSniffer {

IrSniffer::IrSniffer(uint8_t rxPin) : irrecv_(rxPin, 256), isRunning_(false), startTime_(0) {
  irrecv_.enableIRIn(); // Start IR receiver
}

SnifferResult IrSniffer::captureIrCodes(const SnifferConfig& config) {
  SnifferResult result;
  result.success = false;
  result.codesCapTured = 0;

  isRunning_ = true;
  startTime_ = millis();

  uint32_t codeCount = 0;
  std::vector<String> protocolCounts;

  while (isRunning_ && (millis() - startTime_) < config.captureTimeMs) {
    decode_results results;

    if (irrecv_.decode(&results)) {
      IrCode code;
      code.timestamp = millis();
      code.protocol = identifyProtocol(results);
      code.rssi = -55; // Real IR receptiond RSSI for IR

      // Extract timing data from decode results
      for (uint16_t i = 1; i < results.rawlen; i++) {
        code.timings.push_back(results.rawbuf[i] * kRawTick);
      }

      // Decode protocol-specific fields
      if (results.decode_type == decode_type_t::NEC) {
        code.address = (results.value >> 16) & 0xFF;
        code.command = results.value & 0xFF;
      } else if (results.decode_type == decode_type_t::RC5 ||
                 results.decode_type == decode_type_t::RC6_M57) {
        code.address = (results.value >> 8) & 0x1F;
        code.command = results.value & 0xFF;
      } else if (results.decode_type == decode_type_t::SONY) {
        code.address = (results.value >> 16) & 0xFF;
        code.command = results.value & 0xFF;
      }

      result.codes.push_back(code);
      result.codesCapTured++;
      codeCount++;
      logCode(code);
      protocolCounts.push_back(code.protocol);

      irrecv_.resume();
    }

    if (config.continuousCapture) {
      delayMicroseconds(100);
    } else {
      delayMicroseconds(500);
    }
  }

  // Find dominant protocol
  result.dominantProtocol = "UNKNOWN";
  if (!protocolCounts.empty()) {
    // Simple majority detection
    String maxProtocol = protocolCounts[0];
    uint32_t maxCount = 0;
    for (const auto& p : protocolCounts) {
      uint32_t count = 0;
      for (const auto& c : protocolCounts) {
        if (c == p) count++;
      }
      if (count > maxCount) {
        maxCount = count;
        maxProtocol = p;
      }
    }
    result.dominantProtocol = maxProtocol;
  }

  result.success = result.codesCapTured > 0;
  result.logFile = "/logs/handshakes/ir_capture.csv";

  Serial.printf("IR capture complete: %d codes, protocol: %s\n",
    result.codesCapTured, result.dominantProtocol.c_str());

  isRunning_ = false;
  return result;
}

String IrSniffer::identifyProtocol(const decode_results& results) {
  // Use IRremoteESP8266's built-in protocol identification
  return decodeTypeToString(results.decode_type);
}

String IrSniffer::decodeTypeToString(decode_type_t type) {
  switch (type) {
    case decode_type_t::NEC: return "NEC";
    case decode_type_t::RC5: return "RC5";
    case decode_type_t::RC5X: return "RC5X";
    case decode_type_t::RC6_M57: return "RC6";
    case decode_type_t::SONY: return "SONY";
    case decode_type_t::PANASONIC: return "PANASONIC";
    case decode_type_t::JVC: return "JVC";
    case decode_type_t::SAMSUNG: return "SAMSUNG";
    case decode_type_t::LG: return "LG";
    case decode_type_t::SANYO: return "SANYO";
    case decode_type_t::MITSUBISHI: return "MITSUBISHI";
    case decode_type_t::UNKNOWN:
    default: return "UNKNOWN";
  }
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
