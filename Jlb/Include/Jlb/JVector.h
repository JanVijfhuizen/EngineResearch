#pragma once
#include "Array.h"

namespace je
{
	template <typename T>
	struct Vector final
	{
		T* data = nullptr;
		size_t length = 0;
		size_t count = 0;

		T& Add(const T& instance = {});
		void Remove(size_t index);
		void Clear();
	};

	template <typename T>
	T& Vector<T>::Add(const T& instance)
	{
		assert(count < length);
		return data[count++] = instance;
	}

	template <typename T>
	void Vector<T>::Remove(const size_t index)
	{
		data[index] = data[--count];
	}

	template <typename T>
	void Vector<T>::Clear()
	{
		count = 0;
	}

	template <typename T>
	[[nodiscard]] Vector<T> CreateVector(Arena* arena, const size_t length)
	{
		Vector<T> instance{};
		instance.data = arena->New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyVector(Vector<T>* instance, Arena* arena)
	{
		arena->Free(instance->data);
	}
}
