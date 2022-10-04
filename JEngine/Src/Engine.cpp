﻿#include "pch.h"
#include "Engine.h"
#include "ModuleFinder.h"
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
			const Initializer initializer{ *this };
			DefineAdditionalModules(initializer);
		}

		ModuleFinder finder{ _persistentArena, _linkedModules };
		for (auto& [ptr, hashCode] : _linkedModules)
			finder._map.Insert(ptr, hashCode);

		_running = false;
		return EXIT_SUCCESS;
	}

	Engine::Initializer::Initializer(Engine& engine) : _engine(&engine)
	{

	}
}
