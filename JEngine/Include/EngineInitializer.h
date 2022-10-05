#pragma once
#include "Engine.h"
#include <vcruntime_typeinfo.h>

namespace je
{
	struct EngineInitializer final
	{
		friend Engine;

		template <typename T>
		void AddModule();

	private:
		Arena& _persistentArena;
		LinkedList<KeyPair<Module*>> _linkedModules;

		explicit EngineInitializer(Engine& engine);
	};

	template <typename T>
	void EngineInitializer::AddModule()
	{
		KeyPair<Module*> pair{};
		pair.value = _persistentArena.New<T>();
		pair.key = typeid(T).hash_code();
		_linkedModules.Add(pair);
	}
}