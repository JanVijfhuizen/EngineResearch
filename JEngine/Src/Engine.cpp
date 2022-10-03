#include "pch.h"
#include "Engine.h"

#include "Jlb/LinkedList.h"

namespace je
{
	size_t Engine::CreateInfo::GetMemorySpaceRequired() const
	{
		return persistentArenaSize + tempArenaSize + dumpArenaSize;
	}

	Engine::ModuleFinder::ModuleFinder(Engine& engine) : _map(engine._persistentArena, engine._linkedModules.GetCount())
	{

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
		{
			const Initializer initializer{ *this };
			DefineAdditionalModules(initializer);
		}

		ModuleFinder finder{ *this };
		for (auto& [ptr, hashCode] : _linkedModules)
			finder._map.Insert(ptr, hashCode);

		return EXIT_SUCCESS;
	}

	Engine::Initializer::Initializer(Engine& engine) : _engine(&engine)
	{

	}
}
