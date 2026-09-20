#include "custom_module.h"
#include "config.h"

// ============================================================================
// Colle/adapte ici le code que tu as trouvé.
//
// - Si c'est une bibliothèque Arduino (fichiers .h/.cpp séparés) : mets ses
//   fichiers dans un sous-dossier, par ex. src/thirdparty/NomLib/, et
//   #include "thirdparty/NomLib/NomLib.h" ci-dessous. Alternative plus
//   simple : ajoute-la comme dépendance PlatformIO (voir platformio.ini,
//   section lib_deps) avec l'URL GitHub du dépôt, et #include son header
//   normalement.
// - Si c'est un exemple/sketch .ino : le contenu de son setup() va dans
//   begin() ci-dessous, et le contenu de son loop() dans loop() ci-dessous.
// - Broches libres restantes sur ce brochage : GPIO 47 (voir HARDWARE.md
//   §8 "État des GPIO"). Si le programme a besoin de plusieurs broches,
//   il faudra soit en libérer une déjà utilisée, soit passer par un
//   expandeur I2C (le bus I2C n'a que le DS3231 dessus, adresse 0x68).
// ============================================================================

namespace CustomModule {

void begin() {
    // TODO: initialisation (ex: pinMode, Serial, config de la lib, etc.)
}

void loop() {
    // TODO: code appelé à chaque tour de la boucle principale
}

} // namespace CustomModule
