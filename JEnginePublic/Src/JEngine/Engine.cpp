#include "JEngine/pch.h"
#include "JEngine/Engine.h"
#include "JEngine/EngineInfo.h"
#include "Jlb/Finder.h"
#include "JEngine/Modules/TimeModule.h"
#include "JEngine/Modules/WindowModule.h"
#include "JEngine/Modules/RenderModule.h"

namespace je
{
	size_t EngineCreateInfo::GetMemorySpaceRequired() const
	{
		return persistentArenaSize + tempArenaSize + dumpArenaSize;
	}

	Engine::Engine(const EngineCreateInfo& info) :
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

	size_t Engine::Run(const EngineRunInfo& runInfo)
	{
		assert(!_running);
		_running = true;

		assert(runInfo.renderModuleCreateInfo);
		assert(runInfo.defineAdditionalModules);

		const auto _ = _persistentArena.CreateScope();
		
		Finder<Module> finder{_persistentArena};

		{
			Finder<Module>::Initializer initializer{_persistentArena, _tempArena};
			
			initializer.Add<engine::WindowModule>();
			initializer.Add<engine::TimeModule>();
			initializer.Add<engine::RenderModule>(*runInfo.renderModuleCreateInfo);
			runInfo.defineAdditionalModules(_dumpArena, initializer);
			
			finder.Compile(initializer);
		}
		
		engine::Info info{_persistentArena, _tempArena, _dumpArena, finder};

		for (const auto& mod : finder)
			mod->OnInitialize(info);
		for (const auto& mod : finder)
			mod->OnBegin(info);

		_dumpArena.Empty();

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
