#pragma once
#include "Arena.h"

namespace je
{
	// List that linearly links different, likely non adjacent points in memory.
	template <typename T>
	struct LinkedList final
	{
		struct Iterator final
		{
			LinkedList* linked = nullptr;

			T& operator*() const;
			T& operator->() const;

			const Iterator& operator++();
			Iterator operator++(int);

			friend bool operator==(const Iterator& a, const Iterator& b)
			{
				return a.linked == b.linked;
			}

			friend bool operator!= (const Iterator& a, const Iterator& b)
			{
				return !(a == b);
			}
		};

		T instance{};
		LinkedList<T>* next = nullptr;

		[[nodiscard]] T& operator[](size_t index);

		void Inverse();
		void Sort(bool (*comparer)(T& a, T& b));
		[[nodiscard]] size_t GetCount() const;
		
		[[nodiscard]] Iterator begin() const;
		[[nodiscard]] static Iterator end();

	private:
		[[nodiscard]] static LinkedList<T>* Inverse(LinkedList<T>* previous, LinkedList<T>* current);
	};

	template <typename T>
	T& LinkedList<T>::Iterator::operator*() const
	{
		return linked->instance;
	}

	template <typename T>
	T& LinkedList<T>::Iterator::operator->() const
	{
		return linked->instance;
	}

	template <typename T>
	const typename LinkedList<T>::Iterator& LinkedList<T>::Iterator::operator++()
	{
		linked = linked->next;
		return *this;
	}

	template <typename T>
	typename LinkedList<T>::Iterator LinkedList<T>::Iterator::operator++(int)
	{
		Iterator temp{};
		temp.linked = linked;
		linked = linked->next;
		return temp;
	}

	template <typename T>
	T& LinkedList<T>::operator[](const size_t index)
	{
		auto current = next;
		for (size_t i = 1; i < index; ++i)
			current = current->next;
		return current->instance;
	}

	template <typename T>
	void LinkedList<T>::Inverse()
	{
		next = Inverse(nullptr, next);
	}

	template <typename T>
	void LinkedList<T>::Sort(bool(* comparer)(T& a, T& b))
	{
		Chain* current = chain;
		while(current->next)
		{
			auto& currentInstance = current->instance;
			auto& nextInstance = current->next->instance;

			if (comparer(nextInstance, currentInstance))
			{
				T temp = currentInstance;
				currentInstance = nextInstance;
				nextInstance = temp;
			}

			current = current->next;
		}
	}

	template <typename T>
	size_t LinkedList<T>::GetCount() const
	{
		return 1 + next ? next->GetCount() : 0;
	}

	template <typename T>
	typename LinkedList<T>::Iterator LinkedList<T>::begin() const
	{
		Iterator iterator{};
		iterator.chain = chain;
		return iterator;
	}

	template <typename T>
	typename LinkedList<T>::Iterator LinkedList<T>::end()
	{
		return {};
	}

	template <typename T>
	LinkedList<T>* LinkedList<T>::Inverse(LinkedList<T>* previous, LinkedList<T>* current)
	{
		if (!current)
			return previous;
		auto begin = Inverse(current, current->next);
		current->next = previous;
		return begin;
	}

	template <typename T>
	[[nodiscard]] LinkedList<T> CreateLinkedList()
	{
		return {};
	}

	template <typename T>
	void DestroyLinkedList(LinkedList<T>* instance, Arena* arena)
	{
		auto chain = instance->chain;
		while (chain)
		{
			auto next = chain->next;
			arena->Free(chain);
			chain = next;
		}
	}

	template <typename T>
	T& LinkedListAdd(LinkedList<T>* instance, Arena* arena, const T& value)
	{
		++instance->count;
		auto* chain = arena->New<typename LinkedList<T>::Chain>();
		chain->instance = value;
		chain->next = chain;
		instance->chain = chain;
		return chain->instance;
	}

	template <typename T>
	T LinkedListPop(LinkedList<T>* instance, Arena* arena)
	{
		assert(instance->count > 0);
		--instance->count;
		T chainInstance = instance->chain->instance;
		auto next = instance->chain->next;
		arena->Delete(instance->chain);
		instance->chain = next;
		return instance;
	}
}
