#pragma once

namespace SDT
{
	enum class DRIVER_ID : std::uint32_t
	{
		INVALID = static_cast<std::underlying_type_t<DRIVER_ID>>(-1),

		EVENTS = 0,
		WINDOW,
		HAVOK,
		CONTROLS,
		OSD,
		INPUT,
		MISC,
		RENDER,
		PAPYRUS
	};
}