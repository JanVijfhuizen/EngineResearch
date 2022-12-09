#include "JEngine/pch.h"
#include "JEngine/EngineInfo.h"

namespace je::engine
{
	Info::Info(Arena& persistentArena, Arena& tempArena, Arena& frameArena, const Finder<Module>& finder) :
		persistentArena(persistentArena),
		tempArena(tempArena),
		frameArena(frameArena),
		finder(finder)
	{

	}
}
