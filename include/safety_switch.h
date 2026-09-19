#pragma once
#include <Arduino.h>

// Hardware TX interlock. The physical slide switch on GPIO39 is read
// directly (no software override) so that every transmit-capable feature
// — sub-GHz capture replay, Wi-Fi deauth, beacon spam, and the evil-portal
// captive page — is physically gated: flipping the switch is the only way
// to arm them, which is the point of putting it on real hardware instead
// of a checkbox in the web UI.
namespace SafetySwitch {

void begin();
bool isArmed();

} // namespace SafetySwitch
