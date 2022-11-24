#pragma once
#include "Jlb/JVector.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"

namespace je::ecs
{
	struct CEntityInfo final
	{
		bool markedForDelete = false;
	};

	struct Entity final
	{
		size_t index;
		size_t id;

		[[nodiscard]] bool operator==(const Entity& other) const;
	};

	inline bool Entity::operator==(const Entity& other) const
	{
		return index == other.index && id == other.id;
	}

	class Cecsar final
	{
	public:
		explicit Cecsar(Arena& arena, size_t batchSize = 100);

		template <typename ...Args>
		[[nodiscard]] size_t DefineArchetype();

		template <typename ...Args>
		Entity Add(size_t archetype, const Tuple<CEntityInfo, Args...>& entity);
		[[nodiscard]] bool IsAlive(const Entity& entity);
		[[nodiscard]] Archetype* TryGetArchetype(const Entity& entity);

	private:
		struct IntEntity final
		{
			size_t id = 0;
			Archetype* archetype;
			size_t archetypeIndex;
		};

		using Batch = Vector<IntEntity>;

		Arena& _arena;
		const size_t _batchSize;
		LinkedList<Batch> _batches;
		LinkedList<Archetype*> _archetypes;
	};

	template <typename ...Args>
	size_t Cecsar::DefineArchetype()
	{
		LinkedListAdd(_archetypes, _arena, Archetype::Create<CEntityInfo, Args...>());
		return _archetypes.GetCount() - 1;
	}

	template <typename ... Args>
	Entity Cecsar::Add(const size_t archetype, const Tuple<CEntityInfo, Args...>& entity)
	{
		IntEntity intEntity{};
		intEntity.archetype = _archetypes[archetype];
		intEntity.archetypeIndex = intEntity.archetype->Add(entity);

		
	}

	inline bool Cecsar::IsAlive(const Entity& entity)
	{
	}

	inline Cecsar::Cecsar(Arena& arena, const size_t batchSize) : _arena(arena), _batchSize(batchSize)
	{
		_batches = CreateLinkedList<Batch>();
		_archetypes = CreateLinkedList<Archetype*>();
	}
}
