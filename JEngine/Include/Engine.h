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
		friend struct Initializer;
		friend struct EngineInfo;

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
		struct Initializer final
		{
			friend Engine;

			template <typename T, typename ...Args>
			void AddModule(Args... args) const;

		private:
			Engine* _engine = nullptr;

			explicit Initializer(Engine& engine);
		};
		
		virtual void DefineAdditionalModules(const Initializer& initializer) = 0;

	private:
		void* _memory;
		Arena _persistentArena;
		Arena _tempArena;
		Arena _dumpArena;
		LinkedList<KeyPair<Module*>> _linkedModules;
		bool _running = false;

		template <typename T, typename ...Args>
		void AddModule(Args... args);
	};

	template <typename T, typename ...Args>
	void Engine::Initializer::AddModule(Args... args) const
	{
		_engine->AddModule<T>(args...);
	}

	template <typename T, typename ...Args>
	void Engine::AddModule(Args... args)
	{
		KeyPair<Module*> pair{};
		pair.value = _persistentArena.New<T>(1, args...);
		pair.key = typeid(T).hash_code();
		_linkedModules.Add(pair);
	}
}
