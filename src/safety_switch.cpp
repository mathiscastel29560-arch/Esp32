#include "safety_switch.h"
#include "config.h"

namespace SafetySwitch {

void begin() {
    pinMode(PIN_SAFETY_SWITCH, INPUT_PULLDOWN);
}

bool isArmed() {
    return digitalRead(PIN_SAFETY_SWITCH) == HIGH;
}

} // namespace SafetySwitch
