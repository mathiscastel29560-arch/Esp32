#include "ir_tools.h"
#include "config.h"
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

namespace {
IRrecv irrecv(PIN_IR_RX, 1024, 15, true);
IRsend irsend(PIN_IR_TX);

struct PowerCode {
    decode_type_t protocol;
    uint64_t code;
    uint16_t bits;
};

// Small, best-effort set of commonly-published TV power-toggle codes.
// Success varies by brand/model/region — use IrTools::learn() on the
// actual remote for a reliable, specific result.
const PowerCode POWER_CODES[] = {
    {NEC, 0xE0E040BF, 32},  // Samsung TV power
    {SONY, 0xA90, 12},      // Sony TV power (SIRC)
    {RC5, 0x0C, 12},        // Philips/RC5-family TV power
};
}

namespace IrTools {

void begin() {
    irrecv.enableIRIn();
    irsend.begin();
}

void sendUniversalPowerToggle() {
    for (auto &pc : POWER_CODES) {
        irsend.send(pc.protocol, pc.code, pc.bits);
        delay(400);
    }
}

IrCapture learn(uint32_t timeoutMs) {
    IrCapture cap;
    decode_results results;
    irrecv.enableIRIn();

    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (irrecv.decode(&results)) {
            for (uint16_t i = 1; i < results.rawlen; i++) {
                cap.rawUs.push_back(results.rawbuf[i] * kRawTick);
            }
            irrecv.resume();
            break;
        }
        delay(5);
    }
    return cap;
}

void replay(const IrCapture &capture) {
    if (capture.rawUs.empty()) return;
    irsend.sendRaw(capture.rawUs.data(), capture.rawUs.size(), 38);
}

} // namespace IrTools
