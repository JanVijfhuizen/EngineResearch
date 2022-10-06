#include "pch.h"
#include "EngineInitializer.h"

namespace je::engine
{
	Initializer::Initializer(Engine& engine) :
		_persistentArena(engine._persistentArena), _linkedModules(engine._tempArena)
	{
		
	}
}