#pragma once
#include "JEngine/Module.h"
#include "Jlb/Arena.h"
#include "Jlb/Finder.h"

namespace je
{
	namespace engine
	{
		struct Info;
	}

	struct EngineCreateInfo final
	{
		size_t persistentArenaSize = 4096;
		size_t tempArenaSize = 4096;
		size_t frameArenaSize = 4096;

		[[nodiscard]] size_t GetMemorySpaceRequired() const;
	};

	struct EngineRunInfo final
	{
		void(*defineAdditionalModules)(Arena& frameArena, Finder<Module>::Initializer& initializer);
	};

	// Engine class that manages all the modules, like the windowing, resource manager or rendering.
	class Engine final
	{
		friend engine::Info;

	public:
		explicit Engine(const EngineCreateInfo& info = {});
		~Engine();
		[[nodiscard]] size_t Run(const EngineRunInfo& runInfo);

	private:
		void* _memory;
		Arena _persistentArena;
		Arena _tempArena;
		Arena _frameArena;
		bool _running = false;
	};
}
