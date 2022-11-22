#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"

namespace je::ecs
{
	class Archetype final
	{
	public:
		template <typename ...Args>
		[[nodiscard]] static Archetype Create(Arena& arena, size_t capacity);

		template <typename ...Args>
		[[nodiscard]] size_t Add(const Tuple<Args...>& entity);
		[[nodiscard]] size_t Remove(size_t index);

	private:
		struct Batch final
		{
			void** components;
		};

		Arena* _arena = nullptr;
		LinkedList<Batch> _batches{};
		Array<size_t> _sizes{};
		size_t _capacity = 0;
		size_t _count = 0;

		void AddBatch();

		template <typename Head, typename ...Tail>
		void DefineSizes(size_t index);
		template <size_t I, typename ...Args, typename Head, typename ...Tail>
		void DefineComponents(Batch& batch, const Tuple<Args...>& entity, size_t entityIndex);
	};

	template <typename ...Args>
	Archetype Archetype::Create(Arena& arena, const size_t capacity)
	{
		Archetype archetype{};
		archetype._arena = &arena;
		archetype._batches = CreateLinkedList<Batch>();
		archetype._capacity = capacity;
		archetype._sizes = CreateArray<size_t>(arena, sizeof...(Args));
		archetype.DefineSizes<Args...>(0);
		return archetype;
	}

	template <typename ... Args>
	size_t Archetype::Add(const Tuple<Args...>& entity)
	{
		const size_t c = _batches.GetCount();
		if (_count == c * _capacity)
			AddBatch();

		auto& batch = _batches[c - _count / _capacity];
		const size_t index = _count % _capacity;

		DefineComponents<0, Args..., Args...>(batch, entity, index);
		return _count++;
	}

	template <typename Head, typename ... Tail>
	void Archetype::DefineSizes(const size_t index)
	{
		static_assert(sizeof(Head) >= sizeof(void*));
		_sizes[index] = sizeof(Head);
		if constexpr (sizeof...(Tail) > 0)
			DefineSizes<Tail...>(index + 1);
	}

	template <size_t I, typename ...Args, typename Head, typename ...Tail>
	void Archetype::DefineComponents(Batch& batch, const Tuple<Args...>& entity, const size_t entityIndex)
	{
		Head* arr = static_cast<Head>(batch.components);
		arr[entityIndex] = Get<I>(entity);
		if constexpr (sizeof...(Tail) > 0)
			DefineComponents<I + 1, Args..., Tail>(batch, entity, entityIndex);
	}
}
