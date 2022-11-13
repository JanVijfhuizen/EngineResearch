#pragma once
#include "Arena.h"

namespace je
{
	// First-In-Last-Out.
	template <typename T>
	struct Stack final
	{
		T* data = nullptr;
		size_t length = 0;
		size_t count = 0;

		void Push(const T& instance = {});
		[[nodiscard]] T& Peek();
		T Pop();
	};

	template <typename T>
	void Stack<T>::Push(const T& instance)
	{
		assert(count < length);
		data[count++] = instance;
	}

	template <typename T>
	T& Stack<T>::Peek()
	{
		return data[count - 1];
	}

	template <typename T>
	T Stack<T>::Pop()
	{
		assert(count > 0);
		return data[--count];
	}

	template <typename T>
	[[nodiscard]] Stack<T> CreateStack(Arena& arena, const size_t length)
	{
		Stack<T> instance{};
		instance.data = arena.New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyStack(Stack<T>& instance, Arena& arena)
	{
		arena.Free(instance.data);
	}
}

