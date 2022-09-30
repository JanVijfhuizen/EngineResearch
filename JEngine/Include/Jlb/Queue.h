#pragma once
#include "Array.h"

namespace je
{
	template <typename T>
	class Queue final : public Array<T>
	{
	public:
		Queue(Arena& arena, size_t length);
		Queue(Array<T>&& other) noexcept;

		void Enqueue(const T& instance = {});
		[[nodiscard]] T& Peek();
		T Dequeue();

		[[nodiscard]] size_t GetCount() const;

	private:
		size_t _count = 0;
		size_t _begin = 0;
	};

	template <typename T>
	Queue<T>::Queue(Arena& arena, size_t length) : Array<T>(arena, length)
	{
	}

	template <typename T>
	Queue<T>::Queue(Array<T>&& other) noexcept : Array<T>(other), _count(other._count), _begin(other._begin)
	{

	}

	template <typename T>
	void Queue<T>::Enqueue(const T& instance)
	{
		const size_t length = Array<T>::GetLength();
		assert(_count < length);
		size_t tail = _begin + _count++;
		tail %= length;
		Array<T>::GetData()[tail] = instance;
	}

	template <typename T>
	T& Queue<T>::Peek()
	{
		return Array<T>::GetData()[_begin];
	}

	template <typename T>
	T Queue<T>::Dequeue()
	{
		assert(_count > 0);
		const T instance = Array<T>::GetData()[_begin++];
		_begin %= Array<T>::GetLength();
		return instance;
	}

	template <typename T>
	size_t Queue<T>::GetCount() const
	{
		return _count;
	}
}
