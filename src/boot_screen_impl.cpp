#include "boot_screen.h"
#include "display.h"
#include "buzzer.h"

namespace BootScreen {

void show(const String &title, const String &subtitle) {
    uint32_t bootStart = millis();
    uint8_t progress = 0;
    uint8_t stage = 0;

    const char* stages[] = {
        "Initializing radio...",
        "Mounting filesystem...",
        "Configuring WiFi...",
        "Starting BLE stack...",
        "Loading GPS module...",
        "Booting complete."
    };

    Serial.println("\n");
    Serial.println("╔════════════════════════════════════════════════════════════════╗");
    Serial.println("║                 🔫 ESP32-S3 AUDIT TOOL v1.0 🔫                 ║");
    Serial.println("║                                                                ║");
    Serial.println("║  Offensive Security Platform                                   ║");
    Serial.println("║  31 Attack Modules • 4 Defensive Tools                        ║");
    Serial.println("║  WiFi • BLE • Sub-GHz • NRF24 • IR • GPS                       ║");
    Serial.println("║                                                                ║");
    Serial.println("╚════════════════════════════════════════════════════════════════╝");
    Serial.println("");

    // Animated 6-second boot
    while (millis() - bootStart < 6000) {
        uint32_t elapsed = millis() - bootStart;
        progress = (elapsed / 60);  // 0-100 in 6 seconds
        stage = (elapsed / 1000);   // 0-5 stages

        if (stage > 5) stage = 5;

        // Clear screen and redraw
        if (elapsed % 500 == 0) {  // Update every 500ms
            Serial.print("\r[");

            // Progress bar
            for (int i = 0; i < 40; i++) {
                if (i < (progress / 2.5f)) {
                    Serial.print("█");
                } else if (i == (int)(progress / 2.5f)) {
                    Serial.print("▌");
                } else {
                    Serial.print("░");
                }
            }
            Serial.print("] ");
            Serial.print(progress);
            Serial.print("% - ");
            Serial.print(stages[stage]);
            Serial.print("  ");
        }

        delay(10);
    }

    Serial.println("\r[████████████████████████████████████████] 100% - Booting complete.  ");
    Serial.println("");
    Serial.println("✓ System ready!");
    Serial.println("");

    // Quick chirps
    Buzzer::chirpOk();
    delay(100);
    Buzzer::chirpOk();
}

}  // namespace BootScreen
