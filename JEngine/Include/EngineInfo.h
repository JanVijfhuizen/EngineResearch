#pragma once

namespace je
{
	struct ModuleFinder;
	class Arena;
	class Engine;

	// Used to communicate with the engine from the modules.
	struct EngineInfo final
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
		explicit EngineInfo(Engine& engine, const ModuleFinder& finder);
	};
}