#include "lifecycle_manager.h"

const char* Component::getStateName() const {
  switch (state) {
    case ComponentState::UNINITIALIZED: return "UNINITIALIZED";
    case ComponentState::INITIALIZING: return "INITIALIZING";
    case ComponentState::INITIALIZED: return "INITIALIZED";
    case ComponentState::RUNNING: return "RUNNING";
    case ComponentState::PAUSED: return "PAUSED";
    case ComponentState::STOPPING: return "STOPPING";
    case ComponentState::STOPPED: return "STOPPED";
    case ComponentState::ERROR: return "ERROR";
  }
  return "UNKNOWN";
}

void LifecycleManager::registerComponent(Component* component, const char* name) {
  if (component && name) {
    components.push_back(new ComponentEntry(component, name));
  }
}

bool LifecycleManager::initializeAll() {
  for (auto* entry : components) {
    if (!entry->component->initialize()) {
      entry->component->setState(ComponentState::ERROR);
      return false;
    }
    entry->component->setState(ComponentState::INITIALIZED);
  }
  return true;
}

bool LifecycleManager::startAll() {
  for (auto* entry : components) {
    if (entry->component->getState() != ComponentState::INITIALIZED) {
      continue;
    }
    if (!entry->component->start()) {
      entry->component->setState(ComponentState::ERROR);
      return false;
    }
    entry->component->setState(ComponentState::RUNNING);
  }
  return true;
}

bool LifecycleManager::stopAll() {
  for (auto* entry : components) {
    if (entry->component->getState() == ComponentState::RUNNING) {
      entry->component->setState(ComponentState::STOPPING);
      if (!entry->component->stop()) {
        entry->component->setState(ComponentState::ERROR);
      } else {
        entry->component->setState(ComponentState::STOPPED);
      }
    }
  }
  return true;
}

void LifecycleManager::updateAll() {
  for (auto* entry : components) {
    if (entry->component->getState() == ComponentState::RUNNING) {
      entry->component->update();
    }
  }
}

Component* LifecycleManager::getComponent(const char* name) {
  for (auto* entry : components) {
    if (strcmp(entry->name, name) == 0) {
      return entry->component;
    }
  }
  return nullptr;
}

void LifecycleManager::printComponentStatus() {
  Serial.println("\n=== COMPONENT LIFECYCLE ===");
  for (auto* entry : components) {
    Serial.printf("%s: %s\n", entry->name, entry->component->getStateName());
  }
  Serial.println("===========================\n");
}
