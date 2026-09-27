#include "async_logger.h"
#include <stdio.h>
#include <stdarg.h>

void AsyncLogger::log(LogLevel level, const char* format, ...) {
  if (level < minLevel) return;

  char entry[ASYNC_LOG_ENTRY_MAX];
  memset(entry, 0, ASYNC_LOG_ENTRY_MAX);

  va_list args;
  va_start(args, format);
  int len = vsnprintf(entry, ASYNC_LOG_ENTRY_MAX - 1, format, args);
  va_end(args);

  if (len < 0 || len >= (int)ASYNC_LOG_ENTRY_MAX - 1) {
    droppedCount++;
    return;
  }

  entry[len] = '\n';
  entry[len + 1] = '\0';

  appendToBuffer(entry);
  logCount++;
}

void AsyncLogger::appendToBuffer(const char* entry) {
  uint32_t entryLen = strlen(entry);

  if (bufferPos + entryLen >= ASYNC_LOG_BUFFER_SIZE - 1) {
    flush();
  }

  if (bufferPos + entryLen < ASYNC_LOG_BUFFER_SIZE) {
    strncpy(buffer + bufferPos, entry, ASYNC_LOG_BUFFER_SIZE - bufferPos - 1);
    bufferPos += entryLen;
    buffer[bufferPos] = '\0';
  } else {
    droppedCount++;
  }
}

void AsyncLogger::flush() {
  if (bufferPos > 0) {
    Serial.write((uint8_t*)buffer, bufferPos);
    Serial.flush();
    memset(buffer, 0, ASYNC_LOG_BUFFER_SIZE);
    bufferPos = 0;
  }
}

void AsyncLogger::printStats() {
  Serial.printf("\n=== LOGGER STATS ===\n");
  Serial.printf("Logs: %lu, Dropped: %lu\n", logCount, droppedCount);
  Serial.printf("Buffer usage: %lu / %u bytes\n", bufferPos, ASYNC_LOG_BUFFER_SIZE);
  Serial.println("====================\n");
}
