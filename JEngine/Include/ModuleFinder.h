#pragma once
#include "Module.h"
#include "Jlb/JMap.h"
#include "Jlb/KeyPair.h"
#include "Jlb/LinkedList.h"

namespace je
{
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
		return static_cast<T*>(_map.Contains(typeid(T).hash_code()));
	}
}
