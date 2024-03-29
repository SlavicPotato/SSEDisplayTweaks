#pragma once

#include "macro_helpers.h"
#include <skse_version.h>

#define PLUGIN_VERSION_MAJOR    0
#define PLUGIN_VERSION_MINOR    5
#define PLUGIN_VERSION_REVISION 12

#define PLUGIN_VERSION_APPEND_STR ""

#define PLUGIN_VERSION_VERSTRING \
	STR(PLUGIN_VERSION_MAJOR)    \
	"." STR(PLUGIN_VERSION_MINOR) "." STR(PLUGIN_VERSION_REVISION) PLUGIN_VERSION_APPEND_STR

#define MAKE_PLUGIN_VERSION(major, minor, rev) (((major & 0xFF) << 16) | ((minor & 0xFF) << 8) | (rev & 0xFF))
