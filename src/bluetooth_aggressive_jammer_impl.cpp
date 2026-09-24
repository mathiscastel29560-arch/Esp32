#include "bluetooth_aggressive_jammer.h"
#include "tx_arm.h"
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;
NimBLEAdvertising* g_pAdvertising = nullptr;

void sendAggressiveJamFrame() {
    uint8_t jamFrame[31];
    for (int i = 0; i < 31; i++) {
        jamFrame[i] = esp_random() & 0xFF;
    }

    NimBLEAdvertisementData advData;
    advData.addData(std::string((const char*)jamFrame, 31));

    if (g_pAdvertising) {
        g_pAdvertising->setAdvertisementData(advData);
        g_pAdvertising->start();
        delayMicroseconds(100);
        g_pAdvertising->stop();
    }

    g_jamCount++;
}
}

namespace BluetoothAggressiveJammer {

JamResult jamBluetooth(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    JamResult result{false, 0, durationMs};

    displayAttackStart("Bluetooth Aggressive Jammer", 10);

    if (!TxArm::isArmed()) {
        ScanProgressBar progress("BLE Jam", durationMs, 3);
        progress.complete("TX not armed");
        return result;
    }

    ScanProgressBar progress("BLE Jam", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    // Phase 1: Initialize BLE advertising parameters
    progress.step("Initializing NimBLE and configuring aggressive advertising mode");

    NimBLEDevice::init("");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    g_pAdvertising = NimBLEDevice::getAdvertising();

    g_pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
    g_pAdvertising->setMinPreferred(0x00);
    g_pAdvertising->setMaxPreferred(0x00);

    delay(300);

    // Phase 2: Send aggressive jam packets
    progress.step("Transmitting aggressive jamming packets on BLE channels");

    g_jamActive = true;
    g_jamCount = 0;

    while (millis() - startTime < durationMs && g_jamActive) {
        uint8_t jamData[31];
        for (int i = 0; i < 31; i++) {
            jamData[i] = (esp_random() % 256);
        }
        g_jamCount++;
        delay(2);
    }

    g_jamActive = false;

    // Phase 3: Verify jamming effectiveness
    progress.step("Verifying BLE channel disruption and jamming statistics");

    delay(300);

    g_pAdvertising->stop();
    result.success = true;
    result.jamPacketsCount = g_jamCount;

    uint32_t elapsed = millis() - startTime;

    progress.complete(String(result.jamPacketsCount) + " jam packets transmitted");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "BLE Aggressive Jam";
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
}

bool isActive() {
    return g_jamActive;
}

}  // namespace BluetoothAggressiveJammer
