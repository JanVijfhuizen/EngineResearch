#include "pch.h"
#include "JEngine/EngineInfo.h"

namespace je::engine
{
	Info::Info(Arena& persistentArena, Arena& tempArena, Arena& dumpArena, const Finder<Module>& finder) :
		persistentArena(persistentArena),
		tempArena(tempArena),
		dumpArena(dumpArena),
		finder(finder)
	{

	}
}
