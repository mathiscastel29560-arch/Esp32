#include "battery.h"
#include "config.h"

namespace {
// Default assumes a 1:1 divider (e.g. two 100k resistors) bringing a 4.2V
// max LiPo down under the ADC's ~3.3V range. Adjust to match your resistors:
// ratio = (R1 + R2) / R2, where R1 is battery-side, R2 is GND-side.
constexpr float DIVIDER_RATIO = 2.0f;
constexpr float EMPTY_V = 3.0f;
constexpr float FULL_V = 4.2f;
}

namespace Battery {

void begin() {
    analogReadResolution(12);
}

float voltage() {
    uint32_t mv = analogReadMilliVolts(PIN_BATTERY_ADC);
    return (mv / 1000.0f) * DIVIDER_RATIO;
}

uint8_t percent() {
    float v = voltage();
    if (v <= EMPTY_V) return 0;
    if (v >= FULL_V) return 100;
    return (uint8_t)((v - EMPTY_V) / (FULL_V - EMPTY_V) * 100.0f);
}

} // namespace Battery
