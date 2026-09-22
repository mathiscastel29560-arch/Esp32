#ifndef BLE_AUDIO_HIJACKING_H
#define BLE_AUDIO_HIJACKING_H

#include <Arduino.h>
#include <vector>

namespace BleAudioHijacking {

enum AudioDevice {
  HEADPHONES,
  SPEAKER,
  SOUNDBAR,
  UNKNOWN
};

struct AudioConfig {
  uint8_t targetAddr[6];
  AudioDevice deviceType;
  uint32_t durationMs;
  bool volumeControl;
  bool mediaControl;
  bool audioInjection;
};

struct AudioResult {
  bool success;
  AudioDevice detectedType;
  uint32_t commandsSent;
  uint32_t volumeLevel;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class AudioHijacker {
public:
  AudioHijacker();
  AudioResult detectAudioDevice(const uint8_t* addr);
  AudioResult hijackDevice(const AudioConfig& config);
  AudioResult controlVolume(const uint8_t* addr, uint8_t level);
  AudioResult injectAudio(const uint8_t* addr, const uint8_t* audioData, uint32_t len);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void logHijack(AudioDevice type, uint32_t commands);
};

} // namespace BleAudioHijacking

#endif
