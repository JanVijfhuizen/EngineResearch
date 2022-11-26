#pragma once
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"

namespace je::ecs
{
	class Cecsar;

	struct CEntityInfo final
	{
		friend Cecsar;

		bool markedForDelete = false;

		[[nodiscard]] size_t GetIndex() const;

	private:
		size_t _index = SIZE_MAX;
	};

	struct Entity final
	{
		size_t index = SIZE_MAX;
		size_t id = SIZE_MAX;

		[[nodiscard]] bool operator==(const Entity& other) const;
	};

	inline size_t CEntityInfo::GetIndex() const
	{
		return _index;
	}

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

		template <typename ...Args>
		void Iterate(void(*func)(Args&...));
		template <typename ...Args>
		void TryInstance(const Entity& entity, void(*func)(Args&...));

		void RemoveMarked();

	private:
		struct IntEntity final
		{
			size_t id = SIZE_MAX;
			Archetype* archetype = 0;
			size_t archetypeIndex = SIZE_MAX;
		};

		struct Batch final
		{
			size_t open = SIZE_MAX;
			size_t count = 0;
			IntEntity* entities = nullptr;
		};
		
		Arena& _arena;
		const size_t _batchSize;
		LinkedList<Batch> _batches;
		LinkedList<Archetype*> _archetypes;
		size_t _globalId = 0;
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
		size_t i = 0;
		size_t j = 0;
		bool fit = false;

		Get<0>(entity)._index = _globalId;

		for (auto& batch : _batches)
		{
			size_t index = batch.count;

			if(batch.open != SIZE_MAX)
			{
				index = batch.open;
				// Use the id as a pointer to the next open slot, if there is one.
				batch.open = batch.entities[batch.open].id;
			}

			if(index < _batchSize)
			{
				IntEntity intEntity{};
				intEntity.archetype = _archetypes[archetype];
				intEntity.archetypeIndex = intEntity.archetype->Add(entity);
				intEntity.id = _globalId;

				batch.entities[index] = intEntity;
				j = index;
				fit = true;
				++batch.count;
				break;
			}

			i += _batchSize;
		}

		if(!fit)
		{
			auto& batch = LinkedListAdd(_batches, _arena);
			batch.entities = _arena.New<IntEntity>(_batchSize);
			return Add(archetype, entity);
		}

		Entity entity{};
		entity.id = _globalId++;
		entity.index = (_batches.GetCount() - 1 - i) * _batchSize + j;
		return entity;
	}

	template <typename ... Args>
	void Cecsar::Iterate(void(* func)(Args&...))
	{
		for (auto& archetype : _archetypes)
		{
			const auto view = archetype->GetView<CEntityInfo, Args...>();
			view.TryIterate(func);
		}
	}

	template <typename ...Args>
	void Cecsar::TryInstance(const Entity& entity, void(*func)(Args&...))
	{
		const auto& archetype = _archetypes[_archetypes.GetCount() - 1 - entity.index / _batchSize];
		const auto view = archetype->GetView<CEntityInfo, Args...>();
		view.TryInstance(func);
	}

	inline bool Cecsar::IsAlive(const Entity& entity)
	{
		const size_t c = _batches.GetCount();
		const auto& batch = _batches[c - 1 - entity.index / _batchSize];
		return batch.entities[entity.index % _batchSize].id == entity.id;
	}

	inline void Cecsar::RemoveMarked()
	{
		for (auto& archetype : _archetypes)
		{
			const auto view = archetype->GetView<CEntityInfo>();
			// Remove.
		}
	}

	inline Cecsar::Cecsar(Arena& arena, const size_t batchSize) : _arena(arena), _batchSize(batchSize)
	{
		_batches = CreateLinkedList<Batch>();
		_archetypes = CreateLinkedList<Archetype*>();
	}
}
