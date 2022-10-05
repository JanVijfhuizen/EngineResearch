#pragma once
#include "Module.h"
#include "Jlb/Arena.h"
#include "Jlb/JMap.h"
#include "Jlb/LinkedList.h"
#include <vcruntime_typeinfo.h>

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
		virtual void DefineAdditionalModules(const EngineInitializer& initializer) = 0;

	private:
		void* _memory;
		Arena _persistentArena;
		Arena _tempArena;
		Arena _dumpArena;
		LinkedList<KeyPair<Module*>> _linkedModules;
		bool _running = false;

		template <typename T>
		void AddModule();
	};

	template <typename T>
	void Engine::AddModule()
	{
		const size_t key = typeid(T).hash_code();
		/*
		for (const auto& mod : _linkedModules)
		{
			if (mod.key == key)
				return;
		}
		*/
		KeyPair<Module*> pair{};
		pair.value = _persistentArena.New<T>();
		pair.key = key;
		_linkedModules.Add(pair);
	}
}
