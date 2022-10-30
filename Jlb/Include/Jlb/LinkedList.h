#pragma once
#include "Arena.h"

namespace je
{
	// List that linearly links different, likely non adjacent points in memory.
	template <typename T>
	class LinkedList final
	{
		struct Chain final
		{
			T instance{};
			Chain* next = nullptr;
		};

	public:
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

		LinkedList() = default;
		explicit LinkedList(Arena& arena);
		LinkedList(LinkedList<T>&& other) noexcept;
		LinkedList<T>& operator=(LinkedList<T>&& other) noexcept;
		~LinkedList();

		T& Add(const T& instance = {});
		[[nodiscard]] T& operator[](size_t index);

		void Inverse();

		[[nodiscard]] size_t GetCount() const;
		[[nodiscard]] Iterator begin() const;
		[[nodiscard]] Iterator end() const;

	private:
		Arena* _arena = nullptr;
		Chain* _chain = nullptr;
		size_t _count = 0;

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
	LinkedList<T>::LinkedList(Arena& arena) : _arena(&arena)
	{
	}

	template <typename T>
	LinkedList<T>::LinkedList(LinkedList<T>&& other) noexcept : _arena(other._arena), _chain(other._chain)
	{
		other._arena = nullptr;
		other._chain = nullptr;
	}

	template <typename T>
	LinkedList<T>& LinkedList<T>::operator=(LinkedList<T>&& other) noexcept
	{
		_arena = other._arena;
		_chain = other._chain;
		other._arena = nullptr;
		other._chain = nullptr;
		return *this;
	}

	template <typename T>
	LinkedList<T>::~LinkedList()
	{
		if (!_arena)
			return;

		Chain* chain = _chain;
		while(chain)
		{
			Chain* next = chain->next;
			_arena->Free(chain);
			chain = next;
		}
	}

	template <typename T>
	T& LinkedList<T>::Add(const T& instance)
	{
		++_count;
		auto* chain = _arena->New<Chain>();
		chain->instance = instance;
		chain->next = _chain;
		_chain = chain;
		return chain->instance;
	}

	template <typename T>
	T& LinkedList<T>::operator[](const size_t index)
	{
		Chain* chain = _chain;
		for (size_t i = 1; i < index; ++i)
		{
			chain = chain->next;
		}
		return chain->instance;
	}

	template <typename T>
	void LinkedList<T>::Inverse()
	{
		_chain = Inverse(nullptr, _chain);
	}

	template <typename T>
	size_t LinkedList<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	typename LinkedList<T>::Iterator LinkedList<T>::begin() const
	{
		Iterator iterator{};
		iterator.chain = _chain;
		return iterator;
	}

	template <typename T>
	typename LinkedList<T>::Iterator LinkedList<T>::end() const
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
}
