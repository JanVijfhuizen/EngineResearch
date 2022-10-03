﻿#pragma once
#include "Module.h"
#include "Jlb/Arena.h"

namespace je
{
	class Engine
	{
		friend struct Initializer;

	public:
		struct CreateInfo final
		{
			size_t persistentArenaSize = 4096;
			size_t tempArenaSize = 4096;
			size_t dumpArenaSize = 4096;

			[[nodiscard]] size_t GetMemorySpaceRequired() const;
		};

		Engine(const CreateInfo& info = {});
		virtual ~Engine();

		[[nodiscard]] size_t Run();

	protected:
		struct Initializer final
		{
			friend Engine;

			template <typename T>
			void AddModule() const;

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
		Module* _linkedModules = nullptr;

		template <typename T>
		void AddModule();
	};

	template <typename T>
	void Engine::Initializer::AddModule() const
	{
		_engine->AddModule<T>();
	}

	template <typename T>
	void Engine::AddModule()
	{
		T* instance = _persistentArena.New<T>();
		Module* mod = dynamic_cast<Module*>(instance);
		static_assert(dynamic_cast<Module*>(instance));
		mod->_next = _linkedModules;
		_linkedModules = mod;
	}
}
