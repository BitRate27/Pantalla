#pragma once
#include <vector>
#include <string>
#include "DisplayManager.h"
#include "json.hpp"

// Save settings to SETTINGS_FILE. Returns true on success.
bool SaveSettings(const std::vector<DisplayInfo *> &displays, const std::vector<DisplayInfo *> &virtualDisplays, bool startOnUserLogin);

// Load settings from SETTINGS_FILE into the provided vectors. Returns true on success.
bool LoadSettings(std::vector<DisplayInfo *> &displays, std::vector<DisplayInfo *> &virtualDisplays, bool &startOnUserLogin);

// Helper to parse JSON into display lists
void getDisplaysFromSettings(::nlohmann::json &j, const std::string &displayType, std::vector<DisplayInfo *> &displays);

// Add/remove Run on startup entry for current user
bool SetRunOnStartup(bool enable);
