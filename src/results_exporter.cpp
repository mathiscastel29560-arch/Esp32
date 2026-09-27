#include "results_exporter.h"
#include "logging_system.h"
#include <ctime>

ResultsExporter::ResultsExporter() {
  Logger::getInstance().info("Exporter", "Exporteur résultats initialisé");
}

void ResultsExporter::writeHTMLHeader(FILE* file, const char* title) {
  fprintf(file, "<!DOCTYPE html>\n");
  fprintf(file, "<html lang=\"fr\">\n");
  fprintf(file, "<head>\n");
  fprintf(file, "  <meta charset=\"UTF-8\">\n");
  fprintf(file, "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
  fprintf(file, "  <title>%s</title>\n", title);
  fprintf(file, "  <style>\n");
  fprintf(file, "    body { font-family: Segoe UI, Arial; margin: 20px; background: #f5f5f5; }\n");
  fprintf(file, "    .container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
  fprintf(file, "    h1 { color: #333; border-bottom: 3px solid #0066cc; padding-bottom: 10px; }\n");
  fprintf(file, "    h2 { color: #0066cc; margin-top: 30px; }\n");
  fprintf(file, "    .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }\n");
  fprintf(file, "    .stat-box { background: #f9f9f9; padding: 15px; border-radius: 5px; border-left: 4px solid #0066cc; }\n");
  fprintf(file, "    .stat-label { font-size: 12px; color: #666; text-transform: uppercase; }\n");
  fprintf(file, "    .stat-value { font-size: 28px; font-weight: bold; color: #0066cc; }\n");
  fprintf(file, "    .success { border-left-color: #28a745; color: #28a745; }\n");
  fprintf(file, "    .warning { border-left-color: #ffc107; color: #ffc107; }\n");
  fprintf(file, "    .error { border-left-color: #dc3545; color: #dc3545; }\n");
  fprintf(file, "    table { width: 100%%; border-collapse: collapse; margin-top: 20px; }\n");
  fprintf(file, "    th { background: #0066cc; color: white; padding: 12px; text-align: left; }\n");
  fprintf(file, "    td { padding: 10px; border-bottom: 1px solid #ddd; }\n");
  fprintf(file, "    tr:hover { background: #f9f9f9; }\n");
  fprintf(file, "    .status-success { color: #28a745; font-weight: bold; }\n");
  fprintf(file, "    .status-failed { color: #dc3545; font-weight: bold; }\n");
  fprintf(file, "    .timestamp { color: #666; font-size: 12px; }\n");
  fprintf(file, "  </style>\n");
  fprintf(file, "</head>\n");
  fprintf(file, "<body>\n");
  fprintf(file, "  <div class=\"container\">\n");
}

void ResultsExporter::writeHTMLFooter(FILE* file) {
  fprintf(file, "  </div>\n");
  fprintf(file, "</body>\n");
  fprintf(file, "</html>\n");
}

void ResultsExporter::writeAttackStats(FILE* file, Attack* attack) {
  AttackStatus status = attack->getCurrentStatus();
  const char* statusStr = "UNKNOWN";
  const char* statusClass = "warning";

  switch (status) {
    case AttackStatus::SUCCESS:
      statusStr = "SUCCÈS";
      statusClass = "success";
      break;
    case AttackStatus::PARTIAL:
      statusStr = "PARTIEL";
      statusClass = "warning";
      break;
    case AttackStatus::FAILED:
      statusStr = "ÉCHOUÉ";
      statusClass = "error";
      break;
    case AttackStatus::ERROR:
      statusStr = "ERREUR";
      statusClass = "error";
      break;
    default:
      break;
  }

  fprintf(file, "    <div class=\"stats\">\n");
  fprintf(file, "      <div class=\"stat-box %s\">\n", statusClass);
  fprintf(file, "        <div class=\"stat-label\">Statut</div>\n");
  fprintf(file, "        <div class=\"stat-value\">%s</div>\n", statusStr);
  fprintf(file, "      </div>\n");
  fprintf(file, "      <div class=\"stat-box\">\n");
  fprintf(file, "        <div class=\"stat-label\">Résultats</div>\n");
  fprintf(file, "        <div class=\"stat-value\">%u</div>\n", attack->getResultCount());
  fprintf(file, "      </div>\n");
  fprintf(file, "      <div class=\"stat-box\">\n");
  fprintf(file, "        <div class=\"stat-label\">Nom Attaque</div>\n");
  fprintf(file, "        <div class=\"stat-value\" style=\"font-size: 16px;\">%s</div>\n", attack->getName());
  fprintf(file, "      </div>\n");
  fprintf(file, "    </div>\n");
}

void ResultsExporter::writeResultsTable(FILE* file, Attack* attack) {
  if (attack->getResultCount() == 0) {
    fprintf(file, "    <p style=\"color: #666; font-style: italic;\">Aucun résultat</p>\n");
    return;
  }

  fprintf(file, "    <table>\n");
  fprintf(file, "      <thead>\n");
  fprintf(file, "        <tr>\n");
  fprintf(file, "          <th>#</th>\n");
  fprintf(file, "          <th>Description</th>\n");
  fprintf(file, "          <th>Type</th>\n");
  fprintf(file, "          <th>RSSI</th>\n");
  fprintf(file, "          <th>Données</th>\n");
  fprintf(file, "        </tr>\n");
  fprintf(file, "      </thead>\n");
  fprintf(file, "      <tbody>\n");

  for (uint16_t i = 0; i < attack->getResultCount(); i++) {
    const AttackResult* result = attack->getResult(i);
    if (!result) continue;

    const char* typeStr = "UNKNOWN";
    switch (result->type) {
      case ResultType::SCAN:
        typeStr = "SCAN";
        break;
      case ResultType::PACKET:
        typeStr = "PAQUET";
        break;
      case ResultType::CODE:
        typeStr = "CODE";
        break;
      case ResultType::STATUS:
        typeStr = "STATUT";
        break;
      case ResultType::ERROR:
        typeStr = "ERREUR";
        break;
    }

    fprintf(file, "        <tr>\n");
    fprintf(file, "          <td>%u</td>\n", i + 1);
    fprintf(file, "          <td>%s</td>\n", result->data);
    fprintf(file, "          <td>%s</td>\n", typeStr);
    fprintf(file, "          <td>%d dBm</td>\n", result->rssi);
    fprintf(file, "          <td>%u bytes</td>\n", result->dataLength);
    fprintf(file, "        </tr>\n");
  }

  fprintf(file, "      </tbody>\n");
  fprintf(file, "    </table>\n");
}

void ResultsExporter::exportAttackToHTML(Attack* attack, const char* filename) {
  if (!attack) {
    Serial.println("❌ Attaque null");
    return;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    Serial.printf("❌ Impossible d'ouvrir %s\n", filename);
    return;
  }

  writeHTMLHeader(file, attack->getName());

  fprintf(file, "    <h1>Rapport d'Attaque: %s</h1>\n", attack->getName());
  fprintf(file, "    <p class=\"timestamp\">Généré automatiquement</p>\n");

  fprintf(file, "    <h2>📊 Statistiques</h2>\n");
  writeAttackStats(file, attack);

  fprintf(file, "    <h2>📋 Résultats Détaillés</h2>\n");
  writeResultsTable(file, attack);

  writeHTMLFooter(file);
  fclose(file);

  Serial.printf("✓ Rapport HTML exporté: %s\n", filename);
}

void ResultsExporter::exportAttackToJSON(Attack* attack, const char* filename) {
  if (!attack) {
    Serial.println("❌ Attaque null");
    return;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    Serial.printf("❌ Impossible d'ouvrir %s\n", filename);
    return;
  }

  fprintf(file, "{\n");
  fprintf(file, "  \"attack\": {\n");
  fprintf(file, "    \"name\": \"%s\",\n", attack->getName());
  fprintf(file, "    \"status\": %u,\n", (uint8_t)attack->getCurrentStatus());
  fprintf(file, "    \"resultCount\": %u,\n", attack->getResultCount());
  fprintf(file, "    \"results\": [\n");

  for (uint16_t i = 0; i < attack->getResultCount(); i++) {
    const AttackResult* result = attack->getResult(i);
    if (!result) continue;

    fprintf(file, "      {\n");
    fprintf(file, "        \"index\": %u,\n", i);
    fprintf(file, "        \"data\": \"%s\",\n", result->data);
    fprintf(file, "        \"type\": %u,\n", (uint8_t)result->type);
    fprintf(file, "        \"rssi\": %d,\n", result->rssi);
    fprintf(file, "        \"length\": %u\n", result->dataLength);
    fprintf(file, "      }%s\n", i < attack->getResultCount() - 1 ? "," : "");
  }

  fprintf(file, "    ]\n");
  fprintf(file, "  }\n");
  fprintf(file, "}\n");

  fclose(file);
  Serial.printf("✓ Rapport JSON exporté: %s\n", filename);
}

void ResultsExporter::exportAttackToCSV(Attack* attack, const char* filename) {
  if (!attack) {
    Serial.println("❌ Attaque null");
    return;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    Serial.printf("❌ Impossible d'ouvrir %s\n", filename);
    return;
  }

  fprintf(file, "Attaque,%s\n", attack->getName());
  fprintf(file, "Résultats Total,%u\n\n", attack->getResultCount());
  fprintf(file, "Index,Description,Type,RSSI,Données(bytes)\n");

  for (uint16_t i = 0; i < attack->getResultCount(); i++) {
    const AttackResult* result = attack->getResult(i);
    if (!result) continue;

    fprintf(file, "%u,\"%s\",%u,%d,%u\n",
            i + 1, result->data, (uint8_t)result->type, result->rssi, result->dataLength);
  }

  fclose(file);
  Serial.printf("✓ Rapport CSV exporté: %s\n", filename);
}

void ResultsExporter::exportMultipleToHTML(std::vector<Attack*>& attacks, const char* filename) {
  if (attacks.empty()) {
    Serial.println("❌ Aucune attaque à exporter");
    return;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    Serial.printf("❌ Impossible d'ouvrir %s\n", filename);
    return;
  }

  writeHTMLHeader(file, "Rapport Attaques Multiples");

  fprintf(file, "    <h1>📊 Rapport Attaques Multiples</h1>\n");
  fprintf(file, "    <p class=\"timestamp\">%u attaques exportées</p>\n", attacks.size());

  // Statistiques globales
  uint16_t totalResults = 0;
  uint16_t successCount = 0;

  for (auto attack : attacks) {
    totalResults += attack->getResultCount();
    if (attack->getCurrentStatus() == AttackStatus::SUCCESS ||
        attack->getCurrentStatus() == AttackStatus::PARTIAL) {
      successCount++;
    }
  }

  fprintf(file, "    <div class=\"stats\">\n");
  fprintf(file, "      <div class=\"stat-box\">\n");
  fprintf(file, "        <div class=\"stat-label\">Attaques Réussies</div>\n");
  fprintf(file, "        <div class=\"stat-value\">%u/%u</div>\n", successCount, attacks.size());
  fprintf(file, "      </div>\n");
  fprintf(file, "      <div class=\"stat-box\">\n");
  fprintf(file, "        <div class=\"stat-label\">Résultats Totaux</div>\n");
  fprintf(file, "        <div class=\"stat-value\">%u</div>\n", totalResults);
  fprintf(file, "      </div>\n");
  fprintf(file, "    </div>\n");

  // Détail chaque attaque
  for (size_t i = 0; i < attacks.size(); i++) {
    Attack* attack = attacks[i];
    fprintf(file, "    <h2>[%u] %s</h2>\n", i + 1, attack->getName());
    writeAttackStats(file, attack);
    writeResultsTable(file, attack);
  }

  writeHTMLFooter(file);
  fclose(file);

  Serial.printf("✓ Rapport multi-attaques exporté: %s\n", filename);
}

void ResultsExporter::generateReport(std::vector<Attack*>& attacks, const char* filename) {
  // Génère rapport dans 3 formats
  char htmlFile[128], jsonFile[128], csvFile[128];

  snprintf(htmlFile, 127, "%s_report.html", filename);
  snprintf(jsonFile, 127, "%s_data.json", filename);
  snprintf(csvFile, 127, "%s_results.csv", filename);

  exportMultipleToHTML(attacks, htmlFile);

  // Export individuels JSON/CSV
  for (auto attack : attacks) {
    char attackFile[128];
    snprintf(attackFile, 127, "%s_%s.json", filename, attack->getName());
    exportAttackToJSON(attack, attackFile);
  }

  Serial.printf("✓ Rapports complets exportés (%s*)\n", filename);
}
