#ifndef RESOURCE_CACHE_H
#define RESOURCE_CACHE_H

#include <Arduino.h>
#include <map>
#include <cstring>

template<typename T>
class ResourceCache {
private:
  struct CacheEntry {
    T value;
    uint32_t timestamp;
    uint32_t ttl;

    CacheEntry(const T& v, uint32_t ttlMs = 5000)
      : value(v), timestamp(millis()), ttl(ttlMs) {}

    bool isExpired() const {
      return (millis() - timestamp) > ttl;
    }
  };

  std::map<const char*, CacheEntry*> cache;
  uint32_t hitCount;
  uint32_t missCount;

public:
  ResourceCache() : hitCount(0), missCount(0) {}

  ~ResourceCache() {
    for (auto& pair : cache) {
      delete pair.second;
    }
    cache.clear();
  }

  bool get(const char* key, T& output) {
    auto it = cache.find(key);
    if (it != cache.end()) {
      if (it->second->isExpired()) {
        delete it->second;
        cache.erase(it);
        missCount++;
        return false;
      }
      output = it->second->value;
      hitCount++;
      return true;
    }
    missCount++;
    return false;
  }

  void set(const char* key, const T& value, uint32_t ttlMs = 5000) {
    auto it = cache.find(key);
    if (it != cache.end()) {
      delete it->second;
    }
    cache[key] = new CacheEntry(value, ttlMs);
  }

  void clear() {
    for (auto& pair : cache) {
      delete pair.second;
    }
    cache.clear();
    hitCount = 0;
    missCount = 0;
  }

  uint32_t getHitRate() const {
    uint32_t total = hitCount + missCount;
    return total == 0 ? 0 : (hitCount * 100) / total;
  }

  uint16_t getSize() const { return cache.size(); }
};

class SystemResourceCache {
public:
  static SystemResourceCache& getInstance() {
    static SystemResourceCache instance;
    return instance;
  }

  ResourceCache<uint32_t>& getFreeHeapCache() { return freeHeapCache; }
  ResourceCache<uint8_t>& getBatteryCache() { return batteryCache; }
  ResourceCache<int8_t>& getRSSICache() { return rssiCache; }

  void updateAllCaches();
  void printCacheStats();

private:
  SystemResourceCache() {}

  ResourceCache<uint32_t> freeHeapCache;
  ResourceCache<uint8_t> batteryCache;
  ResourceCache<int8_t> rssiCache;
};

#endif
