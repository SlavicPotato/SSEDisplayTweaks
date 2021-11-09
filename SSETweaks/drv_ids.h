#pragma once

namespace SDT
{
	enum class DRIVER_ID : std::uint32_t
	{
		INVALID,
		EVENTS,
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