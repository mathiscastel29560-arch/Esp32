#include "magnetic_stripper.h"
#include "tx_arm.h"

namespace MagneticStripper {

// Magnetic Stripe Encoding:
// Track 1: 7-bit ASCII, 210 bits per inch
// Track 2: 5-bit binary + parity, 75 bpi
// Track 3: 7-bit + encryption, 210 bpi
//
// Detection: Magnetic flux changes via reed switch or Hall effect sensor
// Decoding: FSK (Frequency Shift Keying) or Manchester encoding

StripeResult readMagneticStripe(const StripeConfig& config) {
    StripeResult result;
    result.success = false;

    Serial.printf("[Mag Stripper] Scanning stripe (%lums)\n", config.scanDurationMs);

    uint32_t startTime = millis();

    // Simulate magnetic stripe reading
    // Real implementation: Analog input from magnetic head + ADC
    // Decode FSK: detect frequency transitions at ~7kHz and ~5kHz

    // Simulated Track 2 data (most common, often unencrypted)
    // Format: [PAN];[EXPIRY][SERVICE_CODE][DISCRETIONARY]?
    String track2Sim = "4532123456789123=2512300112345678?";

    // Parse Track 2
    if (track2Sim.indexOf(';') != -1) {
        int semicolonPos = track2Sim.indexOf(';');
        result.pan = track2Sim.substring(0, semicolonPos);

        String afterPAN = track2Sim.substring(semicolonPos + 1);
        if (afterPAN.length() >= 4) {
            result.expiry = afterPAN.substring(0, 4);  // MMYY
            result.expiry = result.expiry.substring(0, 2) + "/" + result.expiry.substring(2);
        }
    }

    // Simulated Track 1 (contains name)
    // Format: %B[PAN][^][NAME][^][EXPIRY]...?
    result.name = "JOHN DOE";

    // Fill track vectors
    for (char c : track2Sim) {
        result.track2.push_back((uint8_t)c);
    }
    for (char c : result.pan) {
        result.track1.push_back((uint8_t)c);
    }

    result.success = true;

    Serial.printf("[Mag Stripper] Read successful\n");
    Serial.printf("  PAN: %s\n", result.pan.c_str());
    Serial.printf("  Name: %s\n", result.name.c_str());
    Serial.printf("  Expiry: %s\n", result.expiry.c_str());

    return result;
}

StripeResult writeStripeCard(const StripeConfig& config, const StripeResult& source) {
    StripeResult result;
    result.success = false;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (source.track2.empty()) {
        result.error = "No source data";
        return result;
    }

    Serial.printf("[Mag Writer] Writing stripe...\n");
    Serial.printf("  PAN: %s\n", source.pan.c_str());
    Serial.printf("  Expiry: %s\n", source.expiry.c_str());

    // Magnetic stripe writer module (e.g., MSR605/609)
    // Coercivity: 4000 Oe (standard HiCo) or 2750 Oe (LoCo)

    // Encode and write each track
    uint8_t writeAttempts = 0;
    const uint8_t MAX_ATTEMPTS = 3;

    while (writeAttempts < MAX_ATTEMPTS) {
        Serial.printf("[Mag Writer] Attempt %d\r", writeAttempts + 1);

        // Simulate writing Track 2 (most critical)
        // Real: PWM pulse train at track coercivity

        // Simulate Track 1 and Track 3
        if ((esp_random() % 100) < 85) {  // 85% success rate
            result.success = true;
            result.track1 = source.track1;
            result.track2 = source.track2;
            result.track3 = source.track3;
            result.pan = source.pan;
            result.name = source.name;
            result.expiry = source.expiry;
            break;
        }

        writeAttempts++;
        delay(500);  // Wait between retries
    }

    if (result.success) {
        Serial.printf("\n[Mag Writer] Write successful after %d attempt(s)\n", writeAttempts + 1);
    } else {
        result.error = "Write failed after max attempts";
        Serial.printf("\n[Mag Writer] Write failed\n");
    }

    return result;
}

StripeResult analyzeTrackFormat(const StripeResult& data) {
    StripeResult result = data;
    result.success = true;

    Serial.printf("[Mag Analyzer] Analyzing format...\n");

    // Detect format based on content
    if (!data.pan.empty()) {
        uint8_t firstChar = data.pan[0];
        // PAN first digit indicates card type:
        // 4 = Visa, 5 = Mastercard, 6 = Discover, 3 = Amex
        switch (firstChar) {
            case '4': Serial.printf("  Card type: Visa\n"); break;
            case '5': Serial.printf("  Card type: Mastercard\n"); break;
            case '6': Serial.printf("  Card type: Discover\n"); break;
            case '3': Serial.printf("  Card type: American Express\n"); break;
            default: Serial.printf("  Card type: Unknown\n");
        }
    }

    // Check for encryption markers
    bool hasEncryption = false;
    for (uint8_t b : data.track2) {
        if (b > 127) {  // High bit set = encrypted
            hasEncryption = true;
            break;
        }
    }
    Serial.printf("  Encryption: %s\n", hasEncryption ? "Yes" : "No");

    // Track quality assessment
    Serial.printf("  Track 1 data: %zu bytes\n", data.track1.size());
    Serial.printf("  Track 2 data: %zu bytes\n", data.track2.size());
    Serial.printf("  Track 3 data: %zu bytes\n", data.track3.size());

    return result;
}

String parseTrack2(const std::vector<uint8_t>& track2Data) {
    String result = "";

    // Track 2 format: [PAN];[EXPIRY][SERVICE][DISCRETIONARY]?
    // Separator: ;
    // Terminator: ?

    for (uint8_t b : track2Data) {
        if (b >= 32 && b < 127) {
            result += (char)b;
        }
    }

    return result;
}

}  // namespace MagneticStripper
