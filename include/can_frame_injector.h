#pragma once
#include <Arduino.h>
#include <vector>

namespace CANFrameInjector {

// CAN frame injection: Active attack to manipulate ECU behavior
// Methods: Replay attack, frame fuzzing, command injection
// Targets: Engine throttle, brake pressure, transmission, steering angle

struct InjectionConfig {
    uint32_t durationMs;      // Attack duration
    uint32_t baudrate;        // CAN bus speed
    uint8_t attackMode;       // 0=replay, 1=throttle, 2=brake, 3=steering
    uint32_t targetECU;       // Target CAN ID
    uint32_t repeatCount;     // How many times to send
    bool rampedAttack;        // Gradual vs immediate
};

struct InjectionResult {
    bool success;
    uint32_t framesSent;
    uint32_t frameFailed;
    uint32_t durationMs;
    String targetECU;
    String attackDescription;
    String error;
};

// Replay captured frame N times (simple but effective)
// Prerequisite: Captured frame from CANBusSniffer
// Effect: Repeat ECU command (can cause uncontrolled acceleration/braking)
InjectionResult replayCANFrame(const InjectionConfig& config, const uint8_t frameData[8]);

// Inject throttle increase command (DANGEROUS)
// Target: 0x100 (Engine Control Unit)
// Effect: Gradually increase RPM / vehicle acceleration
// Safety: Requires TX arm + human intervention monitoring
InjectionResult injectThrottleCommand(const InjectionConfig& config, uint8_t throttlePercent);

// Inject brake pressure command (DANGEROUS)
// Target: 0x200 (Brake Control Module)
// Effect: Trigger brake actuation / emergency stop
// Safety: Requires TX arm + human intervention monitoring
InjectionResult injectBrakeCommand(const InjectionConfig& config, uint8_t brakePressure);

// Fuzz CAN frames: Send malformed/invalid data
// Effect: DoS on specific ECU, causing system shutdown or unpredictable behavior
// Safety: Lab only, requires clear vehicle isolation
InjectionResult fuzzyCANFrames(const InjectionConfig& config);

// Analyze injection success by monitoring CAN bus response
InjectionResult verifyInjectionImpact(uint32_t targetECU, uint32_t verifyDurationMs);

}  // namespace CANFrameInjector
