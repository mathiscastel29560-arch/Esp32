#include "parameter_help.h"

const ParameterHelp ParameterHelpSystem::HELP_DATABASE[] = {
  // 0: Brightness
  {
    "Luminosité",
    "Contrôle la luminosité de l'écran",
    "Ajuste la luminosité du display ILI9341. Plus haut = plus clair mais consomme plus.\n"
    "Influence: autonomie batterie, visibilité en plein soleil.",
    "0-100%",
    "En extérieur: 80-100% | Intérieur: 30-60% | Nuit: 10-30%"
  },

  // 1: Volume
  {
    "Volume",
    "Volume des notifications audio",
    "Contrôle le niveau de volume pour les alertes et retours sonores.\n"
    "0 = muet, 100 = maximum.",
    "0-100%",
    "Discrétion: 20-40% | Normal: 60-80% | Alarme: 100%"
  },

  // 2: Contrast
  {
    "Contraste",
    "Contraste de l'écran (50-150%)",
    "Ajuste le contraste pour une meilleure lisibilité.\n"
    "100% = normal, <100 = moins contrasté, >100 = plus marqué.",
    "50-150%",
    "Yeux fatigués: 50-80% | Normal: 100% | Fort ensoleillement: 120-150%"
  },

  // 3: Color Inversion
  {
    "Inversion Couleur",
    "Active le mode sombre (inversion)",
    "Inverse les couleurs: blanc→noir, noir→blanc. Utile:\n"
    "- Mode sombre pour les yeux (nuit)\n"
    "- OLED/e-ink efficiency\n"
    "- Accessibilité",
    "ON/OFF",
    "Nuit: ON | Jour: OFF | Vision: selon préférence"
  },

  // 4: Screen Timeout
  {
    "Timeout Écran",
    "Durée avant extinction automatique",
    "Éteint l'écran après inactivité pour économiser batterie.\n"
    "Options: 0 (jamais), 60s, 180s, 300s, 600s.",
    "0 (jamais), 60s, 180s, 300s, 600s",
    "Usage actif: 0 | Normal: 300s (5min) | Batterie faible: 60s"
  },

  // 5: Battery Saving Mode
  {
    "Mode Batterie",
    "Stratégie d'économie d'énergie",
    "Trois modes:\n"
    "NORMAL: Performance complète\n"
    "POWER_SAVING: CPU réduit, WiFi sporadique\n"
    "ULTRA_POWER_SAVING: Minimum vital, scan lents",
    "NORMAL / POWER_SAVING / ULTRA",
    "Charge: NORMAL | Batterie: POWER_SAVING | Critique: ULTRA"
  },

  // 6: WiFi Power Saving
  {
    "WiFi Power Save",
    "Réduit consommation WiFi",
    "Active le mode d'économie WiFi de l'ESP32.\n"
    "Moins réactif mais consomme beaucoup moins.\n"
    "Utile pour les scans longue durée.",
    "ON/OFF",
    "Scans rapides: OFF | Scans longs: ON | Batterie faible: ON"
  },

  // 7: CPU Frequency
  {
    "Fréquence CPU",
    "Vitesse du processeur (80/160/240 MHz)",
    "Trois niveaux:\n"
    "80MHz (Low): Économe, lent\n"
    "160MHz (Normal): Équilibré\n"
    "240MHz (High): Rapide, consomme plus",
    "80MHz / 160MHz / 240MHz",
    "Batterie faible: 80MHz | Normal: 160MHz | Urgent: 240MHz"
  },

  // 8: Sleep Mode
  {
    "Mode Veille",
    "Suspend le système après timeout",
    "Met l'ESP32 en deep sleep (consommation minimale).\n"
    "Wakeable via RTC alarm si timeout configuré.\n"
    "Très efficace pour batterie.",
    "ON/OFF + timeout",
    "Tournée: ON (600s) | Démonstration: OFF | Batterie: ON"
  },

  // 9: Bluetooth
  {
    "Bluetooth",
    "Bascule Bluetooth ON/OFF",
    "Active/désactive le module Bluetooth.\n"
    "Important: consomme batterie même inactif.\n"
    "Désactiver si pas utilisé.",
    "ON/OFF",
    "Usage local: ON | Batterie faible: OFF"
  },

  // 10: USB Charging
  {
    "Chargement USB",
    "Détection charge USB",
    "Détecte si branché en USB.\n"
    "Utilisé pour ajuster power profiles.\n"
    "Désactiver si faux positif sur GPIO7.",
    "ON/OFF",
    "Normalement: ON | Batterie externe: peut être OFF"
  },

  // 11: DEBUG Mode
  {
    "Mode DEBUG",
    "Affiche logs détaillés",
    "Active les messages de debug sur serial.\n"
    "Utile pour troubleshooting.\n"
    "Consomme un peu plus de ressources.",
    "ON/OFF",
    "Troubleshooting: ON | Production: OFF"
  },

  // 12: Reset to Defaults
  {
    "Réinitialiser",
    "Remet tous les paramètres par défaut",
    "Restaure les valeurs par défaut pour TOUS les paramètres.\n"
    "Action irréversible! Les changements actuels seront perdus.",
    "Confirmation requise",
    "Utiliser si configuration corrompue ou bugs"
  },

  // 13: Show Report
  {
    "Afficher Rapport",
    "Affiche rapport complet sur serial",
    "Imprime tous les paramètres actuels sur le port serial.\n"
    "Format: tableau formaté avec icônes.\n"
    "Utile pour diagnostic.",
    "N/A",
    "Toujours disponible pour vérifier config actuelle"
  }
};

const ParameterHelp* ParameterHelpSystem::getParameterHelp(int index) const {
  if (index >= 0 && index < HELP_COUNT) {
    return &HELP_DATABASE[index];
  }
  return nullptr;
}

const ParameterHelp* ParameterHelpSystem::getParameterHelpByName(const char* name) const {
  for (int i = 0; i < HELP_COUNT; i++) {
    if (String(HELP_DATABASE[i].name).equals(name)) {
      return &HELP_DATABASE[i];
    }
  }
  return nullptr;
}

void ParameterHelpSystem::printParameterHelp(int index) const {
  const ParameterHelp* help = getParameterHelp(index);
  if (!help) return;

  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.printf("║ AIDE: %s\n", help->name);
  Serial.println("╚════════════════════════════════════════╝");

  Serial.printf("\n📖 Description:\n%s\n\n", help->fullDesc);
  Serial.printf("📊 Plage:\n%s\n\n", help->range);
  Serial.printf("💡 Conseils:\n%s\n\n", help->tips);

  Serial.println("");
}

std::string ParameterHelpSystem::getQuickHelp(int index) const {
  const ParameterHelp* help = getParameterHelp(index);
  if (!help) return "";
  return std::string(help->shortDesc);
}
