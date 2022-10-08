#pragma once
#include "Array.h"
#include "KeyPair.h"
#include "Utils.h"

namespace je
{
	// Automatically sorts values during insertion.
	template <typename T>
	class Heap final : public Array<KeyPair<T>>
	{
	public:
		Heap(Arena& arena, size_t length);
		Heap(Heap<KeyPair<T>>&& other);

		void Insert(const T& value, size_t key);
		
		[[nodiscard]] T Peek();
		T Pop();

		void Clear();
		[[nodiscard]] size_t GetCount() const;

	private:
		size_t _count = 0;
		
		void HeapifyBottomToTop(size_t index);
		void HeapifyTopToBottom(size_t index);
	};

	template <typename T>
	Heap<T>::Heap(Arena& arena, const size_t length) : Array<KeyPair<T>>(arena, length)
	{
	}

	template <typename T>
	Heap<T>::Heap(Heap<KeyPair<T>>&& other) : Array<KeyPair<T>>(Move(other)), _count(other._count)
	{
	}

	template <typename T>
	void Heap<T>::Insert(const T& value, const size_t key)
	{
		assert(_count < Array<KeyPair<T>>::GetLength());
		_count++;
		const auto data = Array<KeyPair<T>>::GetData();

		auto& keyPair = data[_count];
		keyPair.key = key;
		keyPair.value = value;
		HeapifyBottomToTop(_count);
	}

	template <typename T>
	T Heap<T>::Peek()
	{
		assert(_count > 0);
		const auto data = Array<KeyPair<T>>::GetData();
		const T value = data[1].value;
		return value;
	}

	template <typename T>
	T Heap<T>::Pop()
	{
		assert(_count > 0);

		const auto data = Array<KeyPair<T>>::GetData();
		const T value = data[1].value;
		data[1] = data[_count--];

		HeapifyTopToBottom(1);
		return value;
	}

	template <typename T>
	void Heap<T>::Clear()
	{
		_count = 0;
	}

	template <typename T>
	size_t Heap<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	void Heap<T>::HeapifyBottomToTop(const size_t index)
	{
		// Tree root found.
		if (index <= 1)
			return;

		const auto data = Array<KeyPair<T>>::GetData();
		size_t parentIndex = index / 2;

		// If current is smaller than the parent, swap and continue.
		if (data[index].key < data[parentIndex].key)
		{
			Swap(data, index, parentIndex);
			HeapifyBottomToTop(parentIndex);
		}
	}

	template <typename T>
	void Heap<T>::HeapifyTopToBottom(const size_t index)
	{
		const size_t left = index * 2;
		const size_t right = index * 2 + 1;

		// If no more nodes remain on the left side.
		if (_count < left)
			return;

		const auto data = Array<KeyPair<T>>::GetData();
		// Is the left node smaller than index.
		const bool lDiff = data[index].key > data[left].key;
		// Is the right node smaller than index.
		const bool rDiff = _count > left ? data[index].key > data[right].key : false;
		// Is left smaller than right.
		const bool dir = rDiff ? data[left].key > data[right].key : false;

		if (lDiff || rDiff)
		{
			const size_t newIndex = left + dir;
			Swap(data, newIndex, index);
			HeapifyTopToBottom(newIndex);
		}
	}
}
