#include "pch.h"
#include "ModuleFinder.h"

namespace je
{
	ModuleFinder::ModuleFinder(Arena& arena, const LinkedList<KeyPair<Module*>>& modules) :
		_map(arena, modules.GetCount())
	{

	}
}