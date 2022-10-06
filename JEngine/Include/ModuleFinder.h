#pragma once
#include "Module.h"
#include "Jlb/JMap.h"
#include "Jlb/KeyPair.h"
#include "Jlb/LinkedList.h"
#include <vcruntime_typeinfo.h>

namespace je
{
	class Engine;
}

namespace je::engine
{
	// Interface to find modules loaded by the engine.
	struct ModuleFinder final
	{
		friend Engine;

		template <typename T>
		[[nodiscard]] T* Get() const;

		[[nodiscard]] LinkedList<KeyPair<Module*>>::Iterator begin() const;
		[[nodiscard]] LinkedList<KeyPair<Module*>>::Iterator end() const;

	private:
		Map<Module*> _map;
		const LinkedList<KeyPair<Module*>>& _linkedList;

		explicit ModuleFinder(Arena& arena, const LinkedList<KeyPair<Module*>>& modules);
	};

	template <typename T>
	T* ModuleFinder::Get() const
	{
		Module* mod = *_map.Contains(typeid(T).hash_code());
		assert(mod);
		return static_cast<T*>(mod);
	}
}
