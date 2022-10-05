#include "pch.h"
#include "EngineInfo.h"

#include "Engine.h"

namespace je
{
	EngineInfo::EngineInfo(Engine& engine, const ModuleFinder& finder) :
		persistentArena(engine._persistentArena),
		tempArena(engine._tempArena),
		dumpArena(engine._dumpArena),
		finder(finder)
	{

	}
}
