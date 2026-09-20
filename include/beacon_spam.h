#pragma once
#include <Arduino.h>
#include <vector>

// Broadcasts fake 802.11 beacon frames for the SSIDs you give it — meant
// for checking whether a WIDS/rogue-AP detector on your own network
// notices them. There is no built-in SSID list: you always supply the
// names, since this radiates over the air to any Wi-Fi scanner in range,
// not just your own equipment — keep it inside a controlled space.
// Requires the hardware safety switch to be armed to start.
namespace BeaconSpam {

bool start(const std::vector<String> &ssids, bool hopChannels = true);
void stop();
void loop(); // call every main-loop iteration while active
bool active();

} // namespace BeaconSpam
