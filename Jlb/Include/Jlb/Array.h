#pragma once
#include "Arena.h"
#include "Iterator.h"

namespace je
{
	template <typename T>
	struct Array final
	{
		T* data = nullptr;
		size_t length = 0;

		[[nodiscard]] T& operator [](size_t index);
		[[nodiscard]] Iterator<T> begin() const;
		[[nodiscard]] Iterator<T> end() const;
	};

	template <typename T>
	T& Array<T>::operator[](const size_t index)
	{
		assert(index < length);
		return data[index];
	}

	template <typename T>
	Iterator<T> Array<T>::begin() const
	{
		Iterator<T> it{};
		it.length = length;
		it.data = data;
		return it;
	}

	template <typename T>
	Iterator<T> Array<T>::end() const
	{
		Iterator<T> it{};
		it.length = length;
		it.index = length;
		it.data = data;
		return it;
	}

	template <typename T>
	[[nodiscard]] Array<T> CreateArray(Arena& arena, const size_t length)
	{
		Array<T> instance{};
		instance.data = arena.New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyArray(Array<T>& instance, Arena& arena)
	{
		arena.Free(instance.data);
	}
}
