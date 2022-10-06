#include "pch.h"
#include "ModuleFinder.h"

namespace je::engine
{
	LinkedList<KeyPair<Module*>>::Iterator ModuleFinder::begin() const
	{
		return _linkedList.begin();
	}

	LinkedList<KeyPair<Module*>>::Iterator ModuleFinder::end() const
	{
		return _linkedList.end();
	}

	ModuleFinder::ModuleFinder(Arena& arena, const LinkedList<KeyPair<Module*>>& modules) :
		_map(arena, modules.GetCount()), _linkedList(modules)
	{

	}
}