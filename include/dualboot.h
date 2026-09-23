#pragma once

// Dual-boot switching between this firmware (ota_0) and a separately
// flashed app in ota_1 -- see partitions_16mb.csv and README.md
// "Dual-boot". Both firmwares share one bootloader with ESP-IDF's
// app-rollback feature enabled: this firmware calls markValid() once it
// has booted successfully, so the bootloader never rolls it back.
//
// ota_1 currently holds ESP32-DIV (github.com/cifertech/esp32-div),
// flashed from its own precompiled binary -- see README.md. Unlike the
// Bruce board profile this project used to dual-boot (bruce-board/,
// still present for reference), ESP32-DIV's stock build has NOT been
// given our verifyRollbackLater() override: Arduino-ESP32's own
// initArduino() confirms its boot as valid within milliseconds via the
// default weak symbol, before ESP32-DIV's own code could ever crash. So
// the "power-cycle reverts to ota_0 automatically" safety net described
// below does NOT currently protect against an ESP32-DIV crash -- if it
// locks up, recover manually (erase the otadata partition, see
// README.md's dual-boot recovery section) rather than assuming a simple
// power-cycle will bring ota_0 back.
namespace DualBoot {

// Call once, early in setup(), after the firmware has reached a point
// it's fair to call "booted successfully". Cancels any pending rollback
// so the bootloader keeps booting this firmware normally.
void markValid();

// Points the boot partition at ota_1 (currently ESP32-DIV) and restarts
// into it. Returns false (without restarting) if the ota_1 partition
// can't be found, e.g. nothing was ever flashed there.
bool bootIntoEsp32Div();

} // namespace DualBoot
