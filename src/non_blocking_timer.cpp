#include "non_blocking_timer.h"

NonBlockingTimer* TimerManager::createTimer(uint32_t intervalMs, std::function<void()> callback) {
  if (timers.size() >= MAX_TIMERS) {
    return nullptr;
  }

  NonBlockingTimer* timer = new NonBlockingTimer(intervalMs, callback);
  timers.push_back(timer);
  return timer;
}

void TimerManager::updateAll() {
  for (auto* timer : timers) {
    if (timer) {
      timer->update();
    }
  }
}

void TimerManager::removeTimer(NonBlockingTimer* timer) {
  for (size_t i = 0; i < timers.size(); i++) {
    if (timers[i] == timer) {
      delete timers[i];
      timers.erase(timers.begin() + i);
      return;
    }
  }
}
