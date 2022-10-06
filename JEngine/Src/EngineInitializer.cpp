#include "pch.h"
#include "EngineInitializer.h"

namespace je
{
	EngineInitializer::EngineInitializer(Engine& engine) :
		_persistentArena(engine._persistentArena), _linkedModules(engine._tempArena)
	{
		
	}
}