#pragma once
#include <Arduino.h>

// 4 momentary buttons wired to GND (internal pull-ups) for on-device menu
// navigation — an alternative to the Wi-Fi web panel when you don't want
// to pull out a phone.
namespace Buttons {

enum Button { NONE = 0, UP, DOWN, SELECT, BACK };

void begin();

// Debounced, edge-triggered: returns the button that was *just* pressed,
// or NONE most calls. Call every main-loop iteration.
Button poll();

// Raw, instantaneous level check (no debounce/edge logic) — is this button
// physically held down right now? Used by tx_arm.h as a dead-man's-switch
// style interlock: TX-capable actions only fire while BACK is held.
bool isHeld(Button b);

} // namespace Buttons
