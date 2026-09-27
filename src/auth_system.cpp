#include "auth_system.h"
#include "debug_logger.h"
#include <nvs_flash.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

bool AuthSystem::begin() {
  if (initialized) return true;

  if (!loadFromNVS()) {
    DebugLogger::println("[Auth] Creating default API key...");
    generateAPIKey("Default Admin Key", ROLE_ADMIN);
  }

  initialized = true;
  DebugLogger::println("[Auth] Authentication system initialized");
  return true;
}

std::string AuthSystem::generateAPIKey(const std::string& description, UserRole role) {
  std::string key = generateRandomString(32);

  APIKey newKey;
  newKey.key = key;
  newKey.description = description;
  newKey.role = role;
  newKey.createdAt = time(nullptr);
  newKey.lastUsedAt = 0;
  newKey.enabled = true;

  apiKeys.push_back(newKey);
  saveToNVS();

  DebugLogger::printf("[Auth] Generated API key: %s (role=%u)\n", key.c_str(), role);
  return key;
}

bool AuthSystem::validateAPIKey(const std::string& key, UserRole& role) {
  for (auto& apiKey : apiKeys) {
    if (apiKey.key == key && apiKey.enabled) {
      role = apiKey.role;
      apiKey.lastUsedAt = time(nullptr);
      saveToNVS();
      return true;
    }
  }
  return false;
}

bool AuthSystem::hasPermission(UserRole role, const std::string& action) {
  if (role == ROLE_ADMIN) return true;

  if (action == "view_audits" || action == "view_settings") {
    return role != ROLE_ADMIN; // All can view
  }
  if (action == "run_audits" || action == "modify_params") {
    return role == ROLE_ADMIN || role == ROLE_OPERATOR;
  }
  if (action == "modify_settings" || action == "manage_keys") {
    return role == ROLE_ADMIN;
  }

  return false;
}

std::vector<AuthSystem::APIKey> AuthSystem::listAPIKeys() {
  return apiKeys;
}

bool AuthSystem::revokeAPIKey(const std::string& key) {
  for (auto& apiKey : apiKeys) {
    if (apiKey.key == key) {
      apiKey.enabled = false;
      saveToNVS();
      DebugLogger::printf("[Auth] Revoked API key: %s\n", key.c_str());
      return true;
    }
  }
  return false;
}

void AuthSystem::setAuthToken(const std::string& token) {
  authToken = token;
}

bool AuthSystem::validateToken(const std::string& token) {
  return token == authToken && !authToken.empty();
}

bool AuthSystem::canViewAudits(UserRole role) {
  return role <= ROLE_VIEWER; // All roles can view
}

bool AuthSystem::canRunAudits(UserRole role) {
  return role == ROLE_ADMIN || role == ROLE_OPERATOR;
}

bool AuthSystem::canModifySettings(UserRole role) {
  return role == ROLE_ADMIN || role == ROLE_OPERATOR;
}

bool AuthSystem::canManageUsers(UserRole role) {
  return role == ROLE_ADMIN;
}

std::string AuthSystem::generateRandomString(size_t length) {
  const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  std::string result;

  for (size_t i = 0; i < length; i++) {
    result += charset[rand() % 62];
  }

  return result;
}

bool AuthSystem::loadFromNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("auth", NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    DebugLogger::println("[Auth] NVS not initialized yet");
    return false;
  }

  uint32_t keyCount = 0;
  nvs_get_u32(handle, "key_count", &keyCount);

  for (uint32_t i = 0; i < keyCount; i++) {
    char keyName[32];
    snprintf(keyName, sizeof(keyName), "key_%u", i);

    char keyData[64];
    size_t len = sizeof(keyData);
    if (nvs_get_str(handle, keyName, keyData, &len) == ESP_OK) {
      // Simple format: "key|role|enabled"
      // Can be expanded with more data
    }
  }

  nvs_close(handle);
  return true;
}

bool AuthSystem::saveToNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("auth", NVS_READWRITE, &handle);
  if (err != ESP_OK) return false;

  nvs_set_u32(handle, "key_count", apiKeys.size());

  for (size_t i = 0; i < apiKeys.size(); i++) {
    char keyName[32];
    snprintf(keyName, sizeof(keyName), "key_%u", i);

    char keyData[128];
    snprintf(keyData, sizeof(keyData), "%s|%u|%u",
      apiKeys[i].key.c_str(), apiKeys[i].role, apiKeys[i].enabled ? 1 : 0);

    nvs_set_str(handle, keyName, keyData);
  }

  nvs_commit(handle);
  nvs_close(handle);
  return true;
}
