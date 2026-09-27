#ifndef RESULTS_EXPORTER_H
#define RESULTS_EXPORTER_H

#include "attack_framework.h"
#include <vector>

// ============= RESULTS EXPORTER =============
class ResultsExporter {
public:
  static ResultsExporter& getInstance() {
    static ResultsExporter instance;
    return instance;
  }

  // Export formats
  void exportAttackToHTML(Attack* attack, const char* filename);
  void exportAttackToJSON(Attack* attack, const char* filename);
  void exportAttackToCSV(Attack* attack, const char* filename);

  // Batch export
  void exportMultipleToHTML(std::vector<Attack*>& attacks, const char* filename);
  void generateReport(std::vector<Attack*>& attacks, const char* filename);

private:
  ResultsExporter();

  void writeHTMLHeader(FILE* file, const char* title);
  void writeHTMLFooter(FILE* file);
  void writeAttackStats(FILE* file, Attack* attack);
  void writeResultsTable(FILE* file, Attack* attack);
};

#endif // RESULTS_EXPORTER_H
