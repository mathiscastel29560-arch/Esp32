#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H

#include <Arduino.h>
#include <vector>
#include <cstring>

#define ASYNC_LOG_BUFFER_SIZE 16384
#define ASYNC_LOG_ENTRY_MAX 512

enum class LogLevel : uint8_t {
  VERBOSE = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4
};

struct LogEntry {
  uint32_t timestamp;
  LogLevel level;
  char message[ASYNC_LOG_ENTRY_MAX];

  LogEntry() : timestamp(0), level(LogLevel::INFO) {
    memset(message, 0, ASYNC_LOG_ENTRY_MAX);
  }
};

class AsyncLogger {
public:
  static AsyncLogger& getInstance() {
    static AsyncLogger instance;
    return instance;
  }

  void log(LogLevel level, const char* format, ...);
  void flush();
  void setMinLevel(LogLevel level) { minLevel = level; }

  uint32_t getLogCount() const { return logCount; }
  uint32_t getDroppedCount() const { return droppedCount; }

  void printStats();

private:
  AsyncLogger() : logCount(0), droppedCount(0), minLevel(LogLevel::VERBOSE),
                  bufferPos(0) {
    buffer = (char*)malloc(ASYNC_LOG_BUFFER_SIZE);
    memset(buffer, 0, ASYNC_LOG_BUFFER_SIZE);
  }

  ~AsyncLogger() {
    if (buffer) free(buffer);
  }

  char* buffer;
  uint32_t bufferPos;
  uint32_t logCount;
  uint32_t droppedCount;
  LogLevel minLevel;

  void appendToBuffer(const char* entry);
};

#define LOG_V(fmt, ...) AsyncLogger::getInstance().log(LogLevel::VERBOSE, fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) AsyncLogger::getInstance().log(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) AsyncLogger::getInstance().log(LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) AsyncLogger::getInstance().log(LogLevel::WARN, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) AsyncLogger::getInstance().log(LogLevel::ERROR, fmt, ##__VA_ARGS__)

#endif
