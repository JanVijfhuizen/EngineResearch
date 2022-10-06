#pragma once

namespace je
{
	class Arena;
	class Engine;

	namespace engine
	{
		struct ModuleFinder;

		// Used to communicate with the engine from the modules.
		struct Info final
		{
			friend Engine;

			// Doesn't get deallocated until the engine goes out of scope. No destructors will be called.
			Arena& persistentArena;
			// Doesn't get deallocated automatically, meant for temporary 
			Arena& tempArena;
			Arena& dumpArena;
			const ModuleFinder& finder;
			bool quit = false;

		private:
			explicit Info(Engine& engine, const ModuleFinder& finder);
		};
	}
}