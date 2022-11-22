#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"
#include "vcruntime_typeinfo.h"

namespace je::ecs
{
	class Archetype final
	{
	public:
		struct Batch final
		{
			void** components = nullptr;
			size_t count = 0;
		};

		template <typename ...Args>
		struct View final
		{
			friend Archetype;

		private:
			size_t _indexes[sizeof...(Args)]{};
			Archetype* _archetype{};
		};

		struct Iterator final
		{
			LinkedList<Batch>* linked = nullptr;
			size_t count = 0;
			size_t capacity = 0;

			const Batch& operator*() const;
			const Batch& operator->() const;

			const Iterator& operator++();
			Iterator operator++(int);

			friend bool operator==(const Iterator& a, const Iterator& b)
			{
				return a.linked == b.linked;
			}

			friend bool operator!= (const Iterator& a, const Iterator& b)
			{
				return !(a == b);
			}
		};

		template <typename ...Args>
		[[nodiscard]] static Archetype Create(Arena& arena, size_t capacity);

		template <typename ...Args>
		[[nodiscard]] size_t Add(Tuple<Args...>& entity);
		[[nodiscard]] size_t Remove(size_t index);

		template <typename T>
		[[nodiscard]] bool Contains(size_t& outIndex) const;

		[[nodiscard]] Iterator begin() const;
		[[nodicard]] static Iterator end();

		template <typename ...Args>
		[[nodiscard]] View<Args...> GetView();

	private:
		Arena* _arena = nullptr;
		LinkedList<Batch> _batches{};
		Array<size_t> _sizes{};
		Array<size_t> _typeIds{};
		size_t _capacity = 0;
		size_t _count = 0;

		void AddBatch();

		template <typename Head, typename ...Tail>
		void DefineSizes(size_t index);
		template <typename Head, typename ...Tail>
		void DefineTypeIds(size_t index);
		template <size_t I, typename U, typename Head, typename ...Tail>
		void DefineComponents(Batch& batch, U& entity, size_t entityIndex);
		template <typename T, typename Head, typename ...Tail>
		void DefineViewIndex(T& view, size_t index);
	};

	template <typename ...Args>
	Archetype Archetype::Create(Arena& arena, const size_t capacity)
	{
		Archetype archetype{};
		archetype._arena = &arena;
		archetype._batches = CreateLinkedList<Batch>();
		archetype._capacity = capacity;
		archetype._sizes = CreateArray<size_t>(arena, sizeof...(Args));
		archetype._typeIds = CreateArray<size_t>(arena, sizeof...(Args));
		archetype.DefineSizes<Args...>(0);
		archetype.DefineTypeIds<Args...>(0);
		return archetype;
	}

	template <typename ...Args>
	size_t Archetype::Add(Tuple<Args...>& entity)
	{
		if (_count == _batches.GetCount() * _capacity)
			AddBatch();

		auto& batch = _batches[_batches.GetCount() - 1 - _count / _capacity];
		++batch.count;
		const size_t index = _count % _capacity;

		DefineComponents<0, Tuple<Args...>, Args...>(batch, entity, index);
		return _count++;
	}

	template <typename T>
	bool Archetype::Contains(size_t& outIndex) const
	{
		const size_t tId = typeid(T).hash_code();

		size_t i = 0;
		for (const auto& typeId : _typeIds)
		{
			if (typeId == tId)
			{
				outIndex = i;
				return true;
			}
			++i;
		}
			
		return false;
	}

	template <typename ... Args>
	Archetype::View<Args...> Archetype::GetView()
	{
		View<Args...> view{};
		view._archetype = this;
		DefineViewIndex<View<Args...>, Args...>(view, 0);
		return view;
	}

	template <typename Head, typename ... Tail>
	void Archetype::DefineSizes(const size_t index)
	{
		_sizes[index] = sizeof(Head);
		if constexpr (sizeof...(Tail) > 0)
			DefineSizes<Tail...>(index + 1);
	}

	template <typename Head, typename ... Tail>
	void Archetype::DefineTypeIds(size_t index)
	{
		_typeIds[index] = typeid(Head).hash_code();
		if constexpr (sizeof...(Tail) > 0)
			DefineSizes<Tail...>(index + 1);
	}

	template <size_t I, typename U, typename Head, typename ... Tail>
	void Archetype::DefineComponents(Batch& batch, U& entity, size_t entityIndex)
	{
		void* vPtr = batch.components[I];
		Head* arr = static_cast<Head*>(vPtr);
		arr[entityIndex] = Get<I>(entity);
		if constexpr (sizeof...(Tail) > 0)
			DefineComponents<I + 1, U, Tail...>(batch, entity, entityIndex);
	}

	template <typename T, typename Head, typename ... Tail>
	void Archetype::DefineViewIndex(T& view, const size_t index)
	{
		_typeIds[index] = typeid(Head).hash_code();

		size_t i = 0;
		const auto contains = Contains<Head>(i);
		assert(contains);
		view._indexes[index] = i;

		if constexpr (sizeof...(Tail) > 0)
			DefineSizes<Tail...>(index + 1);
	}
}
