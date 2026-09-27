#include "resource_cache.h"

void SystemResourceCache::updateAllCaches() {
  uint32_t freeHeap = ESP.getFreeHeap();
  freeHeapCache.set("freeHeap", freeHeap, 1000);

  uint8_t battery = (uint8_t)(analogRead(GPIO_NUM_7) / 40.95f);
  batteryCache.set("battery", battery, 2000);
}

void SystemResourceCache::printCacheStats() {
  Serial.println("\n=== CACHE STATISTICS ===");

  Serial.printf("FreeHeap Cache Hit Rate: %lu%%\n", freeHeapCache.getHitRate());
  Serial.printf("FreeHeap Cache Size: %u entries\n", freeHeapCache.getSize());

  Serial.printf("Battery Cache Hit Rate: %lu%%\n", batteryCache.getHitRate());
  Serial.printf("Battery Cache Size: %u entries\n", batteryCache.getSize());

  Serial.printf("RSSI Cache Hit Rate: %lu%%\n", rssiCache.getHitRate());
  Serial.printf("RSSI Cache Size: %u entries\n", rssiCache.getSize());

  Serial.println("========================\n");
}
