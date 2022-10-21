#pragma once
#include "Array.h"

namespace je
{
	// First-In-Last-Out.
	template <typename T>
	class Stack final : public Array<T>
	{
	public:
		Stack(Arena& arena, size_t length);
		Stack(Stack<T>&& other) noexcept;
		[[nodiscard]] Stack<T>& operator=(Stack<T>&& other) noexcept;

		void Push(const T& instance = {});
		[[nodiscard]] T& Peek();
		T Pop();
		[[nodiscard]] size_t GetCount() const;
		[[nodiscard]] View<T> GetView() const override;

	private:
		size_t _count = 0;
	};

	template <typename T>
	Stack<T>::Stack(Arena& arena, const size_t length) : Array<T>(arena, length)
	{
	}

	template <typename T>
	Stack<T>::Stack(Stack<T>&& other) noexcept : Array<T>(Move(other)), _count(other._count)
	{
		
	}

	template <typename T>
	Stack<T>& Stack<T>::operator=(Stack<T>&& other) noexcept
	{
		_count = other._count;
		Array<T>::operator=(Move(other));
		return *this;
	}

	template <typename T>
	void Stack<T>::Push(const T& instance)
	{
		assert(_count < Array<T>::GetLength());
		Array<T>::GetData()[_count++] = instance;
	}

	template <typename T>
	T& Stack<T>::Peek()
	{
		return Array<T>::GetData()[_count - 1];
	}

	template <typename T>
	T Stack<T>::Pop()
	{
		assert(_count > 0);
		return Array<T>::GetData()[--_count];
	}

	template <typename T>
	size_t Stack<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	View<T> Stack<T>::GetView() const
	{
		return { Array<T>::GetData(), _count };
	}
}

