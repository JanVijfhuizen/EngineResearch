#pragma once
#include "Arena.h"

namespace je
{
	// List that linearly links different, likely non adjacent points in memory.
	template <typename T>
	struct LinkedList final
	{
		struct Chain final
		{
			T instance{};
			Chain* next = nullptr;
		};

		struct Iterator final
		{
			Chain* chain = nullptr;

			T& operator*() const;
			T& operator->() const;

			const Iterator& operator++();
			Iterator operator++(int);

			friend bool operator==(const Iterator& a, const Iterator& b)
			{
				return a.chain == b.chain;
			}

			friend bool operator!= (const Iterator& a, const Iterator& b)
			{
				return !(a == b);
			}
		};
		
		Chain* chain = nullptr;
		size_t count = 0;

		[[nodiscard]] T& operator[](size_t index);

		void Inverse();
		void Sort(bool (*comparer)(T& a, T& b));
		
		[[nodiscard]] Iterator begin() const;
		[[nodiscard]] static Iterator end();

	private:
		[[nodiscard]] Chain* Inverse(Chain* previous, Chain* current);
	};

	template <typename T>
	T& LinkedList<T>::Iterator::operator*() const
	{
		return chain->instance;
	}

	template <typename T>
	T& LinkedList<T>::Iterator::operator->() const
	{
		return chain->instance;
	}

	template <typename T>
	const typename LinkedList<T>::Iterator& LinkedList<T>::Iterator::operator++()
	{
		chain = chain->next;
		return *this;
	}

	template <typename T>
	typename LinkedList<T>::Iterator LinkedList<T>::Iterator::operator++(int)
	{
		Iterator temp{};
		temp.chain = chain;
		chain = chain->next;
		return temp;
	}

	template <typename T>
	T& LinkedList<T>::operator[](const size_t index)
	{
		Chain* current = chain;
		for (size_t i = 1; i < index; ++i)
			current = current->next;
		return current->instance;
	}

	template <typename T>
	void LinkedList<T>::Inverse()
	{
		chain = Inverse(nullptr, chain);
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
	typename LinkedList<T>::Chain* LinkedList<T>::Inverse(Chain* previous, Chain* current)
	{
		if (!current)
			return previous;
		Chain* begin = Inverse(current, current->next);
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
	T& LinkedListAdd(LinkedList<T>* instance, Arena* arena)
	{
		++instance->count;
		auto* chain = arena->New<typename LinkedList<T>::Chain>();
		chain->instance = instance;
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
