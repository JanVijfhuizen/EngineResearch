#pragma once
#include "Module.h"
#include "Jlb/Finder.h"

namespace je
{
	class Arena;
	class Engine;

	namespace engine
	{
		// Used to communicate with the engine from the modules.
		struct Info final
		{
			friend Engine;

			// Doesn't get deallocated until the engine goes out of scope. No destructors will be called.
			Arena& persistentArena;
			// Doesn't get deallocated automatically, meant for temporary 
			Arena& tempArena;
			Arena& dumpArena;
			const Finder<Module>& finder;
			bool quit = false;

		private:
			explicit Info(Engine& engine, const Finder<Module>& finder);
		};
	}
}
