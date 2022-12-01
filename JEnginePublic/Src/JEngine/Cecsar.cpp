#include "JEngine/pch.h"
#include "JEngine/Cecsar.h"

namespace je::ecs
{
	Cecsar::Cecsar(Arena& arena) : _arena(arena)
	{
		_archetypes = CreateLinkedList<Archetype>();
	}

	void Cecsar::PreUpdate()
	{
		auto scope = CreateScope<Entity>();
		scope.Iterate([](Entity& e)
			{
				return e.alive;
			});
	}
}