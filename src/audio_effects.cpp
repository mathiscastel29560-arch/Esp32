#include "audio_effects.h"
#include "config.h"

namespace AudioEffects {

void initAudio() {
    // Stub: Intégration avec DAC/PWM/buzzer ESP32
    // Sur GPIO 26 (DAC1) par exemple
}

void playSound(SoundType type) {
    #if !ENABLE_AUDIO_EFFECTS
    return;
    #endif

    switch (type) {
        case SUCCESS_BEEP:
            playSuccessBeep();
            break;
        case EXPLOIT_ALERT:
            playExploitAlert();
            break;
        case ACHIEVEMENT:
            playAchievementUnlock();
            break;
        case CHAOS_MODE:
            playChaosMode();
            break;
        case ERROR_BEEP:
            playErrorBeep();
            break;
    }
}

void playSuccessBeep() {
    // Pattern: Bip court + silence + bip court (double tap)
    Serial.println("🔊 SUCCESS BEEP: beep-beep!");
    // digitalWrite(BUZZER_PIN, HIGH); delay(100);
    // digitalWrite(BUZZER_PIN, LOW);  delay(50);
    // digitalWrite(BUZZER_PIN, HIGH); delay(100);
    // digitalWrite(BUZZER_PIN, LOW);
}

void playExploitAlert() {
    // Pattern: 3 bips rapides (aaa-aaa-aaa)
    Serial.println("🔊 EXPLOIT ALERT: beep-beep-beep!");
    // for(int i = 0; i < 3; i++) {
    //     digitalWrite(BUZZER_PIN, HIGH); delay(80);
    //     digitalWrite(BUZZER_PIN, LOW);  delay(40);
    // }
}

void playAchievementUnlock() {
    // Pattern: Montée progressive (do-re-mi-fa)
    Serial.println("🔊 ACHIEVEMENT UNLOCK: dooo-reee-miii-faaaa!");
    // Bips avec délais croissants pour effet "ascending"
    // delay(100); beep(); delay(50);
    // delay(80);  beep(); delay(50);
    // delay(60);  beep(); delay(50);
    // delay(40);  beep();
}

void playChaosMode() {
    // Pattern: Bruit chaotique (bzzzzzt-bzzzzzt-bzzzt!)
    Serial.println("🔊 CHAOS MODE: bzzzzzzzzzzt!!!");
    // Alternance rapide pour créer du "bruit chaotique" amusant
    // for(int i = 0; i < 10; i++) {
    //     digitalWrite(BUZZER_PIN, HIGH); delay(30);
    //     digitalWrite(BUZZER_PIN, LOW);  delay(15);
    // }
}

void playErrorBeep() {
    // Pattern: Bip triste (beeeep... silence... beeeep)
    Serial.println("🔊 ERROR BEEP: sad-beep...");
    // digitalWrite(BUZZER_PIN, HIGH); delay(200);
    // digitalWrite(BUZZER_PIN, LOW);  delay(100);
    // digitalWrite(BUZZER_PIN, HIGH); delay(200);
    // digitalWrite(BUZZER_PIN, LOW);
}

} // namespace AudioEffects
