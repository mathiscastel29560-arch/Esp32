#ifndef DESIGN_PATTERNS_H
#define DESIGN_PATTERNS_H

#include <Arduino.h>

// Singleton pattern (thread-safe on single-core)
template<typename T>
class Singleton {
public:
  static T& getInstance() {
    static T instance;
    return instance;
  }

protected:
  Singleton() {}
  ~Singleton() {}

private:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
};

// Observer pattern for event handling
template<typename EventT>
class Observer {
public:
  virtual ~Observer() {}
  virtual void onEvent(const EventT& event) = 0;
};

template<typename EventT>
class ObserverList {
public:
  void subscribe(Observer<EventT>* observer) {
    if (observer) observers.push_back(observer);
  }

  void unsubscribe(Observer<EventT>* observer) {
    for (size_t i = 0; i < observers.size(); i++) {
      if (observers[i] == observer) {
        observers.erase(observers.begin() + i);
        return;
      }
    }
  }

  void notify(const EventT& event) {
    for (auto* observer : observers) {
      if (observer) observer->onEvent(event);
    }
  }

private:
  std::vector<Observer<EventT>*> observers;
};

// Factory pattern for object creation
template<typename BaseT, typename DerivedT>
class Factory {
public:
  static BaseT* create() {
    return new DerivedT();
  }
};

// RAII pattern for resource management
class Resource {
public:
  virtual ~Resource() {}
  virtual bool acquire() = 0;
  virtual void release() = 0;
};

template<typename ResourceT>
class AutoResource {
public:
  AutoResource(ResourceT* res) : resource(res) {
    if (resource) resource->acquire();
  }

  ~AutoResource() {
    if (resource) resource->release();
  }

  ResourceT* operator->() { return resource; }

private:
  ResourceT* resource;
  AutoResource(const AutoResource&) = delete;
  AutoResource& operator=(const AutoResource&) = delete;
};

// Strategy pattern for algorithm selection
template<typename InputT, typename OutputT>
class Strategy {
public:
  virtual ~Strategy() {}
  virtual bool execute(const InputT& input, OutputT& output) = 0;
};

template<typename InputT, typename OutputT>
class StrategyContext {
public:
  void setStrategy(Strategy<InputT, OutputT>* strat) {
    if (strategy) delete strategy;
    strategy = strat;
  }

  bool execute(const InputT& input, OutputT& output) {
    if (strategy) return strategy->execute(input, output);
    return false;
  }

private:
  Strategy<InputT, OutputT>* strategy = nullptr;
};

#endif
