#pragma once
#include "Module.h"
#include "Jlb/Arena.h"
#include "Jlb/JMap.h"
#include "Jlb/LinkedList.h"

namespace je
{
	class Engine
	{
		friend struct EngineInfo;
		friend struct EngineInitializer;

	public:
		struct CreateInfo final
		{
			size_t persistentArenaSize = 4096;
			size_t tempArenaSize = 4096;
			size_t dumpArenaSize = 4096;

			[[nodiscard]] size_t GetMemorySpaceRequired() const;
		};

		explicit Engine(const CreateInfo& info = {});
		virtual ~Engine();

		[[nodiscard]] size_t Run();

	protected:
		virtual void DefineAdditionalModules(EngineInitializer& initializer) = 0;

	private:
		void* _memory;
		Arena _persistentArena;
		Arena _tempArena;
		Arena _dumpArena;
		bool _running = false;
	};
}
