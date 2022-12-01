#pragma once
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"
#include <Jlb/Archetype.h>

namespace je::ecs
{
	struct Entity final
	{
		bool alive = true;
	};

	class Cecsar final
	{
	public:
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

		template <typename ...Args>
		[[nodiscard]] size_t DefineArchetype(size_t batchSize = 16);

		template <typename ...Args>
		void Add(size_t archetype, Tuple<Entity, Args...>& entity);

		template <typename ...Args>
		[[nodiscard]] Scope<Args...> CreateScope();

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
