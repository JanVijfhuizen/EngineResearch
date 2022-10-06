﻿#include "pch.h"
#include "Engine.h"
#include "EngineInfo.h"
#include "EngineInitializer.h"
#include "ModuleFinder.h"
#include "ResourceModule.h"
#include "TimeModule.h"
#include "WindowModule.h"
#include "Jlb/LinkedList.h"

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

		LinkedList<KeyPair<Module*>> linkedModules{_persistentArena};

		{
			engine::Initializer initializer{ *this };
			initializer.AddModule<engine::ResourceModule>();
			initializer.AddModule<engine::WindowModule>();
			initializer.AddModule<engine::TimeModule>();
			DefineAdditionalModules(initializer);
			
			for (auto& mod : initializer._linkedModules)
				linkedModules.Add(mod);
		}

		engine::ModuleFinder finder{ _persistentArena, linkedModules };
		for (auto& [ptr, hashCode] : linkedModules)
			finder._map.Insert(ptr, hashCode);

		engine::Info info{*this, finder};

		{
			const auto _ = _dumpArena.CreateScope();

			for (const auto& [value, hashCode] : linkedModules)
				value->OnInitialize(info);
			for (const auto& [value, hashCode] : linkedModules)
				value->OnBegin(info);
		}

		while(!info.quit)
		{
			const auto _ = _dumpArena.CreateScope();

			for (const auto& [ptr, hashCode] : linkedModules)
				ptr->OnUpdate(info);
			for (const auto& [ptr, hashCode] : linkedModules)
				ptr->OnPostUpdate(info);
		}

		{
			const auto _ = _dumpArena.CreateScope();
			for (const auto& [ptr, hashCode] : linkedModules)
				ptr->OnExit(info);
		}
		
		_running = false;
		return EXIT_SUCCESS;
	}
}
