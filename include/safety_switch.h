#pragma once
#include <Arduino.h>

// Hardware TX interlock. The physical slide switch on GPIO39 is read
// directly (no software override) so that transmit-capable features
// (currently: sub-GHz capture replay on the CC1101, for re-testing a
// signal you already recorded from your own equipment) are physically
// gated: flipping the switch is the only way to arm them, which is the
// point of putting it on real hardware instead of a checkbox in the web UI.
namespace SafetySwitch {

void begin();
bool isArmed();

} // namespace SafetySwitch
