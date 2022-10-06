#pragma once
#include "Engine.h"
#include <vcruntime_typeinfo.h>

#include "Jlb/KeyPair.h"
#include "Jlb/LinkedList.h"

namespace je
{
	class Engine;

	namespace engine
	{
		// Used to initialize the modules for the engine.
		struct Initializer final
		{
			friend Engine;

			// Load in a module for the engine.
			template <typename T>
			void AddModule();

		private:
			Arena& _persistentArena;
			LinkedList<KeyPair<Module*>> _linkedModules;

			explicit Initializer(Engine& engine);
		};

		template <typename T>
		void Initializer::AddModule()
		{
			KeyPair<Module*> pair{};
			pair.value = _persistentArena.New<T>();
			pair.key = typeid(T).hash_code();
			_linkedModules.Add(pair);
		}
	}

}