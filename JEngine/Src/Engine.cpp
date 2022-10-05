#include "pch.h"
#include "Engine.h"

#include "EngineInfo.h"
#include "EngineInitializer.h"
#include "ModuleFinder.h"
#include "Window.h"
#include "Jlb/LinkedList.h"

namespace je
{
	size_t Engine::CreateInfo::GetMemorySpaceRequired() const
	{
		return persistentArenaSize + tempArenaSize + dumpArenaSize;
	}

	Engine::Engine(const CreateInfo& info) :
		_memory(malloc(info.GetMemorySpaceRequired())),
		_persistentArena(_memory, info.persistentArenaSize),
		_tempArena(static_cast<unsigned char*>(_memory) + info.persistentArenaSize, info.tempArenaSize),
		_dumpArena(static_cast<unsigned char*>(_memory) + info.persistentArenaSize + info.tempArenaSize, info.dumpArenaSize),
		_linkedModules(_persistentArena)
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

		{
			EngineInitializer initializer{ *this };
			initializer.AddModule<engine::Window>();
			DefineAdditionalModules(initializer);

			// todo move content to _linkedModules.
		}

		ModuleFinder finder{ _persistentArena, _linkedModules };
		for (auto& [ptr, hashCode] : _linkedModules)
			finder._map.Insert(ptr, hashCode);

		EngineInfo info{*this, finder};

		for (const auto& [value, hashCode] : _linkedModules)
			value->OnInitialize(info);
		for (const auto& [value, hashCode] : _linkedModules)
			value->OnBegin(info);

		while(!info.quit)
		{
			for (const auto& [ptr, hashCode] : _linkedModules)
				ptr->OnUpdate(info);
			for (const auto& [ptr, hashCode] : _linkedModules)
				ptr->OnPostUpdate(info);

			_dumpArena.Empty();
		}

		for (const auto& [ptr, hashCode] : _linkedModules)
			ptr->OnExit(info);

		_running = false;
		return EXIT_SUCCESS;
	}
}
