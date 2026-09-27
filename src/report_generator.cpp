#include "report_generator.h"
#include "async_logger.h"

void ExploitationReportGenerator::startReport(const char* target, uint32_t timestamp) {
  targetName = target;
  reportStartTime = timestamp;
  clearReport();
  LOG_I("Starting report for target: %s", target);
}

void ExploitationReportGenerator::addExecutiveSummary(uint16_t vulnFound, uint16_t vulnExploited,
                                                      uint8_t maxAccess, float rate) {
  vulnFound = vulnFound;
  vulnExploited = vulnExploited;
  maxAccessLevel = maxAccess;
  successRate = rate;
  LOG_I("Added executive summary: %u/%u exploited (%.1f%% success)", vulnExploited, vulnFound, rate);
}

void ExploitationReportGenerator::addVulnerabilitySection(const char* vulnName, uint8_t severity,
                                                         const char* description, bool exploited) {
  vulnerabilities.push_back(vulnName);
  Serial.printf("[REPORT] Vuln: %s | Severity: %u/10 | Exploited: %s\n",
    vulnName, severity, exploited ? "YES" : "NO");
}

void ExploitationReportGenerator::addTimelineEntry(uint32_t timestamp, const char* event, uint8_t phase) {
  timeline.push_back({timestamp, event, phase});
}

void ExploitationReportGenerator::addRiskAssessment(uint8_t risk, const char* recommendations) {
  overallRisk = risk;
  LOG_I("Risk assessment: %u/100", risk);
}

void ExploitationReportGenerator::finishReport() {
  LOG_I("Report completed - %u vulnerabilities documented", vulnerabilities.size());
}

void ExploitationReportGenerator::printHTMLReport() {
  generateHTMLHeader();
  generateHTMLBody();
  generateHTMLFooter();
}

void ExploitationReportGenerator::printTextReport() {
  Serial.printf("\n========================================\n");
  Serial.printf("EXPLOITATION REPORT - %s\n", targetName);
  Serial.printf("========================================\n\n");

  Serial.printf("EXECUTIVE SUMMARY\n");
  Serial.printf("Target: %s\n", targetName);
  Serial.printf("Vulnerabilities Found: %u\n", vulnFound);
  Serial.printf("Vulnerabilities Exploited: %u\n", vulnExploited);
  Serial.printf("Exploitation Success Rate: %.1f%%\n", successRate);
  Serial.printf("Maximum Access Level: %u\n", maxAccessLevel);
  Serial.printf("Overall Risk Level: %u/100\n\n", overallRisk);

  Serial.printf("VULNERABILITIES DISCOVERED\n");
  for (uint16_t i = 0; i < vulnerabilities.size(); i++) {
    Serial.printf("%u. %s\n", i + 1, vulnerabilities[i]);
  }

  Serial.printf("\nATTACK TIMELINE\n");
  for (const auto& event : timeline) {
    Serial.printf("[%u] Phase %u: %s\n", event.timestamp, event.phase, event.description);
  }

  Serial.printf("\n========================================\n\n");
}

void ExploitationReportGenerator::printJSONReport() {
  Serial.println("{");
  Serial.printf("  \"target\": \"%s\",\n", targetName);
  Serial.printf("  \"timestamp\": %u,\n", reportStartTime);
  Serial.printf("  \"vulnerabilities_found\": %u,\n", vulnFound);
  Serial.printf("  \"vulnerabilities_exploited\": %u,\n", vulnExploited);
  Serial.printf("  \"success_rate\": %.2f,\n", successRate);
  Serial.printf("  \"max_access_level\": %u,\n", maxAccessLevel);
  Serial.printf("  \"overall_risk\": %u,\n", overallRisk);
  Serial.printf("  \"vulnerabilities\": [\n");

  for (uint16_t i = 0; i < vulnerabilities.size(); i++) {
    Serial.printf("    \"%s\"%s\n", vulnerabilities[i], (i < vulnerabilities.size() - 1) ? "," : "");
  }

  Serial.println("  ]");
  Serial.println("}");
}

void ExploitationReportGenerator::exportReport(const char* filename) {
  LOG_I("Exporting report to: %s", filename);
  // Implementation depends on file system availability
  printJSONReport();
}

void ExploitationReportGenerator::clearReport() {
  vulnFound = 0;
  vulnExploited = 0;
  maxAccessLevel = 0;
  successRate = 0.0f;
  overallRisk = 0;
  vulnerabilities.clear();
  timeline.clear();
  sections.clear();
}

void ExploitationReportGenerator::generateHTMLHeader() {
  Serial.println("<!DOCTYPE html>");
  Serial.println("<html>");
  Serial.println("<head>");
  Serial.printf("<title>Exploitation Report - %s</title>\n", targetName);
  Serial.println("<style>");
  Serial.println("body { font-family: Arial; margin: 20px; }");
  Serial.println(".header { border-bottom: 3px solid #333; padding-bottom: 10px; }");
  Serial.println(".summary { background: #f0f0f0; padding: 15px; margin: 10px 0; }");
  Serial.println(".vulnerability { border-left: 4px solid #ff6b6b; padding: 10px; margin: 10px 0; }");
  Serial.println(".timeline { margin: 20px 0; }");
  Serial.println(".event { border-left: 2px solid #4ecdc4; padding: 10px; margin: 5px 0; }");
  Serial.println("</style>");
  Serial.println("</head>");
  Serial.println("<body>");
}

void ExploitationReportGenerator::generateHTMLBody() {
  Serial.printf("<div class=\"header\"><h1>Exploitation Report: %s</h1></div>\n", targetName);

  Serial.println("<div class=\"summary\">");
  Serial.printf("<h2>Executive Summary</h2>\n");
  Serial.printf("<p>Target: <strong>%s</strong></p>\n", targetName);
  Serial.printf("<p>Vulnerabilities Found: <strong>%u</strong></p>\n", vulnFound);
  Serial.printf("<p>Vulnerabilities Exploited: <strong>%u</strong></p>\n", vulnExploited);
  Serial.printf("<p>Success Rate: <strong>%.1f%%</strong></p>\n", successRate);
  Serial.printf("<p>Maximum Access Level: <strong>%u</strong></p>\n", maxAccessLevel);
  Serial.printf("<p>Overall Risk: <strong>%u/100</strong></p>\n", overallRisk);
  Serial.println("</div>");

  Serial.println("<h2>Vulnerabilities</h2>");
  for (const auto& vuln : vulnerabilities) {
    Serial.printf("<div class=\"vulnerability\">%s</div>\n", vuln);
  }

  Serial.println("<h2>Attack Timeline</h2>");
  Serial.println("<div class=\"timeline\">");
  for (const auto& event : timeline) {
    Serial.printf("<div class=\"event\">[Phase %u] %s</div>\n", event.phase, event.description);
  }
  Serial.println("</div>");
}

void ExploitationReportGenerator::generateHTMLFooter() {
  Serial.println("</body>");
  Serial.println("</html>");
}

void TimelineVisualizer::recordEvent(uint32_t timestamp, const char* description, uint8_t phase) {
  events.push_back({timestamp, description, phase});
}

void TimelineVisualizer::displayTimeline() {
  Serial.printf("\n=== ATTACK TIMELINE ===\n");
  for (const auto& event : events) {
    Serial.printf("[%05u ms] Phase %u: %s\n", event.timestamp, event.phase, event.description);
  }
  Serial.println("======================\n");
}

void TimelineVisualizer::exportTimeline() {
  Serial.println("\n[TIMELINE_EXPORT_START]");
  for (const auto& event : events) {
    Serial.printf("%u,%u,%s\n", event.timestamp, event.phase, event.description);
  }
  Serial.println("[TIMELINE_EXPORT_END]\n");
}

void CoverageAnalyzer::recordAttackVector(const char* vector, bool successful) {
  for (auto& v : vectors) {
    if (strcmp(v.vector, vector) == 0) {
      v.attempts++;
      if (successful) {
        v.successRate = ((v.successRate * (v.attempts - 1)) + 100) / v.attempts;
      } else {
        v.successRate = (v.successRate * (v.attempts - 1)) / v.attempts;
      }
      return;
    }
  }

  VectorEffectiveness newVector = {vector, (uint8_t)(successful ? 100 : 0), 1};
  vectors.push_back(newVector);
}

void CoverageAnalyzer::analyzeEffectiveness() {
  Serial.printf("\n=== ATTACK VECTOR ANALYSIS ===\n");
  for (const auto& v : vectors) {
    Serial.printf("%s: %.0f%% success rate (%u attempts)\n", v.vector, (float)v.successRate, v.attempts);
  }
  Serial.println("===============================\n");
}

void CoverageAnalyzer::printCoverageReport() {
  analyzeEffectiveness();
}

void CoverageAnalyzer::exportCoverageMatrix() {
  Serial.println("\n[COVERAGE_MATRIX_START]");
  for (const auto& v : vectors) {
    Serial.printf("%s,%u,%u\n", v.vector, v.successRate, v.attempts);
  }
  Serial.println("[COVERAGE_MATRIX_END]\n");
}
