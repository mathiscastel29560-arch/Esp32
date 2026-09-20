#pragma once
#include <Arduino.h>

// Emplacement pour ton propre ajout (bibliothèque/code trouvé ailleurs).
// Garde la même forme que les autres modules du projet (rtc_clock, ir_tools,
// etc.) : begin() une fois au démarrage, loop() à chaque tour de boucle
// principale. Renomme le namespace/fichier si tu veux un nom plus parlant.
namespace CustomModule {

void begin();
void loop();

} // namespace CustomModule
