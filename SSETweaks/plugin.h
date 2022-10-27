#pragma once

#define PLUGIN_NAME   "SSEDisplayTweaks"
#define PLUGIN_AUTHOR "SlavicPotato"

#define PLUGIN_BASE_PATH  "Data\\SKSE\\Plugins\\"
#define PLUGIN_BASE_PATHW L"Data\\SKSE\\Plugins\\"

static inline constexpr const char* PLUGIN_LOG_PATH                                              = "My Games\\Skyrim Special Edition\\SKSE\\" PLUGIN_NAME ".log";
static inline constexpr const char* PLUGIN_LOG_PATH_GOG                                          = "My Games\\Skyrim Special Edition GOG\\SKSE\\" PLUGIN_NAME ".log";
static inline constexpr const char* PLUGIN_INI_FILE                                              = PLUGIN_BASE_PATH PLUGIN_NAME ".ini";
static inline constexpr const char*                                    SKYRIM_PREFS_INI_FILE     = "My Games\\Skyrim Special Edition\\SkyrimPrefs.ini";
static inline constexpr const char*                                    SKYRIM_PREFS_INI_FILE_GOG = "My Games\\Skyrim Special Edition GOG\\SkyrimPrefs.ini";
static inline constexpr const char* PLUGIN_INI_CUSTOM_FILE                                       = PLUGIN_BASE_PATH PLUGIN_NAME "_custom.ini";
static inline constexpr const wchar_t*                                        OSD_FONT_PATH      = PLUGIN_BASE_PATHW L"SDTFonts\\";

#include "version.h"
