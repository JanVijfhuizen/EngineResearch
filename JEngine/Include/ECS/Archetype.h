#pragma once
#include "Jlb/Array.h"
#include "Jlb/LinkedList.h"
#include "Jlb/Tuple.h"
#include "vcruntime_typeinfo.h"

namespace je::ecs
{
	class Archetype final
	{
		template <typename ...Args>
		friend struct View;
		using Batch = void**;

	public:
		template <typename ...Args>
		struct View final
		{
			friend Archetype;

			template <typename T>
			bool TryIterate(const T& func) const;
			template <typename T>
			bool TryInstance(size_t index, const T& func) const;
			[[nodiscard]] operator bool() const;

		private:
			size_t _indexes[sizeof...(Args)]{};
			Archetype* _archetype{};
			bool _isValid = true;

			template <typename T>
			[[nodiscard]] T& GetMember(Batch& batch, size_t& memberIndex, size_t entityIndex) const;
		};

		template <typename ...Args>
		[[nodiscard]] static Archetype Create(Arena& arena, size_t batchSize);

		template <typename ...Args>
		[[nodiscard]] size_t Add(Tuple<Args...>& entity);
		[[nodiscard]] size_t Remove(size_t index);
		
		template <typename ...Args>
		[[nodiscard]] View<Args...> GetView();

	private:
		Arena* _arena = nullptr;
		LinkedList<Batch> _batches{};
		Array<size_t> _sizes{};
		Array<size_t> _typeIds{};
		size_t _batchSize = 0;
		size_t _count = 0;

		void AddBatch();

		template <typename Head, typename ...Tail>
		void DefineSizes(size_t index);
		template <typename Head, typename ...Tail>
		void DefineTypeIds(size_t index);
		template <size_t I, typename U, typename Head, typename ...Tail>
		void DefineComponents(Batch& batch, U& entity, size_t entityIndex);
		template <typename T, typename Head, typename ...Tail>
		void DefineViewIndexes(T& view, size_t index);

		template <typename T>
		[[nodiscard]] bool Contains(size_t& outIndex) const;
	};

	template <typename ...Args>
	template <typename T>
	bool Archetype::View<Args...>::TryIterate(const T& func) const
	{
		if (!_isValid)
			return false;

		const size_t capacity = _archetype->_batchSize;

		size_t count = _archetype->_count % capacity;
		for (auto& batch : _archetype->_batches)
		{
			for (int64_t i = count - 1; i >= 0; --i)
			{
				size_t m = sizeof...(Args) - 1;
				func(GetMember<Args>(batch, m, i)...);
			}
			count = capacity;
		}

		return true;
	}

	template <typename ... Args>
	template <typename T>
	bool Archetype::View<Args...>::TryInstance(const size_t index, const T& func) const
	{
		if (!_isValid)
			return false;

		auto& batches = _archetype->_batches;
		auto& batch = batches[batches.GetCount() - 1 - index / _archetype->_batchSize];
		size_t m = sizeof...(Args) - 1;
		func(GetMember<Args>(batch, m, index % _archetype->_batchSize)...);
		return true;
	}

	template <typename ... Args>
	Archetype::View<Args...>::operator bool() const
	{
		return _isValid;
	}

	template <typename ...Args>
	template <typename T>
	T& Archetype::View<Args...>::GetMember(Batch& batch, size_t& memberIndex, const size_t entityIndex) const
	{
		T* ptr = static_cast<T*>(batch[_indexes[memberIndex--]]);
		return ptr[entityIndex];
	}

	template <typename ...Args>
	Archetype Archetype::Create(Arena& arena, const size_t batchSize)
	{
		Archetype archetype{};
		archetype._arena = &arena;
		archetype._batches = CreateLinkedList<Batch>();
		archetype._batchSize = batchSize;
		archetype._sizes = CreateArray<size_t>(arena, sizeof...(Args));
		archetype._typeIds = CreateArray<size_t>(arena, sizeof...(Args));
		archetype.DefineSizes<Args...>(0);
		archetype.DefineTypeIds<Args...>(0);
		return archetype;
	}

	template <typename ...Args>
	size_t Archetype::Add(Tuple<Args...>& entity)
	{
		if (_count == _batches.GetCount() * _batchSize)
			AddBatch();

		auto& batch = _batches[_batches.GetCount() - 1 - _count / _batchSize];
		const size_t index = _count % _batchSize;

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

	template <typename ...Args>
	Archetype::View<Args...> Archetype::GetView()
	{
		View<Args...> view{};
		view._archetype = this;
		DefineViewIndexes<View<Args...>, Args...>(view, 0);
		return view;
	}

	template <typename Head, typename ...Tail>
	void Archetype::DefineSizes(const size_t index)
	{
		_sizes[index] = sizeof(Head);
		if constexpr (sizeof...(Tail) > 0)
			DefineSizes<Tail...>(index + 1);
	}

	template <typename Head, typename ...Tail>
	void Archetype::DefineTypeIds(const size_t index)
	{
		_typeIds[index] = typeid(Head).hash_code();
		if constexpr (sizeof...(Tail) > 0)
			DefineTypeIds<Tail...>(index + 1);
	}

	template <size_t I, typename U, typename Head, typename ...Tail>
	void Archetype::DefineComponents(Batch& batch, U& entity, size_t entityIndex)
	{
		void* vPtr = batch[I];
		Head* arr = static_cast<Head*>(vPtr);
		arr[entityIndex] = Get<I>(entity);
		if constexpr (sizeof...(Tail) > 0)
			DefineComponents<I + 1, U, Tail...>(batch, entity, entityIndex);
	}

	template <typename T, typename Head, typename ...Tail>
	void Archetype::DefineViewIndexes(T& view, const size_t index)
	{
		size_t i = 0;
		const auto contains = Contains<Head>(i);
		if(!contains)
		{
			view._isValid = false;
			return;
		}

		view._indexes[index] = i;

		if constexpr (sizeof...(Tail) > 0)
			DefineViewIndexes<T, Tail...>(view, index + 1);
	}
}
