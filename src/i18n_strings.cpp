#include "i18n_strings.h"

const char* I18nStrings::EN_STRINGS[][2] = {
  {"title", "ESP32 Audit Dashboard"},
  {"system_status", "System Status"},
  {"uptime", "Uptime"},
  {"free_memory", "Free Memory"},
  {"audit_stats", "Audit Statistics"},
  {"total_audits", "Total Audits"},
  {"success_rate", "Success Rate"},
  {"total_devices", "Total Devices"},
  {"settings", "Settings"},
  {"brightness", "Brightness"},
  {"volume", "Volume"},
  {"save", "Save"},
  {"recent_activity", "Recent Activity"},
  {"dashboard", "Dashboard"},
  {"dark_mode", "Dark Mode"},
  {"language", "Language"},
  {"export_csv", "Export CSV"},
  {"export_json", "Export JSON"},
  {"filters", "Filters"},
  {"alerts", "Alerts"},
  {"battery", "Battery"},
  {"status", "Status"},
  {"error", "Error"},
  {"warning", "Warning"},
  {"info", "Info"},
  {"success", "Success"},
  {nullptr, nullptr}
};

const char* I18nStrings::FR_STRINGS[][2] = {
  {"title", "Tableau de Bord d'Audit ESP32"},
  {"system_status", "État du Système"},
  {"uptime", "Temps de fonctionnement"},
  {"free_memory", "Mémoire libre"},
  {"audit_stats", "Statistiques d'Audit"},
  {"total_audits", "Audits totaux"},
  {"success_rate", "Taux de réussite"},
  {"total_devices", "Appareils totaux"},
  {"settings", "Paramètres"},
  {"brightness", "Luminosité"},
  {"volume", "Volume"},
  {"save", "Enregistrer"},
  {"recent_activity", "Activité récente"},
  {"dashboard", "Tableau de bord"},
  {"dark_mode", "Mode sombre"},
  {"language", "Langue"},
  {"export_csv", "Exporter CSV"},
  {"export_json", "Exporter JSON"},
  {"filters", "Filtres"},
  {"alerts", "Alertes"},
  {"battery", "Batterie"},
  {"status", "État"},
  {"error", "Erreur"},
  {"warning", "Avertissement"},
  {"info", "Info"},
  {"success", "Succès"},
  {nullptr, nullptr}
};

const char* I18nStrings::ES_STRINGS[][2] = {
  {"title", "Panel de Control de Auditoría ESP32"},
  {"system_status", "Estado del Sistema"},
  {"uptime", "Tiempo de funcionamiento"},
  {"free_memory", "Memoria libre"},
  {"audit_stats", "Estadísticas de auditoría"},
  {"total_audits", "Auditorías totales"},
  {"success_rate", "Tasa de éxito"},
  {"total_devices", "Dispositivos totales"},
  {"settings", "Configuración"},
  {"brightness", "Brillo"},
  {"volume", "Volumen"},
  {"save", "Guardar"},
  {"recent_activity", "Actividad reciente"},
  {"dashboard", "Panel de control"},
  {"dark_mode", "Modo oscuro"},
  {"language", "Idioma"},
  {"export_csv", "Exportar CSV"},
  {"export_json", "Exportar JSON"},
  {"filters", "Filtros"},
  {"alerts", "Alertas"},
  {"battery", "Batería"},
  {"status", "Estado"},
  {"error", "Error"},
  {"warning", "Advertencia"},
  {"info", "Información"},
  {"success", "Éxito"},
  {nullptr, nullptr}
};

std::string I18nStrings::get(const std::string& key, const std::string& lang) {
  const char* (*strings)[2] = nullptr;

  if (lang == "fr") {
    strings = FR_STRINGS;
  } else if (lang == "es") {
    strings = ES_STRINGS;
  } else {
    strings = EN_STRINGS;
  }

  if (!strings) return key;

  for (int i = 0; strings[i][0] != nullptr; i++) {
    if (key == strings[i][0]) {
      return strings[i][1];
    }
  }

  return key;
}

void I18nStrings::setLanguage(const std::string& lang) {
  if (lang == "en" || lang == "fr" || lang == "es") {
    currentLanguage = lang;
  }
}
