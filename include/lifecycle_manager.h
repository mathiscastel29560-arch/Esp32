#ifndef LIFECYCLE_MANAGER_H
#define LIFECYCLE_MANAGER_H

#include <Arduino.h>
#include <vector>

enum class ComponentState {
  UNINITIALIZED = 0,
  INITIALIZING = 1,
  INITIALIZED = 2,
  RUNNING = 3,
  PAUSED = 4,
  STOPPING = 5,
  STOPPED = 6,
  ERROR = 7
};

class Component {
public:
  virtual ~Component() {}

  virtual bool initialize() = 0;
  virtual bool start() = 0;
  virtual bool stop() = 0;
  virtual void update() = 0;
  virtual bool cleanup() = 0;

  ComponentState getState() const { return state; }
  const char* getStateName() const;

protected:
  ComponentState state = ComponentState::UNINITIALIZED;

  void setState(ComponentState newState) {
    ComponentState oldState = state;
    state = newState;
    onStateChange(oldState, newState);
  }

  virtual void onStateChange(ComponentState oldState, ComponentState newState) {}
};

class LifecycleManager {
public:
  static LifecycleManager& getInstance() {
    static LifecycleManager instance;
    return instance;
  }

  void registerComponent(Component* component, const char* name);
  bool initializeAll();
  bool startAll();
  bool stopAll();
  void updateAll();

  Component* getComponent(const char* name);
  uint16_t getComponentCount() const { return components.size(); }

  void printComponentStatus();

private:
  LifecycleManager() {}

  struct ComponentEntry {
    Component* component;
    const char* name;
    ComponentState lastState;

    ComponentEntry(Component* c, const char* n)
      : component(c), name(n), lastState(ComponentState::UNINITIALIZED) {}
  };

  std::vector<ComponentEntry*> components;
};

#endif
