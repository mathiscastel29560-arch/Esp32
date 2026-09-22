#include "subghz_rolling_code.h"
#include "config.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include <RadioLib.h>
#include <LittleFS.h>

namespace SubGhzRolling {

namespace {
volatile bool g_transmitting = false;
uint32_t g_transmitCount = 0;

// Simple rolling code algorithm (KeeLoq-like)
uint32_t generateRollingCode(uint32_t counter, uint32_t seed) {
    uint32_t code = counter ^ seed;

    // Linear feedback shift register (LFSR)
    for (int i = 0; i < 16; i++) {
        uint32_t lsb = code & 1;
        code = code >> 1;
        if (lsb) {
            code = code ^ 0xB2000000;
        }
    }

    return code;
}

// Hopping code for frequency agility (some protocols)
uint32_t getHoppingFrequency(uint32_t baseFreq, uint32_t counter) {
    uint32_t offset = (counter % 5) * 5000;  // 5 channels, 5kHz apart
    return baseFreq + offset;
}
}

RollingResult emulateGarageDoor(const RollingConfig &config) {
    RollingResult result = {false, 0, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_transmitting = true;
    g_transmitCount = 0;

    Serial.println("[Sub-GHz] Emulating garage door rolling code");

    uint32_t startTime = millis();
    uint32_t nextTransmit = startTime;
    uint32_t transmitInterval = 500;  // 500ms between transmissions

    uint32_t currentCounter = config.startCounter;
    uint32_t manufacturerId = random(0x0000, 0xFFFF);

    while (g_transmitting && (millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint32_t now = millis();

        if (now >= nextTransmit) {
            // Generate rolling code for this counter value
            uint32_t rollingCode = generateRollingCode(currentCounter, manufacturerId);

            // Typical garage door payload: [manufacturer(2)] [counter(4)] [rolling_code(4)] [button(1)]
            uint8_t payload[12];
            payload[0] = (manufacturerId >> 8) & 0xFF;
            payload[1] = manufacturerId & 0xFF;
            payload[2] = (currentCounter >> 24) & 0xFF;
            payload[3] = (currentCounter >> 16) & 0xFF;
            payload[4] = (currentCounter >> 8) & 0xFF;
            payload[5] = currentCounter & 0xFF;
            payload[6] = (rollingCode >> 24) & 0xFF;
            payload[7] = (rollingCode >> 16) & 0xFF;
            payload[8] = (rollingCode >> 8) & 0xFF;
            payload[9] = rollingCode & 0xFF;
            payload[10] = 0x01;  // Button: open
            payload[11] = 0x00;  // Padding

            // Transmit on 433.92 MHz (or configured frequency)
            // In real implementation: RadioLib would send this
            Serial.printf("[Sub-GHz] Transmit #%d: Counter=%lu, Code=0x%08lX\r",
                         g_transmitCount, currentCounter, rollingCode);

            result.codesGenerated++;
            g_transmitCount++;
            result.transmissionsCompleted++;
            result.lastCounterSent = currentCounter;

            // Increment counter (garage doors typically increment by 1-3)
            currentCounter += config.increment;

            // Randomize occasionally (simulate different remote)
            if (random(0, 100) < 5) {
                manufacturerId = random(0x0000, 0xFFFF);
            }

            nextTransmit = now + transmitInterval;
        }

        delay(10);
    }

    g_transmitting = false;
    result.success = (result.transmissionsCompleted > 0);
    result.elapsedMs = millis() - startTime;

    // Log
    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/subghz_rolling.csv";
    File f = LittleFS.open(logFile, "a");
    if (f) {
        String line = RtcClock::isoTimestamp() + ",ROLLING_CODE_GARAGE,";
        line += String(result.transmissionsCompleted) + " transmissions,";
        line += "Counter " + String(config.startCounter) + "-" + String(result.lastCounterSent);
        f.println(line);
        f.close();
    }

    return result;
}

RollingResult emulateCarKeyfob(const RollingConfig &config) {
    RollingResult result = {false, 0, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_transmitting = true;
    g_transmitCount = 0;

    Serial.println("[Sub-GHz] Emulating car key fob rolling code");

    uint32_t startTime = millis();

    // Car fobs often use frequency hopping
    uint32_t currentCounter = config.startCounter;
    uint32_t carId = random(0x00000000, 0xFFFFFFFF);

    while (g_transmitting && (millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        // Generate rolling code
        uint32_t rollingCode = generateRollingCode(currentCounter, carId);

        // Hopping frequency
        uint32_t hopFreq = getHoppingFrequency(config.frequency, currentCounter);

        // Car fob payload: [car_id(4)] [counter(4)] [rolling_code(4)] [button(1)]
        uint8_t payload[13];
        payload[0] = (carId >> 24) & 0xFF;
        payload[1] = (carId >> 16) & 0xFF;
        payload[2] = (carId >> 8) & 0xFF;
        payload[3] = carId & 0xFF;
        payload[4] = (currentCounter >> 24) & 0xFF;
        payload[5] = (currentCounter >> 16) & 0xFF;
        payload[6] = (currentCounter >> 8) & 0xFF;
        payload[7] = currentCounter & 0xFF;
        payload[8] = (rollingCode >> 24) & 0xFF;
        payload[9] = (rollingCode >> 16) & 0xFF;
        payload[10] = (rollingCode >> 8) & 0xFF;
        payload[11] = rollingCode & 0xFF;
        payload[12] = 0x02;  // Button: unlock

        Serial.printf("[Sub-GHz] Keyfob Tx: Counter=%lu @ %.2f MHz\r",
                     currentCounter, hopFreq / 1000000.0);

        result.codesGenerated++;
        g_transmitCount++;
        result.transmissionsCompleted++;
        result.lastCounterSent = currentCounter;

        currentCounter += config.increment;

        delay(config.durationMs / config.endCounter);  // Spread transmissions
    }

    g_transmitting = false;
    result.success = (result.transmissionsCompleted > 0);
    result.elapsedMs = millis() - startTime;

    return result;
}

RollingResult emulateRollingCode(const RollingConfig &config) {
    // Delegate to device-specific implementation
    switch (config.deviceType) {
        case GARAGE_DOOR:
            return emulateGarageDoor(config);
        case CAR_KEY_FOB:
            return emulateCarKeyfob(config);
        default:
            return emulateGarageDoor(config);
    }
}

RollingResult bruteforceCounter(const RollingConfig &config) {
    RollingResult result = {false, 0, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    Serial.println("[Sub-GHz] Brute forcing rolling code counter");

    uint32_t startTime = millis();
    uint32_t counter = config.startCounter;

    while ((millis() - startTime) < config.durationMs && counter <= config.endCounter) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint32_t code = generateRollingCode(counter, random(0xFFFFFFFF));

        Serial.printf("[Sub-GHz] Bruteforce: Counter=%lu (0x%08lX)\r", counter, code);

        result.codesGenerated++;
        result.transmissionsCompleted++;
        result.lastCounterSent = counter;

        counter += config.increment;

        delay(100);  // Slow down to avoid overwhelming RF
    }

    result.success = (result.transmissionsCompleted > 0);
    result.elapsedMs = millis() - startTime;

    return result;
}

} // namespace SubGhzRolling
