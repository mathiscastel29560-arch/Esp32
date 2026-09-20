#include "mifare_bruteforce.h"
namespace MifareBruteforce {
BruteResult bruteForceKeys(uint8_t sector, uint16_t timeoutMs) {
    return {false, "", 0, 0, "Mifare bruteforce stub - requires PN532 + auth integration"};
}
std::vector<String> getCommonKeys() {
    return {"FFFFFFFFFFFF", "000000000000", "A0A1A2A3A4A5", "B0B1B2B3B4B5"};
}
} // namespace MifareBruteforce
