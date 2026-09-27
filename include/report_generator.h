#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <Arduino.h>
#include <vector>

// ============= PROFESSIONAL REPORT GENERATOR =============

struct ReportSection {
  const char* title;
  const char* content;
  uint8_t priority;  // 1-10, higher = more important
};

class ExploitationReportGenerator {
public:
  static ExploitationReportGenerator& getInstance() {
    static ExploitationReportGenerator instance;
    return instance;
  }

  // Report building
  void startReport(const char* targetName, uint32_t timestamp);
  void addExecutiveSummary(uint16_t vulnFound, uint16_t vulnExploited,
                          uint8_t maxAccess, float successRate);
  void addVulnerabilitySection(const char* vulnName, uint8_t severity,
                              const char* description, bool exploited);
  void addTimelineEntry(uint32_t timestamp, const char* event, uint8_t phase);
  void addRiskAssessment(uint8_t overallRisk, const char* recommendations);
  void finishReport();

  // Report output
  void printHTMLReport();
  void printTextReport();
  void printJSONReport();
  void exportReport(const char* filename);

  void clearReport();

private:
  ExploitationReportGenerator() : reportStartTime(0), targetName("") {}

  struct TimelineEvent {
    uint32_t timestamp;
    const char* event;
    uint8_t phase;
  };

  const char* targetName;
  uint32_t reportStartTime;
  uint16_t vulnFound;
  uint16_t vulnExploited;
  uint8_t maxAccessLevel;
  float successRate;
  uint8_t overallRisk;

  std::vector<ReportSection> sections;
  std::vector<TimelineEvent> timeline;
  std::vector<const char*> vulnerabilities;

  void generateHTMLHeader();
  void generateHTMLBody();
  void generateHTMLFooter();
  void generateTimeline();
};

// ============= TIMELINE VISUALIZER =============

class TimelineVisualizer {
public:
  static TimelineVisualizer& getInstance() {
    static TimelineVisualizer instance;
    return instance;
  }

  void recordEvent(uint32_t timestamp, const char* description, uint8_t phase);
  void displayTimeline();
  void exportTimeline();

private:
  TimelineVisualizer() {}

  struct Event {
    uint32_t timestamp;
    const char* description;
    uint8_t phase;
  };

  std::vector<Event> events;
};

// ============= COVERAGE ANALYZER =============

class CoverageAnalyzer {
public:
  static CoverageAnalyzer& getInstance() {
    static CoverageAnalyzer instance;
    return instance;
  }

  void recordAttackVector(const char* vector, bool successful);
  void analyzeEffectiveness();
  void printCoverageReport();
  void exportCoverageMatrix();

private:
  CoverageAnalyzer() {}

  struct VectorEffectiveness {
    const char* vector;
    uint8_t successRate;
    uint16_t attempts;
  };

  std::vector<VectorEffectiveness> vectors;
};

#endif
