#pragma once

#include <string>
#include <vector>
#include <cstdint>

// RBAC + API Key authentication system
class AuthSystem {
public:
  enum UserRole {
    ROLE_ADMIN = 0,    // Full access
    ROLE_OPERATOR = 1, // Can run audits, view results
    ROLE_VIEWER = 2    // Read-only access
  };

  struct APIKey {
    std::string key;
    std::string description;
    UserRole role;
    uint32_t createdAt;
    uint32_t lastUsedAt;
    bool enabled;
  };

  static AuthSystem& getInstance() {
    static AuthSystem instance;
    return instance;
  }

  // Initialize authentication system
  bool begin();

  // Generate new API key
  std::string generateAPIKey(const std::string& description, UserRole role);

  // Validate API key and get role
  bool validateAPIKey(const std::string& key, UserRole& role);

  // Check if user has permission for action
  bool hasPermission(UserRole role, const std::string& action);

  // List all API keys (admin only)
  std::vector<APIKey> listAPIKeys();

  // Revoke API key
  bool revokeAPIKey(const std::string& key);

  // Set default authentication token (for initial setup)
  void setAuthToken(const std::string& token);

  // Validate request token
  bool validateToken(const std::string& token);

  // Permissions
  static bool canViewAudits(UserRole role);
  static bool canRunAudits(UserRole role);
  static bool canModifySettings(UserRole role);
  static bool canManageUsers(UserRole role);

private:
  AuthSystem() = default;

  std::string authToken;
  std::vector<APIKey> apiKeys;
  bool initialized = false;

  // Generate random alphanumeric string
  std::string generateRandomString(size_t length);

  // Load keys from NVS
  bool loadFromNVS();

  // Save keys to NVS
  bool saveToNVS();
};

#endif // AUTH_SYSTEM_H
