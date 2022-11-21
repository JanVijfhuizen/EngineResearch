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

			// Meant for stable allocations. Will be deallocated at the end of the program.
			Arena& persistentArena;
			// Meant for temporary allocations, will be deallocated at the end of every phase.
			Arena& tempArena;
			// Will be deallocated at the end of every frame.
			Arena& dumpArena;
			const Finder<Module>& finder;
			bool quit = false;

		private:
			explicit Info(Engine& engine, const Finder<Module>& finder);
		};
	}
}
