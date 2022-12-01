#pragma once
#include "Module.h"
#include "Jlb/Arena.h"
#include "Jlb/Finder.h"

namespace je
{
	namespace engine
	{
		struct Info;
		struct RenderModuleCreateInfo;
	}

	struct EngineCreateInfo final
	{
		size_t persistentArenaSize = 4096;
		size_t tempArenaSize = 4096;
		size_t dumpArenaSize = 4096;

		[[nodiscard]] size_t GetMemorySpaceRequired() const;
	};

	struct EngineRunInfo final
	{
		engine::RenderModuleCreateInfo* renderModuleCreateInfo = nullptr;
	};

	// Engine class that manages all the modules, like the windowing, resource manager or rendering.
	class Engine
	{
		friend engine::Info;

	public:
		explicit Engine(const EngineCreateInfo& info = {});
		virtual ~Engine();

		[[nodiscard]] size_t Run(const EngineRunInfo& runInfo);

	protected:
		// Define what additional modules are loaded into the engine. This can include a game manager, or a movement system for example.
		virtual void DefineAdditionalModules(Arena& dumpArena, Finder<Module>::Initializer& initializer) = 0;

	private:
		void* _memory;
		Arena _persistentArena;
		Arena _tempArena;
		Arena _dumpArena;
		bool _running = false;
	};
}
