#pragma once
#include "Arena.h"

namespace je
{
	template <typename T>
	class LinkedList final
	{
	private:
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

		explicit LinkedList(Arena& arena);
		LinkedList(LinkedList&& other) noexcept;
		~LinkedList();

		T& Add(const T& instance = {});

		[[nodiscard]] size_t GetCount() const;
		Iterator begin() const;
		static Iterator end();

	private:
		Arena* _arena = nullptr;
		Chain* _chain = nullptr;
		size_t _count = 0;
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
	LinkedList<T>::LinkedList(LinkedList&& other) noexcept : _arena(other._arena), _chain(other._chain)
	{
		other._arena = nullptr;
		other._chain = nullptr;
	}

	template <typename T>
	LinkedList<T>::~LinkedList()
	{
		Chain* chain = _chain;
		while(chain)
		{
			_arena->Free(chain);
			chain = chain->next;
		}
	}

	template <typename T>
	T& LinkedList<T>::Add(const T& instance)
	{
		++_count;
		Chain* chain = _arena->New<Chain>();
		chain->instance = instance;
		chain->next = _chain;
		_chain = chain;
		return chain->instance;
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
	typename LinkedList<T>::Iterator LinkedList<T>::end()
	{
		return {};
	}
}
