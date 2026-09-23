#pragma once

// Physical dead-man's-switch interlock for every TX-capable feature
// (Wi-Fi deauth, beacon spam, evil portal, sub-GHz replay): the action only
// fires while the BACK button is physically held down on the device at the
// exact moment it's triggered — whether that trigger came from the
// on-device menu or an HTTP request from the web panel. Releasing BACK (or
// never having pressed it) blocks the action.
//
// This replaces the old dedicated slide-switch interlock: that switch is
// now the device's power switch (wired to the battery, not a GPIO — see
// config.h), so there's no longer a spare hardware switch to dedicate to
// this job. Holding a button at the moment of firing is a weaker guarantee
// than a switch you set in advance, but it still requires a deliberate,
// simultaneous physical action at the device — not just a UI checkbox.
namespace TxArm {

bool isArmed();

} // namespace TxArm
