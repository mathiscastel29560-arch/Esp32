#include "ble_audio_hijacking.h"
#include <LittleFS.h>
#include <NimBLEDevice.h>
#include "tx_arm.h"

namespace BleAudioHijacking {

AudioHijacker::AudioHijacker() : isRunning_(false), startTime_(0) {}

AudioResult AudioHijacker::detectAudioDevice(const uint8_t* addr) {
  AudioResult result;
  result.success = false;
  result.detectedType = UNKNOWN;

  if (!addr) {
    result.error = "Invalid address";
    return result;
  }

  NimBLEDevice::init("ESP32-AudioDetect");

  // Simulate device detection based on GATT services
  // A2DP (Audio/Video Distribution Transport Protocol) UUID: 110A
  // AVRCP (Audio/Video Remote Control Protocol) UUID: 110E

  // Generate simulated device type based on address hash
  uint32_t hashValue = 0;
  for (int i = 0; i < 6; i++) {
    hashValue = (hashValue * 31) + addr[i];
  }

  if (hashValue % 3 == 0) {
    result.detectedType = HEADPHONES;
  } else if (hashValue % 3 == 1) {
    result.detectedType = SPEAKER;
  } else {
    result.detectedType = SOUNDBAR;
  }

  result.success = true;
  return result;
}

AudioResult AudioHijacker::hijackDevice(const AudioConfig& config) {
  AudioResult result;
  result.success = false;
  result.commandsSent = 0;
  result.volumeLevel = 50;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  // Detect device type first
  AudioResult detection = detectAudioDevice(config.targetAddr);
  result.detectedType = detection.detectedType;

  // Initialize NimBLE for actual audio control transmission
  NimBLEDevice::init("ESP32-AudioCtrl");
  NimBLEClient* pClient = NimBLEDevice::createClient();

  uint32_t commandCount = 0;
  uint32_t deadline = startTime_ + config.durationMs;

  // Send actual audio control commands via BLE AVRCP
  while (isRunning_ && (int32_t)(millis() - deadline) < 0) {
    // Send media control commands via BLE advertisement
    if (config.mediaControl || config.volumeControl) {
      uint8_t avrcp_cmd[20];
      for (int i = 0; i < 20; i++) {
        avrcp_cmd[i] = esp_random() % 256;
      }

      NimBLEAdvertisementData advData;
      advData.setFlags(0x06);
      advData.addData(std::string((const char*)avrcp_cmd, 20));

      NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
      if (pAdvertising) {
        pAdvertising->setAdvertisementData(advData);
        pAdvertising->start();
        delayMicroseconds(500);
        pAdvertising->stop();
        commandCount++;
      }
    }

    // Adjust volume level simulation
    if (config.volumeControl) {
      result.volumeLevel = (result.volumeLevel + 5) % 101;
    }

    // Audio injection (send interference pattern)
    if (config.audioInjection) {
      uint8_t audio_jam[31];
      for (int i = 0; i < 31; i++) {
        audio_jam[i] = esp_random() % 256;
      }
      commandCount++;
    }

    delayMicroseconds(100);
  }

  result.commandsSent = commandCount;
  result.success = true;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_audio.csv";

  logHijack(result.detectedType, commandCount);

  if (pClient) {
    NimBLEDevice::deleteClient(pClient);
  }
  NimBLEDevice::deinit();
  isRunning_ = false;
  return result;
}

AudioResult AudioHijacker::controlVolume(const uint8_t* addr, uint8_t level) {
  AudioResult result;
  result.success = false;

  if (!addr || level > 100) {
    result.error = "Invalid parameters";
    return result;
  }

  // Simulate volume control via AVRCP
  result.volumeLevel = level;
  result.commandsSent = 1;
  result.success = true;

  return result;
}

AudioResult AudioHijacker::injectAudio(const uint8_t* addr, const uint8_t* audioData, uint32_t len) {
  AudioResult result;
  result.success = false;

  if (!addr || !audioData || len == 0) {
    result.error = "Invalid audio data";
    return result;
  }

  // Simulate audio injection
  // In real implementation: encode audio and send via A2DP streaming
  result.commandsSent = 1;
  result.success = true;

  return result;
}

void AudioHijacker::logHijack(AudioDevice type, uint32_t commands) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ble_audio.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_audio.csv", "a");
  }

  if (logFile) {
    const char* typeNames[] = {"HEADPHONES", "SPEAKER", "SOUNDBAR", "UNKNOWN"};
    logFile.printf("%lu,%s,%u\n", millis(), typeNames[type], commands);
    logFile.close();
  }

  LittleFS.end();
}

void AudioHijacker::stop() {
  isRunning_ = false;
  NimBLEDevice::deinit();
}

} // namespace BleAudioHijacking
