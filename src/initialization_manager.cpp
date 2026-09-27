#include "initialization_manager.h"
#include "debug_logger.h"

void InitializationManager::registerDriver(const char* name, std::function<bool()> initFunc, bool critical) {
  drivers.push_back(new DriverInitEntry(name, initFunc, critical));
}

bool InitializationManager::initializeAll() {
  totalInitTime = millis();

  for (auto* entry : drivers) {
    uint32_t startTime = millis();
    bool result = entry->initFunc();
    entry->initTime = millis() - startTime;
    entry->initialized = result;

    if (result) {
      initializedCount++;
      DEBUG_LOG_INFO(entry->name, "initialized in %lums", entry->initTime);
    } else {
      failedCount++;
      if (entry->critical) {
        DEBUG_LOG_ERROR(entry->name, "CRITICAL initialization failed!");
        return false;
      } else {
        DEBUG_LOG_WARN(entry->name, "initialization failed (continuing anyway)");
      }
    }
  }

  totalInitTime = millis() - totalInitTime;
  return true;
}

bool InitializationManager::initializeDriver(const char* name) {
  for (auto* entry : drivers) {
    if (strcmp(entry->name, name) == 0) {
      if (!entry->initFunc()) {
        entry->initialized = false;
        failedCount++;
        if (entry->critical) return false;
      } else {
        entry->initialized = true;
        initializedCount++;
      }
      return entry->initialized;
    }
  }
  return false;
}

bool InitializationManager::isDriverInitialized(const char* name) {
  for (auto* entry : drivers) {
    if (strcmp(entry->name, name) == 0) {
      return entry->initialized;
    }
  }
  return false;
}

void InitializationManager::printInitReport() {
  Serial.println("\n=== INITIALIZATION REPORT ===");
  Serial.printf("Total time: %lu ms\n", totalInitTime);
  Serial.printf("Initialized: %u / Failed: %u\n", initializedCount, failedCount);
  Serial.println("---");

  for (auto* entry : drivers) {
    char status = entry->initialized ? '+' : '-';
    Serial.printf("[%c] %s: %lu ms\n", status, entry->name, entry->initTime);
  }
  Serial.println("=============================\n");
}
