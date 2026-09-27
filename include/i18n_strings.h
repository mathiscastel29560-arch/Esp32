#pragma once

#include <string>
#include <map>

// Multi-language string support
class I18nStrings {
public:
  static I18nStrings& getInstance() {
    static I18nStrings instance;
    return instance;
  }

  // Get string by key and language
  std::string get(const std::string& key, const std::string& lang = "en");

  // Set active language
  void setLanguage(const std::string& lang);

  // Get current language
  std::string getLanguage() const { return currentLanguage; }

private:
  I18nStrings() : currentLanguage("en") {}

  std::string currentLanguage;

  // English translations
  static const char* EN_STRINGS[][2];

  // French translations
  static const char* FR_STRINGS[][2];

  // Spanish translations
  static const char* ES_STRINGS[][2];
};

#endif // I18N_STRINGS_H
