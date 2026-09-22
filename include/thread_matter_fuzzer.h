#ifndef THREAD_MATTER_FUZZER_H
#define THREAD_MATTER_FUZZER_H

#include <Arduino.h>

namespace ThreadMatterFuzzer {

struct FuzzerConfig {
  uint32_t durationMs;
  bool fuzzMlrRequests;
  bool fuzzCommissioningMessages;
};

struct FuzzerResult {
  bool success;
  uint32_t messagesSent;
  uint32_t crashesDetected;
  String logFile;
};

class Fuzzer {
public:
  Fuzzer();
  FuzzerResult fuzzMatterDevice(const FuzzerConfig& config);
  void stop();

private:
  bool isRunning_;
};

} // namespace ThreadMatterFuzzer

#endif
