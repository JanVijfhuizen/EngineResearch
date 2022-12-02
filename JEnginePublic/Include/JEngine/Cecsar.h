#pragma once
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"
#include "JEngine/Archetype.h"

namespace je::ecs
{
	// Default entity component.
	struct Entity final
	{
		bool alive = true;
	};

	// Manages the archetypes for entities in a level.
	class Cecsar final
	{
	public:
		// Defines a set of components to iterate on.
		// Iterates through all the different archetypes that contain those components.
		template <typename ...Args>
		struct Scope final
		{
			friend Cecsar;

			template <typename T>
			void Iterate(const T& func);

		private:
			Cecsar& _cecsar;

			explicit Scope(Cecsar& cecsar);
		};

		explicit Cecsar(Arena& arena);

		// Define a new type of entity, and returns a handle to the newly created archetype.
		template <typename ...Args>
		[[nodiscard]] size_t DefineArchetype(size_t batchSize = 16);

		// Add an entity to target archetype. The entity's components must match the archetype's layout.
		template <typename ...Args>
		void Add(size_t archetype, Tuple<Entity, Args...>& entity);

		template <typename ...Args>
		[[nodiscard]] Scope<Args...> CreateScope();

		// Call this at the start of the frame.
		// Removes all dead entities from the system.
		void PreUpdate();

	private:
		Arena& _arena;
		LinkedList<Archetype> _archetypes;
	};

	template <typename ...Args>
	template <typename T>
	void Cecsar::Scope<Args...>::Iterate(const T& func)
	{
		auto funcScoped = [&func](Args&... args)
		{
			func(args...);
			return true;
		};

		for (auto& archetype : _cecsar._archetypes)
		{
			const auto view = archetype.GetView<Args...>();
			view.TryIterate(funcScoped);
		}
	}

	template <typename ...Args>
	Cecsar::Scope<Args...>::Scope(Cecsar& cecsar) : _cecsar(cecsar)
	{
		
	}

	template <typename ...Args>
	size_t Cecsar::DefineArchetype(const size_t batchSize)
	{
		LinkedListAdd(_archetypes, _arena, Archetype::Create<Entity, Args...>(_arena, batchSize));
		return _archetypes.GetCount() - 1;
	}

	template <typename ...Args>
	void Cecsar::Add(const size_t archetype, Tuple<Entity, Args...>& entity)
	{
		_archetypes[archetype].Add(entity);
	}

	template <typename ...Args>
	Cecsar::Scope<Args...> Cecsar::CreateScope()
	{
		return Scope<Args...>(*this);
	}
}
