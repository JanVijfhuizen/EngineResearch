#pragma once

namespace je
{
	struct ModuleFinder;
	class Arena;
	class Engine;

	struct EngineInfo final
	{
		friend Engine;

		Arena& persistentArena;
		Arena& tempArena;
		Arena& dumpArena;
		const ModuleFinder& finder;
		bool quit = false;

	private:
		explicit EngineInfo(Engine& engine, const ModuleFinder& finder);
	};
}