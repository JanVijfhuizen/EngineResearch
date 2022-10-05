#include "pch.h"
#include "EngineInitializer.h"

namespace je
{
	EngineInitializer::EngineInitializer(Engine& engine) : _tempArena(engine._tempArena), _linkedModules(engine._tempArena)
	{

	}
}