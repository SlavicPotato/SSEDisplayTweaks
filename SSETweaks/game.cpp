#include "pch.h"

namespace SDT
{
	namespace Game
	{
		const float* g_frameTimer = IAL::Addr<const float*>(AID::FrameTimerNoSlow, 410200);
		const float* g_frameTimerSlow = IAL::Addr<const float*>(AID::FrameTimerSlow, 410199);
	}
}