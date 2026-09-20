#pragma once
#include <Arduino.h>

namespace AudioEffects {

enum SoundType {
    SUCCESS_BEEP,      // Bip court joyeux
    EXPLOIT_ALERT,     // Alert de exploit réussi
    ACHIEVEMENT,       // Son de déverrouillage
    CHAOS_MODE,        // Son chaotique
    ERROR_BEEP         // Bip d'erreur
};

void playSound(SoundType type);
void playSuccessBeep();
void playExploitAlert();
void playAchievementUnlock();
void playChaosMode();
void playErrorBeep();
void initAudio();

} // namespace AudioEffects
