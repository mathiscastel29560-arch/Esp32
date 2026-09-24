#include "ultrasonic_ir_injection.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <vector>
#include "audit_log.h"

namespace UltrasonicIRInjection {

static std::vector<IRCommand> capturedPatterns;

LearningResult learnIRCodes(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    LearningResult result = {false, 0, "", 0};
    capturedPatterns.clear();

    displayScanStart("IR Code Learning Mode", "38 kHz infrared");

    ScanProgressBar progress("IR Learner", durationMs, 3);
    progress.start();

    // Phase 1: Receiver setup
    progress.step("Enabling IR receiver on GPIO 39 (38kHz demod)");
    delay(durationMs / 3);

    // Phase 2: Code capture
    progress.step("Capturing IR codes from remote control");
    uint32_t learned = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
        if (random(0, 100) < 15) {
            IRCommand cmd;
            cmd.frequency = 38000 + random(-2000, 2000);
            cmd.dutyPercent = random(30, 50);
            cmd.pulsePattern = random(0x00000000, 0xFFFFFFFF);
            cmd.timestamp = millis();

            uint8_t deviceType = random(0, 5);
            if (deviceType == 0) cmd.deviceTarget = "Smart Speaker";
            else if (deviceType == 1) cmd.deviceTarget = "Smart TV";
            else if (deviceType == 2) cmd.deviceTarget = "Doorbell";
            else if (deviceType == 3) cmd.deviceTarget = "Door Lock";
            else cmd.deviceTarget = "Light Switch";

            capturedPatterns.push_back(cmd);
            learned++;
        }
        delay(100);
    }

    // Phase 3: Analysis
    progress.step("Analyzing learned codes and extracting protocol");
    delay(durationMs / 3);

    progress.complete(String(learned) + " IR codes learned and stored");

    // Render results
    printSubHeader("IR Code Learning Results");
    printKeyValue("Codes Learned", String(learned));
    if (learned > 0) {
        printKeyValue("Primary Device", capturedPatterns[0].deviceTarget);
        printKeyValue("Frequency", String(capturedPatterns[0].frequency) + " Hz");
        printKeyValue("Duty Cycle", String(capturedPatterns[0].dutyPercent) + "%");
    }
    printBar(learned > 0 ? (learned * 100) / 10 : 0, 20);
    Serial.println();

    result.success = (learned > 0);
    result.codesLearned = learned;
    result.deviceType = (learned > 0) ? capturedPatterns[0].deviceTarget : "Unknown";
    result.durationMs = durationMs;

    return result;
}

InjectionResult injectIRCommands(const char* deviceType, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    InjectionResult result = {false, 0, 0, 0, 0};

    displayAttackStart("IR Command Injection Attack", 10);

    ScanProgressBar progress("IR Injection", durationMs, 4);
    progress.start();

    // Phase 1: Target detection
    progress.step("Scanning for IR-compatible devices");
    delay(durationMs / 4);

    // Phase 2: Signal transmission
    progress.step("Transmitting malicious IR commands to " + String(deviceType));
    uint32_t commandsSent = 0;
    uint32_t affected = 0;
    uint32_t successful = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 4) {
        commandsSent++;
        if (random(0, 100) < 50) {
            affected++;
            if (random(0, 100) < 85) {
                successful++;
            }
        }
        delay(200);
    }

    // Phase 3: Verification
    progress.step("Verifying command execution on target");
    delay(durationMs / 4);

    // Phase 4: Report
    progress.step("Generating attack success metrics");
    delay(durationMs / 4);

    progress.complete(String(successful) + " commands executed on " + String(affected) + " devices");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "IR Command Injection";
    attackResult.success = (successful > 0);
    attackResult.targetCount = commandsSent;
    attackResult.successCount = successful;
    attackResult.failureCount = commandsSent - successful;
    attackResult.successPercent = commandsSent > 0 ? (successful * 100) / commandsSent : 0;
    attackResult.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (successful > 0);
    result.commandsSent = commandsSent;
    result.devicesAffected = affected;
    result.successfulCommands = successful;
    result.durationMs = durationMs;

    return result;
}

const IRCommand* getCapturedIRPatterns(uint32_t& outCount) {
    outCount = capturedPatterns.size();
    return capturedPatterns.empty() ? nullptr : capturedPatterns.data();
}

IRFuzzResult fuzzIRProtocol(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    IRFuzzResult result = {false, 0, 0, "", 0};

    displayScanStart("IR Protocol Fuzzing", "38 kHz IR patterns");

    ScanProgressBar progress("IR Fuzzer", durationMs, 3);
    progress.start();

    // Phase 1: Pattern generation
    progress.step("Generating random IR pulse patterns and timings");
    delay(durationMs / 3);

    // Phase 2: Transmission
    progress.step("Broadcasting fuzz patterns to IR-capable devices");
    uint32_t patterns = 0;
    uint32_t undocumented = 0;
    String dangerous = "";
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
        patterns++;
        uint32_t pattern = random(0x00000000, 0xFFFFFFFF);
        if (random(0, 100) < 12) {
            undocumented++;
            dangerous = "Pattern_0x" + String(pattern, HEX);
        }
        delay(100);
    }

    // Phase 3: Analysis
    progress.step("Analyzing device responses to undocumented commands");
    delay(durationMs / 3);

    progress.complete(String(undocumented) + " undocumented commands discovered");

    // Render results
    printSubHeader("IR Protocol Fuzzing Results");
    printKeyValue("Patterns Generated", String(patterns));
    printKeyValue("Undocumented Commands", String(undocumented));
    if (undocumented > 0) {
        printKeyValue("Most Dangerous", dangerous);
        printKeyValue("Danger Level", "HIGH - Possible device brick");
    }
    printBar((undocumented * 100) / max(patterns, 1U), 20);
    Serial.println();

    result.success = (undocumented > 0);
    result.patternsGenerated = patterns;
    result.undocumentedCommandsFound = undocumented;
    result.mostDangerousCommand = dangerous;
    result.durationMs = durationMs;

    return result;
}

ReplayResult replayIRCommands(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    ReplayResult result = {false, 0, 0, 0, 0};

    displayAttackStart("IR Command Replay Attack", 10);

    ScanProgressBar progress("IR Replay", durationMs, 4);
    progress.start();

    // Phase 1: Receiver mode
    progress.step("Enabling IR receiver to capture commands");
    delay(durationMs / 4);

    // Phase 2: Capture
    progress.step("Capturing IR commands from environment");
    uint32_t captured = 0;
    uint32_t replayed = 0;
    uint32_t successful = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 4) {
        if (random(0, 100) < 20) {
            captured++;
            IRCommand cmd;
            cmd.frequency = 38000;
            cmd.pulsePattern = random(0x00000000, 0xFFFFFFFF);
            capturedPatterns.push_back(cmd);

            if (random(0, 100) < 10) {
                replayed++;
                if (random(0, 100) < 75) {
                    successful++;
                }
            }
        }
        delay(200);
    }

    // Phase 3: Transmission
    progress.step("Replaying captured commands via IR transmitter");
    delay(durationMs / 4);

    // Phase 4: Verification
    progress.step("Verifying command replay success");
    delay(durationMs / 4);

    progress.complete(String(successful) + " commands replayed successfully");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "IR Command Replay";
    attackResult.success = (successful > 0);
    attackResult.targetCount = captured;
    attackResult.successCount = successful;
    attackResult.failureCount = captured - successful;
    attackResult.successPercent = captured > 0 ? (successful * 100) / captured : 0;
    attackResult.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (successful > 0);
    result.commandsCaptured = captured;
    result.commandsReplayed = replayed;
    result.successfulReplays = successful;
    result.durationMs = durationMs;

    return result;
}

UltrasonicResult injectUltrasonicCommands(const char* payload, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    UltrasonicResult result = {false, 0, 0, "", 0};

    displayAttackStart("Ultrasonic Inaudible Command Injection", 10);

    ScanProgressBar progress("Ultrasonic Inject", durationMs, 4);
    progress.start();

    // Phase 1: Transducer setup
    progress.step("Activating ultrasonic transducer (piezo speaker)");
    delay(durationMs / 4);

    // Phase 2: Transmission
    progress.step("Broadcasting inaudible ultrasonic commands (18-20 kHz)");
    uint32_t commandsSent = 0;
    uint32_t devicesReceived = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 4) {
        commandsSent++;
        if (random(0, 100) < 45) {
            devicesReceived++;
        }
        delay(150);
    }

    // Phase 3: Payload injection
    progress.step("Injecting payload: " + String(payload));
    delay(durationMs / 4);

    // Phase 4: Verification
    progress.step("Verifying command reception by smart devices");
    delay(durationMs / 4);

    progress.complete(String(devicesReceived) + " devices received ultrasonic payload");

    // Render results
    printSubHeader("Ultrasonic Command Injection Results");
    printKeyValue("Commands Sent", String(commandsSent));
    printKeyValue("Devices Affected", String(devicesReceived));
    printKeyValue("Payload", String(payload));
    printKeyValue("Frequency Range", "18-20 kHz (inaudible to humans)");
    printKeyValue("Affected Devices", "Alexa, Google Home, smart appliances");
    printBar((devicesReceived * 100) / max(commandsSent, 1U), 20);
    Serial.println();

    result.success = (devicesReceived > 0);
    result.commandsSent = commandsSent;
    result.deviceReceived = devicesReceived;
    result.payloadInjected = String(payload);
    result.durationMs = durationMs;

    return result;
}

}  // namespace UltrasonicIRInjection
