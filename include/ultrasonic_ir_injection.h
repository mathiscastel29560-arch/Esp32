#pragma once
#include <Arduino.h>

namespace UltrasonicIRInjection {

struct IRCommand {
    uint32_t frequency;        // 38kHz typical, but variable
    uint16_t dutyPercent;      // 30-50% duty cycle
    uint32_t pulsePattern;     // Raw pulse timings
    String deviceTarget;       // Device type: Smart Speaker, TV, Doorbell, etc
    uint32_t timestamp;
};

struct InjectionResult {
    bool success;
    uint32_t commandsSent;
    uint32_t devicesAffected;
    uint32_t successfulCommands;
    uint32_t durationMs;
};

// Learn IR codes from legitimate remote
struct LearningResult {
    bool success;
    uint32_t codesLearned;
    String deviceType;
    uint32_t durationMs;
};
LearningResult learnIRCodes(uint32_t durationMs = 30000);

// Inject IR commands to smart devices
InjectionResult injectIRCommands(const char* deviceType, uint32_t durationMs = 25000);

// Get captured IR patterns
const IRCommand* getCapturedIRPatterns(uint32_t& outCount);

// Fuzzing IR protocol to find undocumented commands
struct IRFuzzResult {
    bool success;
    uint32_t patternsGenerated;
    uint32_t undocumentedCommandsFound;
    String mostDangerousCommand;
    uint32_t durationMs;
};
IRFuzzResult fuzzIRProtocol(uint32_t durationMs = 45000);

// Replay attack - capture and repeat commands
struct ReplayResult {
    bool success;
    uint32_t commandsCaptured;
    uint32_t commandsReplayed;
    uint32_t successfulReplays;
    uint32_t durationMs;
};
ReplayResult replayIRCommands(uint32_t durationMs = 35000);

// Ultrasonic payload injection (inaudible commands)
struct UltrasonicResult {
    bool success;
    uint32_t commandsSent;
    uint32_t deviceReceived;
    String payloadInjected;
    uint32_t durationMs;
};
UltrasonicResult injectUltrasonicCommands(const char* payload, uint32_t durationMs = 20000);

}  // namespace UltrasonicIRInjection
