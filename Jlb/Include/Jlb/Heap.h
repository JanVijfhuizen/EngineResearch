#pragma once
#include "Array.h"
#include "KeyPair.h"
#include "Swap.h"

namespace je
{
	// Automatically sorts values during insertion.
	template <typename T>
	struct Heap final
	{
		KeyPair<T>* data = nullptr;
		size_t length = 0;
		size_t count = 0;
		
		void Insert(const T& value, size_t key);
		[[nodiscard]] T Peek();
		T Pop();
		void Clear();

	private:
		void HeapifyBottomToTop(size_t index);
		void HeapifyTopToBottom(size_t index);
	};

	template <typename T>
	void Heap<T>::Insert(const T& value, const size_t key)
	{
		assert(count < length);
		count++;

		auto& keyPair = data[count];
		keyPair.key = key;
		keyPair.value = value;
		HeapifyBottomToTop(count);
	}

	template <typename T>
	T Heap<T>::Peek()
	{
		assert(count > 0);
		const T value = data[1].value;
		return value;
	}

	template <typename T>
	T Heap<T>::Pop()
	{
		assert(count > 0);
		
		const T value = data[1].value;
		data[1] = data[count--];

		HeapifyTopToBottom(1);
		return value;
	}

	template <typename T>
	void Heap<T>::Clear()
	{
		count = 0;
	}

	template <typename T>
	void Heap<T>::HeapifyBottomToTop(const size_t index)
	{
		// Tree root found.
		if (index <= 1)
			return;
		
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
		if (count < left)
			return;

		const auto data = Array<KeyPair<T>>::GetData();
		// Is the left node smaller than index.
		const bool lDiff = data[index].key > data[left].key;
		// Is the right node smaller than index.
		const bool rDiff = count > left ? data[index].key > data[right].key : false;
		// Is left smaller than right.
		const bool dir = rDiff ? data[left].key > data[right].key : false;

		if (lDiff || rDiff)
		{
			const size_t newIndex = left + dir;
			Swap(data, newIndex, index);
			HeapifyTopToBottom(newIndex);
		}
	}

	template <typename T>
	[[nodiscard]] Heap<T> CreateHeap(Arena* arena, const size_t length)
	{
		Heap<T> instance{};
		instance.data = arena->New<T>(length + 1);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void CreateHeap(Heap<T>* instance, Arena* arena)
	{
		arena->Free(instance->data);
	}
}
