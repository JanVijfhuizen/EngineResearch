#pragma once

namespace je
{
	// First-In-First-Out.
	template <typename T>
	struct Queue final
	{
		T* data = nullptr;
		size_t length = 0;
		size_t count = 0;
		size_t begin = 0;
		
		void Enqueue(const T& instance = {});
		[[nodiscard]] T& Peek();
		T Dequeue();
		void Clear();
	};

	template <typename T>
	void Queue<T>::Enqueue(const T& instance)
	{
		assert(count < length);
		size_t tail = begin + count++;
		tail %= length;
		data[tail] = instance;
	}

	template <typename T>
	T& Queue<T>::Peek()
	{
		return data[begin];
	}

	template <typename T>
	T Queue<T>::Dequeue()
	{
		assert(count > 0);
		const T instance = data[begin++];
		begin %= length;
		return instance;
	}

	template <typename T>
	void Queue<T>::Clear()
	{
		begin = 0;
		count = 0;
	}

	template <typename T>
	[[nodiscard]] Queue<T> CreateQueue(Arena* arena, const size_t length)
	{
		Queue<T> instance{};
		instance.data = arena->New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyQueue(Queue<T>* instance, Arena* arena)
	{
		arena->Free(instance->data);
	}
}
