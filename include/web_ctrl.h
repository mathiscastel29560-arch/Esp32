#pragma once
#include <Arduino.h>

/// HTTP REST API Control Interface for Audit Logger
///
/// Serves on port 8080 (not 80) to avoid collision with EvilPortal's captive-portal.
/// Connect to the SoftAP (see WifiTools::begin for SSID/password) and browse to
/// http://192.168.4.1:8080/
///
/// ENDPOINTS SUMMARY:
///
/// STATUS & SYSTEM
///   GET  /                    - HTML control panel
///   GET  /api/status          - Device status (battery, GPS, clients, time)
///
/// WiFi ATTACKS
///   GET  /api/wifi/scan                         - Scan available networks
///   GET  /api/wifi/sniff?bssid=XX:XX&channel=X&ms=5000 - Sniff clients on network
///   POST /api/wifi/deauth?bssid=XX:XX&client=YY:YY&channel=X&frames=N - Deauth attack
///   POST /api/wifi/beacon/start?ssids=SSID1,SSID2&hop=true - Beacon spam
///   POST /api/wifi/beacon/stop                  - Stop beacon spam
///   GET  /api/wifi/beacon/status                - Beacon spam status
///
/// BLE ATTACKS & SCANNING
///   GET  /api/ble/scan?seconds=5                - BLE device discovery
///   POST /api/ble/gatt-audit?address=XX:XX:XX  - Security audit of BLE device
///   POST /api/ble/fuzz?address=XX:XX:XX        - Fuzz BLE device (protocol testing)
///   POST /api/ble/spam/start                    - Start BLE spam detection/attack
///   POST /api/ble/spam/stop                     - Stop BLE spam
///   GET  /api/ble/spam/check                    - Check BLE spam alerts
///
/// RF ATTACKS & SCANNING
///   GET  /api/nrf24/scan?samples=50             - NRF24 channel activity scan
///   GET  /api/subghz/rssi?freq=433.92           - Read RSSI at frequency (MHz)
///   POST /api/subghz/record?freq=433.92&ms=8000 - Record Sub-GHz signal
///   POST /api/subghz/replay                     - Replay last recorded signal
///
/// EVIL PORTAL (WiFi Phishing)
///   POST /api/portal/start?ssid=FAKE&maxMs=600000 - Start fake AP
///   POST /api/portal/stop                       - Stop fake AP
///   GET  /api/portal/status                     - Portal active status
///   GET  /api/portal/log                        - CSV of captured credentials
///
/// INFRARED
///   POST /api/ir/power                          - Send universal TV power toggle
///   POST /api/ir/learn?ms=5000                  - Learn IR signal (capture)
///   POST /api/ir/replay                         - Replay learned IR signal
///
/// WARDRIVING (Location-based WiFi logging)
///   POST /api/wardrive/snapshot                 - Capture GPS + WiFi networks
///   GET  /api/wardrive/log                      - CSV of locations and networks
///
/// RESPONSE FORMAT:
/// Most endpoints return JSON. Status codes:
///   200 - Success (see JSON for details)
///   404 - Endpoint not found
///   500 - Internal error
///
/// SAFETY:
/// All destructive attacks (deauth, beacon spam, evil portal) require manual
/// confirmation: hold the BACK button on the device to arm TX. See TxArm namespace.
///
namespace WebCtrl {

void begin();
void loop(); // call every iteration of the main loop
String lastAction(); // for the OLED status line

} // namespace WebCtrl
