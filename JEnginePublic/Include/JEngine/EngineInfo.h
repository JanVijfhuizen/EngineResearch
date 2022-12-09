#pragma once
#include "Module.h"
#include "Jlb/Finder.h"

namespace je
{
	class Arena;

	namespace engine
	{
		// Used to communicate with the engine from the modules.
		struct Info final
		{
			friend Engine;

			// Meant for stable allocations. Will be deallocated at the end of the program.
			Arena& arena;
			// Meant for temporary allocations, will be deallocated at the end of every phase.
			Arena& tempArena;
			// Will be deallocated at the end of every frame.
			Arena& frameArena;
			const Finder<Module>& finder;
			bool quit = false;
			
			explicit Info(Arena& persistentArena, Arena& tempArena, Arena& frameArena, const Finder<Module>& finder);
		};
	}
}
