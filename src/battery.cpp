#include "battery.h"
#include "config.h"
#include "audit_log.h"

namespace {
// Default assumes a 1:1 divider (e.g. two 100k resistors) bringing a 4.2V
// max LiPo down under the ADC's ~3.3V range. Adjust to match your resistors:
// ratio = (R1 + R2) / R2, where R1 is battery-side, R2 is GND-side.
constexpr float DIVIDER_RATIO = 2.0f;
constexpr float EMPTY_V = 3.0f;
constexpr float FULL_V = 4.2f;
constexpr float CRITICAL_V = 3.1f;
constexpr float LOW_V = 3.5f;
constexpr uint8_t CRITICAL_PCT = 5;
constexpr uint8_t LOW_PCT = 15;

volatile Battery::BatteryState last_state = Battery::BatteryState::NORMAL;
volatile uint32_t last_warning_time = 0;
constexpr uint32_t WARNING_INTERVAL = 60000;  // Only warn every 60 seconds
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

BatteryState state() {
    float v = voltage();
    uint8_t p = percent();

    if (v < CRITICAL_V || p < CRITICAL_PCT) {
        return BatteryState::CRITICAL;
    } else if (v < LOW_V || p < LOW_PCT) {
        return BatteryState::WARNING;
    } else {
        return BatteryState::NORMAL;
    }
}

bool isLow() {
    BatteryState s = state();
    BatteryState prev = last_state;
    last_state = s;

    if (s == BatteryState::WARNING || s == BatteryState::CRITICAL) {
        if ((millis() - last_warning_time) > WARNING_INTERVAL) {
            char details[96];
            snprintf(details, sizeof(details), "voltage=%.2fV,percent=%d%%",
                     voltage(), percent());
            AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "Battery", details);
            last_warning_time = millis();
        }
        return true;
    }
    return false;
}

bool isCritical() {
    return state() == BatteryState::CRITICAL;
}

} // namespace Battery
