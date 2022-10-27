#ifndef PCH_H
#define PCH_H

#include <ext/ICommon.h>

#include <skse64/FormTraits.h>

#include <ext/GameHandlesExtra.h>

#include <ext/ID3D11.h>
#include <ext/IHook.h>
#include <ext/INIReader.h>
#include <ext/IPluginInfo.h>
#include <ext/ITasks.h>
#include <ext/JITASM.h>
#include <ext/Patching.h>
#include <ext/StrHelpers.h>
#include <ext/stl_containers.h>
#include <ext/stl_allocator_mm.h>
#include <ext/stl_math.h>
#include <ext/str_conv.h>

#include <skse64/GameData.h>
#include <skse64/GameEvents.h>
#include <skse64/GameMenus.h>
#include <skse64/GameSettings.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <d3d11.h>
#include <dxgi1_6.h>
#include <shlobj.h>
#include <wrl/client.h>

#include <directxtk/CommonStates.h>
#include <directxtk/SpriteFont.h>

#include "common.h"
#include "config.h"

#include "drv_base.h"

#include "dispatcher.h"

#include "data.h"

#include "controls.h"
#include "drv_ids.h"
#include "events.h"
#include "game.h"
#include "havok.h"
#include "helpers.h"
#include "input.h"
#include "misc.h"
#include "osd.h"
#include "papyrus.h"
#include "plugin.h"
#include "render.h"
#include "resource.h"
#include "skse.h"
#include "stats.h"
#include "window.h"

#endif  //PCH_H
