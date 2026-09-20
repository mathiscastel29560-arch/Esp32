#pragma once

// Dual-boot switching between this firmware (ota_0) and a separately
// flashed, unmodified build of Bruce (ota_1) -- see partitions_16mb.csv
// and README.md "Dual-boot with Bruce". Both firmwares share one
// bootloader with ESP-IDF's app-rollback feature enabled: this firmware
// calls markValid() once it has booted successfully, so the bootloader
// never rolls it back. Bruce is built unmodified and never calls the
// rollback-cancel API, so if you power-cycle while Bruce is running, the
// bootloader treats that boot as never having been confirmed and reverts
// the boot partition back to this firmware on its own -- no menu action,
// no button combo, just cycle power.
namespace DualBoot {

// Call once, early in setup(), after the firmware has reached a point
// it's fair to call "booted successfully". Cancels any pending rollback
// so the bootloader keeps booting this firmware normally.
void markValid();

// Points the boot partition at ota_1 (Bruce) and restarts into it.
// Returns false (without restarting) if the ota_1 partition can't be
// found, e.g. Bruce was never flashed there.
bool bootIntoBruce();

} // namespace DualBoot
