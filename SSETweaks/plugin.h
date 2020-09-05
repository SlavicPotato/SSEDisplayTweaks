#pragma once

#define PLUGIN_NAME                 "SSEDisplayTweaks"
#define PLUGIN_AUTHOR               "SlavicPotato"

#define PLUGIN_BASE_PATH            "Data\\SKSE\\Plugins\\"
#define PLUGIN_BASE_PATHW           L"Data\\SKSE\\Plugins\\"

constexpr const char* PLUGIN_LOG_PATH = "My Games\\Skyrim Special Edition\\SKSE\\" PLUGIN_NAME ".log";
constexpr const char* PLUGIN_INI_FILE = PLUGIN_BASE_PATH PLUGIN_NAME ".ini";
constexpr const wchar_t* OSD_FONT_PATH = PLUGIN_BASE_PATHW L"SDTFonts\\";

#include "version.h"
