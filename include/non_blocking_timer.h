#ifndef NON_BLOCKING_TIMER_H
#define NON_BLOCKING_TIMER_H

#include <Arduino.h>
#include <vector>
#include <functional>

class NonBlockingTimer {
public:
  NonBlockingTimer(uint32_t intervalMs, std::function<void()> callback)
    : interval(intervalMs), callback(callback), lastTrigger(0), isActive(true) {}

  bool update() {
    if (!isActive) return false;

    uint32_t now = millis();
    if (now - lastTrigger >= interval) {
      callback();
      lastTrigger = now;
      return true;
    }
    return false;
  }

  void start() { isActive = true; lastTrigger = millis(); }
  void stop() { isActive = false; }
  void reset() { lastTrigger = millis(); }
  void setInterval(uint32_t ms) { interval = ms; }

private:
  uint32_t interval;
  std::function<void()> callback;
  uint32_t lastTrigger;
  bool isActive;
};

class TimerManager {
public:
  static TimerManager& getInstance() {
    static TimerManager instance;
    return instance;
  }

  NonBlockingTimer* createTimer(uint32_t intervalMs, std::function<void()> callback);
  void updateAll();
  void removeTimer(NonBlockingTimer* timer);

  uint16_t getActiveTimerCount() const { return timers.size(); }

private:
  TimerManager() {}

  std::vector<NonBlockingTimer*> timers;

  static const uint16_t MAX_TIMERS = 32;
};

#endif
