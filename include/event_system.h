#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <cstring>

struct Event {
  uint32_t id;
  uint32_t timestamp;
  char type[64];
  uint8_t priority;
  void* data;
  uint16_t dataSize;

  Event(const char* t, uint8_t p = 0)
    : id(0), timestamp(millis()), priority(p), data(nullptr), dataSize(0) {
    strncpy(type, t, 63);
    type[63] = '\0';
  }

  ~Event() {
    if (data) free(data);
  }
};

class EventListener {
public:
  virtual ~EventListener() {}
  virtual void onEvent(const Event& event) = 0;
};

class EventBus {
public:
  static EventBus& getInstance() {
    static EventBus instance;
    return instance;
  }

  void subscribe(const char* eventType, EventListener* listener) {
    if (listener) {
      listeners.push_back({eventType, listener});
    }
  }

  void unsubscribe(const char* eventType, EventListener* listener) {
    for (size_t i = 0; i < listeners.size(); i++) {
      if (listeners[i].eventType == eventType &&
          listeners[i].listener == listener) {
        listeners.erase(listeners.begin() + i);
        return;
      }
    }
  }

  void publish(Event& event) {
    event.timestamp = millis();
    event.id = nextEventId++;

    for (auto& sub : listeners) {
      if (strcmp(sub.eventType, event.type) == 0) {
        sub.listener->onEvent(event);
      }
    }
  }

  void publishBroadcast(Event& event) {
    event.timestamp = millis();
    event.id = nextEventId++;

    for (auto& sub : listeners) {
      sub.listener->onEvent(event);
    }
  }

  uint32_t getListenerCount() const { return listeners.size(); }
  uint32_t getEventCount() const { return nextEventId; }

  void printEventStats() {
    Serial.printf("Event bus: %lu listeners, %lu events published\n",
      getListenerCount(), getEventCount());
  }

private:
  EventBus() : nextEventId(1) {}

  struct Subscription {
    const char* eventType;
    EventListener* listener;
  };

  std::vector<Subscription> listeners;
  uint32_t nextEventId;
};

#endif
