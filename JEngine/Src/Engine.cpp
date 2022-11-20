#include "pch.h"
#include "Engine.h"
#include "EngineInfo.h"
#include "Jlb/Finder.h"
#include "Modules/TimeModule.h"
#include "Modules/WindowModule.h"
#include "Modules/RenderModule.h"

namespace je
{
	size_t Engine::CreateInfo::GetMemorySpaceRequired() const
	{
		return persistentArenaSize + dumpArenaSize;
	}

	Engine::Engine(const CreateInfo& info) :
		_memory(malloc(info.GetMemorySpaceRequired())),
		_persistentArena(_memory, info.persistentArenaSize),
		_tempArena(static_cast<unsigned char*>(_memory) + info.persistentArenaSize, info.tempArenaSize),
		_dumpArena(static_cast<unsigned char*>(_memory) + info.persistentArenaSize + info.tempArenaSize, info.dumpArenaSize)
	{
	}

	Engine::~Engine()
	{
		free(_memory);
	}

	size_t Engine::Run()
	{
		assert(!_running);
		_running = true;

		const auto _ = _persistentArena.CreateScope();
		
		Finder<Module> finder{_persistentArena};

		{
			Finder<Module>::Initializer initializer{_persistentArena, _tempArena};
			
			initializer.Add<engine::WindowModule>();
			initializer.Add<engine::TimeModule>();
			initializer.Add<engine::RenderModule>();
			DefineAdditionalModules(initializer);
			
			finder.Compile(initializer);
		}
		
		engine::Info info{*this, finder};

		{
			const auto _ = _dumpArena.CreateScope();

			for (const auto& mod : finder)
				mod->OnInitialize(info);
			for (const auto& mod : finder)
				mod->OnBegin(info);
		}

		while(!info.quit)
		{
			const auto _ = _dumpArena.CreateScope();
			for (const auto& mod : finder)
				mod->OnUpdate(info);
		}

		{
			const auto _ = _dumpArena.CreateScope();
			for (const auto& mod : finder)
				mod->OnExit(info);
		}
		
		_running = false;
		return EXIT_SUCCESS;
	}
}
