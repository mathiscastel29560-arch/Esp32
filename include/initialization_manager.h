#ifndef INITIALIZATION_MANAGER_H
#define INITIALIZATION_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <functional>

struct DriverInitEntry {
  const char* name;
  std::function<bool()> initFunc;
  bool critical;
  uint32_t initTime;
  bool initialized;

  DriverInitEntry(const char* n, std::function<bool()> f, bool crit = false)
    : name(n), initFunc(f), critical(crit), initTime(0), initialized(false) {}
};

class InitializationManager {
public:
  static InitializationManager& getInstance() {
    static InitializationManager instance;
    return instance;
  }

  void registerDriver(const char* name, std::function<bool()> initFunc, bool critical = false);
  bool initializeAll();
  bool initializeDriver(const char* name);

  uint32_t getTotalInitTime() const { return totalInitTime; }
  uint16_t getInitializedCount() const { return initializedCount; }
  uint16_t getFailedCount() const { return failedCount; }

  bool isDriverInitialized(const char* name);
  void printInitReport();

private:
  InitializationManager() : totalInitTime(0), initializedCount(0), failedCount(0) {}

  std::vector<DriverInitEntry*> drivers;
  uint32_t totalInitTime;
  uint16_t initializedCount;
  uint16_t failedCount;
};

#endif
