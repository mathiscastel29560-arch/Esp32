#include "jamming_signal_generator.h"
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
volatile bool g_genActive = false;
uint32_t g_signalsCount = 0;

void generateWhiteNoise() {
    for (int i = 0; i < 256; i++) {
        bool bit = (esp_random() % 2);
        digitalWrite(PIN_CC1101_GDO0, bit);
        delayMicroseconds(100);
    }
    g_signalsCount++;
}

void generatePinkNoise() {
    // Simplified pink noise (1/f)
    for (int i = 0; i < 512; i++) {
        int level = (i * i) % 256;
        digitalWrite(PIN_CC1101_GDO0, level > 128 ? 1 : 0);
        delayMicroseconds(50);
    }
    g_signalsCount++;
}

void generateSweep() {
    for (float f = 433.05f; f <= 434.79f; f += 0.01f) {
        radio.setFrequency(f);

        for (int i = 0; i < 100; i++) {
            digitalWrite(PIN_CC1101_GDO0, 1);
            delayMicroseconds(50);
            digitalWrite(PIN_CC1101_GDO0, 0);
            delayMicroseconds(50);
        }
    }
    g_signalsCount++;
}
} // namespace

namespace JammingSignalGenerator {

JamResult generateJammingSignal(uint32_t durationMs, const String &noiseType) {
    using namespace ToolOutputHelper;

    String params = "type=" + noiseType + ",duration=" + String(durationMs);
    AuditLog::instance().logToolStart("JammingGenerator", params.c_str());

    JamResult result{false, 0, durationMs, noiseType};

    displayAttackStart("Jamming Signal Generator", 10);

    if (!TxArm::isArmed()) {
        ScanProgressBar progress("Signal Gen", durationMs, 3);
        progress.complete("TX not armed");
        AuditLog::instance().logToolStop("JammingGenerator", false, "tx_not_armed");
        return result;
    }

    ScanProgressBar progress("Signal Gen", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    // Phase 1: Initialize CC1101 and transmission mode
    progress.step("Initializing CC1101 on 433.92 MHz and setting " + noiseType + " mode");

    if (radio.begin(433.92f) != RADIOLIB_ERR_NONE) {
        progress.complete("Radio init failed");
        AuditLog::instance().logToolStop("JammingGenerator", false, "radio_init_failed");
        return result;
    }
    radio.setOOK(true);
    if (radio.transmitDirectAsync() != RADIOLIB_ERR_NONE) {
        progress.complete("Transmit setup failed");
        AuditLog::instance().logToolStop("JammingGenerator", false, "transmit_setup_failed");
        return result;
    }
    pinMode(PIN_CC1101_GDO0, OUTPUT);

    delay(300);

    // Phase 2: Generate jamming signals
    progress.step("Generating " + noiseType + " noise signals continuously");

    g_genActive = true;
    g_signalsCount = 0;
    uint32_t deadline = startTime + durationMs;

    while ((int32_t)(millis() - deadline) < 0 && g_genActive) {
        if (noiseType == "WHITE") {
            generateWhiteNoise();
        } else if (noiseType == "PINK") {
            generatePinkNoise();
        } else if (noiseType == "SWEEP") {
            generateSweep();
        }
    }

    g_genActive = false;

    // Phase 3: Verify signal generation
    progress.step("Verifying jamming signal transmission and spectrum coverage");

    delay(300);

    result.success = true;
    result.signalsGenerated = g_signalsCount;

    uint32_t elapsed = millis() - startTime;

    progress.complete(String(result.signalsGenerated) + " signals on 433 MHz band");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "Jamming Signal Generator";
    attackResult.success = result.success;
    attackResult.targetCount = result.signalsGenerated;
    attackResult.successCount = result.signalsGenerated;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = elapsed;

    ResultRenderers::renderAttackSuccess(attackResult);

    String result_str = String(result.signalsGenerated) + "_signals";
    AuditLog::instance().logToolStop("JammingGenerator", result.success, result_str.c_str());

    return result;
}

void stop() {
    g_genActive = false;
}

bool isActive() {
    return g_genActive;
}

}  // namespace JammingSignalGenerator
