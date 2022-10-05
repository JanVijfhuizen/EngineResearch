#pragma once
#include "Module.h"
#include "Jlb/JMap.h"
#include "Jlb/KeyPair.h"
#include "Jlb/LinkedList.h"

namespace je
{
	// Interface to find modules loaded by the engine.
	struct ModuleFinder final
	{
		friend class Engine;

		template <typename T>
		[[nodiscard]] T* Get();

	private:
		Map<Module*> _map;

		explicit ModuleFinder(Arena& arena, const LinkedList<KeyPair<Module*>>& modules);
	};

	template <typename T>
	T* ModuleFinder::Get()
	{
		Module* mod = _map.Contains(typeid(T).hash_code());
		assert(mod);
		return static_cast<T*>(mod);
	}
}
