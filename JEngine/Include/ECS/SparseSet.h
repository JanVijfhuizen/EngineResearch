#pragma once
#include "Jlb/Arena.h"
#include "Jlb/Iterator.h"
#include "Jlb/Swap.h"

namespace je
{
	template <typename T>
	class SparseSet final
	{
	public:
		struct Node final
		{
			T instance{};
			size_t sparseIndex = SIZE_MAX;
		};

		Node* dense = nullptr;
		size_t* sparse = nullptr;
		size_t length = 0;
		size_t count = 0;

		[[nodiscard]] Node& operator[](size_t index) const;

		Node& Insert(size_t index, const T& instance = {});
		void RemoveAt(size_t index);
		void Clear();

		[[nodiscard]] bool Contains(size_t index) const;

		[[nodiscard]] Iterator<Node> begin() const;
		[[nodiscard]] Iterator<Node> end() const;

		[[nodiscard]] static SparseSet Create(Arena& arena, size_t capacity);
		static void Destroy(const SparseSet& instance, Arena& arena);
	};

	template <typename T>
	typename SparseSet<T>::Node& SparseSet<T>::operator[](const size_t index) const
	{
		return dense[sparse[index]];
	}

	template <typename T>
	typename SparseSet<T>::Node& SparseSet<T>::Insert(const size_t index, const T& instance)
	{
		sparse[index] = count;
		return dense[count++] = {instance, index});
	}

	template <typename T>
	void SparseSet<T>::RemoveAt(const size_t index)
	{
		auto& instance = sparse[index];
		Swap(dense, instance, count--);

		auto& node = dense[instance];
		sparse[node.sparseIndex] = index;
		instance = SIZE_MAX;
	}

	template <typename T>
	void SparseSet<T>::Clear()
	{
		for (size_t i = 0; i < count; ++i)
			sparse[dense[i].sparseIndex] = SIZE_MAX;
		count = 0;
	}

	template <typename T>
	bool SparseSet<T>::Contains(const size_t index) const
	{
		return sparse[index] != SIZE_MAX;
	}

	template <typename T>
	Iterator<typename SparseSet<T>::Node> SparseSet<T>::begin() const
	{
		Iterator<Node> it{};
		it.length = count;
		it.data = dense;
		it.index = 0;
		return it;
	}

	template <typename T>
	Iterator<typename SparseSet<T>::Node> SparseSet<T>::end() const
	{
		Iterator<Node> it{};
		it.length = count;
		it.data = dense;
		it.index = count;
		return it;
	}

	template <typename T>
	SparseSet<T> SparseSet<T>::Create(Arena& arena, const size_t capacity)
	{
		SparseSet<T> set{};
		set.dense = arena.New<Node>(capacity);
		constexpr size_t s = SIZE_MAX;
		set.sparse = arena.New<size_t>(capacity, s);
		return set;
	}

	template <typename T>
	void SparseSet<T>::Destroy(const SparseSet& instance, Arena& arena)
	{
		arena.Free(instance.sparse);
		arena.Free(instance.dense);
	}
}
