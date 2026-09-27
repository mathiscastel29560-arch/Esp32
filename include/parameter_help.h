#pragma once

#include <string>

// ============= PARAMETER HELP SYSTEM =============
struct ParameterHelp {
  const char* name;
  const char* shortDesc;      // 1 line description
  const char* fullDesc;       // Full explanation
  const char* range;          // Valid range/options
  const char* tips;           // Usage tips
};

class ParameterHelpSystem {
public:
  static const ParameterHelpSystem& getInstance() {
    static ParameterHelpSystem instance;
    return instance;
  }

  // Get help for a parameter by index (from parametersMenuItems)
  const ParameterHelp* getParameterHelp(int index) const;

  // Get help by name
  const ParameterHelp* getParameterHelpByName(const char* name) const;

  // Print full help for a parameter
  void printParameterHelp(int index) const;

  // Display quick help (short desc only)
  std::string getQuickHelp(int index) const;

  // Total help entries
  int getTotalHelpEntries() const { return HELP_COUNT; }

private:
  ParameterHelpSystem() = default;

  static constexpr int HELP_COUNT = 14;

  // Help database
  static const ParameterHelp HELP_DATABASE[];
};

#endif // PARAMETER_HELP_H
