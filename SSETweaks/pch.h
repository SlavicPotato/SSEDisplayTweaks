#ifndef PCH_H
#define PCH_H

#include "common/IMemPool.h"

#include "skse64_common/skse_version.h"
#include "skse64_common/SafeWrite.h"
#include "skse64_common/BranchTrampoline.h"

#include "skse64/PluginAPI.h"
#include "xbyak/xbyak.h"

#include "ext/ICommon.h"
#include "ext/IHook.h"
#include "skse64/GameData.h"
#include "skse64/GameEvents.h"
#include "skse64/GameMenus.h"
#include "skse64/GameSettings.h"
#include "skse64/GameRTTI.h"

#include <string>
#include <vector>
#include <sstream> 
#include <algorithm>
#include <map>
#include <unordered_map>
#include <functional>

#include <shlobj.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Inc/SpriteFont.h"
#include "Inc/CommonStates.h"

#include "resource.h"

#include "data.h"

#define INVALID_DRIVER	-1
#define DRIVER_EVENTS	1
#define DRIVER_TASKS	2
#define DRIVER_RENDER	3
#define DRIVER_WINDOW	4
#define DRIVER_PAPYRUS	5
#define DRIVER_MISC		6
#define DRIVER_ANIM		7
#define DRIVER_MFG		8
#define DRIVER_HAVOK	9
#define DRIVER_CONTROLS	10
#define DRIVER_INPUT	11
#define DRIVER_OSD  	12

#include "plugin.h"

#define PLUGIN_BASE_PATH       "Data\\SKSE\\Plugins\\"
#define PLUGIN_BASE_PATHW      L"Data\\SKSE\\Plugins\\"

constexpr char* PLUGIN_LOG_PATH = "\\My Games\\Skyrim Special Edition\\SKSE\\" PLUGIN_NAME ".log";
constexpr char* PLUGIN_INI_FILE = PLUGIN_BASE_PATH PLUGIN_NAME ".ini";
constexpr wchar_t* OSD_FONT_PATH = PLUGIN_BASE_PATHW L"SDTFonts\\";

#include "helpers.h"
#include "common.h"
#include "INIReader.h"
#include "patching.h"
#include "stats.h"
#include "logging.h"
#include "os.h"
#include "skse.h"
#include "config.h"
#include "drv_base.h"
#include "dispatcher.h"
#include "jitasm.h"
#include "events.h"
#include "input.h"
#include "render.h"
#include "osd.h"
#include "havok.h"
#include "controls.h"
#include "window.h"
#include "papyrus.h"
#include "misc.h"

#endif //PCH_H
