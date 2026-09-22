#pragma once
#include <Arduino.h>
#include <vector>

namespace MagneticStripper {

// Magnetic stripe cards contain 3 tracks:
// Track 1: 210 bpi, ASCII format (80 chars max)
//   Format: %B[PAN][^][NAME][^][EXPIRY][^]?
// Track 2: 75 bpi, numeric format (40 digits max)
//   Format: [PAN];[EXPIRY][SERVICE CODE][DISCRETIONARY]?
// Track 3: 210 bpi, PIN encipherment (80 chars max)
//
// Encoding: FSK (Frequency Shift Keying)
//   - Bit: magnetic flux reversal
//   - Clock: 7kHz (track 1/3) or 5kHz (track 2)
//   - Detect via induction loop + ADC

struct StripeConfig {
    uint32_t scanDurationMs;
    bool captureMode;              // true=read, false=write
    bool analyzeAll3Tracks;        // Read all 3 or just Track2
    std::vector<uint8_t> dataToWrite;
};

struct StripeResult {
    bool success;
    std::vector<uint8_t> track1;
    std::vector<uint8_t> track2;
    std::vector<uint8_t> track3;
    String pan;                    // Primary Account Number (card #)
    String name;                   // Cardholder name
    String expiry;                 // Expiration date (MMYY)
    String error;
};

// Passive read: Swipe card through magnetic reader head
// Decodes FSK + Manchester encoding
// Extracts: PAN, name, expiry, service code
StripeResult readMagneticStripe(const StripeConfig& config);

// Active clone: Write captured data to blank magnetic stripe card
// Requires: Magnetic stripe writer module
// Success: High (90%+) if original read was clean
StripeResult writeStripeCard(const StripeConfig& config, const StripeResult& source);

// Analyze track encoding format
// Detects: IATA vs ABA format, encryption markers
StripeResult analyzeTrackFormat(const StripeResult& data);

// Extract sensitive data from Track 2 (most common)
// Parses: PAN, expiry, service code, CVC derivation
String parseTrack2(const std::vector<uint8_t>& track2Data);

}  // namespace MagneticStripper
