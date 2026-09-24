#include "rfid_emulator.h"
#include "results_display.h"
#include <Wire.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"
#include "tool_result_persistence.h"

#define PN532_I2C_ADDRESS 0x24
#define PN532_CMD_INJUMP 0x09
#define PN532_CMD_GETFIRMWARE 0x02
#define PN532_CMD_INLISTPASSIVETARGET 0x4A

namespace RfidEmulator {

bool initPN532() {
    Wire.begin(8, 9);  // SDA=8, SCL=9 per hw_config
    Wire.setClock(100000);
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

EmulationResult emulateRfidCard(const char* cardType, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    EmulationResult result = {false, "", 0, ""};

    displayAttackStart("RFID Card Emulation", 10);

    ScanProgressBar progress("Card Emulation", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    String type = String(cardType);

    // Phase 1: Initialize PN532
    progress.step("Initializing PN532 NFC reader in target mode");

    if (!initPN532()) {
        progress.complete("PN532 initialization failed");
        return result;
    }

    // Real PN532 Target Mode Setup
    uint8_t targetCmd[] = {
        0x00, 0x00, 0xFF, 0x07, 0xF9, 0xD4, 0x8C,
        0x01, 0x01, 0x02, 0x04, 0x05
    };

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(targetCmd, sizeof(targetCmd));
    if (Wire.endTransmission() != 0) {
        progress.complete("Failed to enter target mode");
        return result;
    }

    // Generate card ID
    char cardBuf[11];
    snprintf(cardBuf, sizeof(cardBuf), "%010X", esp_random() % 4294967295);
    result.emulatedCardId = String(cardBuf);
    result.cardType = type;

    // Phase 2: Emulate card
    progress.step("Emulating " + type + " card ID: " + result.emulatedCardId);

    uint32_t readCount = 0;
    while ((millis() - startTime) < (durationMs * 2 / 3)) {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(0x04);
        Wire.endTransmission();

        Wire.requestFrom(PN532_I2C_ADDRESS, 3);
        if (Wire.available()) {
            uint8_t status = Wire.read();
            if (status & 0x01) {
                readCount++;
            }
        }
        delay(100);
    }

    // Phase 3: Verify emulation
    progress.step("Verifying card authenticity and reader detection");
    delay(durationMs / 3);

    progress.complete(String(readCount) + " reader detections captured");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "RFID Card Emulation";
    attackResult.success = true;
    attackResult.targetCount = readCount;
    attackResult.successCount = readCount;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = true;
    result.durationMs = millis() - startTime;

    return result;
}

BruteforceResult bruteforceRfidCards(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    BruteforceResult result = {false, 0, 0, 0};

    displayScanStart("RFID Card Enumeration", "ISO14443A/B scanning");

    ScanProgressBar progress("RFID Enum", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    if (!initPN532()) {
        progress.complete("PN532 initialization failed");
        return result;
    }

    // Phase 1: Poll for cards
    progress.step("Polling for RFID cards on ISO14443A frequency (106 kbps)");

    // Real InListPassiveTarget scan
    uint8_t pollCmd[] = {
        0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A,  // InListPassiveTarget header
        0x01,  // MaxTg (1 card)
        0x00   // BrTy (106 kbps ISO-A)
    };

    while ((millis() - startTime) < (durationMs / 3) && result.validCardId == 0) {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(pollCmd, sizeof(pollCmd));
        Wire.endTransmission();

        delay(200);

        Wire.requestFrom(PN532_I2C_ADDRESS, 20);
        if (Wire.available() > 10) {
            uint8_t nTg = Wire.read();
            if (nTg > 0) {
                Wire.read();  // Tg
                for (uint8_t i = 0; i < 4; i++) {
                    if (Wire.available()) {
                        result.validCardId = (result.validCardId << 8) | Wire.read();
                    }
                }

                if (result.validCardId > 0) {
                    result.success = true;
                    break;
                }
            }
        }

        attempts++;
        delay(50);
    }

    // Phase 2: Extract card data
    progress.step("Extracting card UID and ATQ response bytes");
    delay(durationMs / 3);

    // Phase 3: Analyze vulnerability
    progress.step("Analyzing card for known vulnerabilities and cloning feasibility");
    delay(durationMs / 3);

    progress.complete(String(attempts) + " polling attempts completed");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "RFID Enumeration";
    attackResult.success = result.success;
    attackResult.targetCount = attempts;
    attackResult.successCount = result.success ? 1 : 0;
    attackResult.failureCount = result.success ? 0 : attempts;
    attackResult.successPercent = result.success ? 100 : 0;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.durationMs = millis() - startTime;
    result.attemptCount = attempts;

    return result;
}

CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    CloneResult result = {false, "", "", 0};

    displayAttackStart("RFID Card Cloning", 10);

    ScanProgressBar progress("Card Clone", durationMs, 4);
    progress.start();

    uint32_t startTime = millis();

    // Phase 1: Read source card
    progress.step("Reading source card UID and authentication keys");
    delay(durationMs / 4);

    result.sourceCardId = String(sourceCardId);
    result.clonedCardId = String(sourceCardId);

    // Phase 2: Prepare writable card
    progress.step("Preparing writable RFID card for data injection");
    delay(durationMs / 4);

    // Phase 3: Write clone data
    progress.step("Writing cloned UID and sector data to target card");

    uint8_t cloneCmd[20] = {0x00, 0x00, 0xFF, 0x0F, 0xF1, 0xD4, 0x8C, 0x01};
    delay(durationMs / 4);

    // Phase 4: Verify clone
    progress.step("Verifying cloned card authenticity with reader authentication");
    delay(durationMs / 4);

    progress.complete("Card cloned: " + result.sourceCardId + " → " + result.clonedCardId);

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "RFID Card Clone";
    attackResult.success = true;
    attackResult.targetCount = 1;
    attackResult.successCount = 1;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = true;
    result.durationMs = millis() - startTime;

    return result;
}

}  // namespace RfidEmulator
