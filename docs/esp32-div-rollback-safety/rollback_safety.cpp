// Rollback safety verification function
// Added to enable OTA dual-boot rollback protection for ESP32-DIV
// This function is called by the bootloader to verify if rollback to ota_1 should be allowed
// Returning true indicates the current firmware is valid and should not be rolled back
// Origin: Added by Claude for enhanced OTA safety in field deployments
// Not part of upstream cifertech/esp32-div project

extern "C" bool verifyRollbackLater() {
    // Return true to indicate this firmware is valid
    // Return false to trigger rollback to ota_1 on next boot
    return true;
}
