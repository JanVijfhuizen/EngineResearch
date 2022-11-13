#pragma once
#include "Iterator.h"

namespace je
{
	template <typename T>
	struct View
	{
		T* data = nullptr;
		size_t length = 0;

		virtual T& operator[](size_t index);
		[[nodiscard]] virtual Iterator<T> begin() const;
		[[nodiscard]] virtual Iterator<T> end() const;
		[[nodiscard]] operator bool() const;
		[[nodiscard]] operator T* () const;
	};

	template <typename T>
	T& View<T>::operator[](size_t index)
	{
		return data[index];
	}

	template <typename T>
	Iterator<T> View<T>::begin() const
	{
		Iterator<T> it{};
		it.length = length;
		it.data = data;
		return it;
	}

	template <typename T>
	Iterator<T> View<T>::end() const
	{
		Iterator<T> it{};
		it.length = length;
		it.index = length;
		it.data = data;
		return it;
	}

	template <typename T>
	View<T>::operator bool() const
	{
		return data;
	}

	template <typename T>
	View<T>::operator T*() const
	{
		return data;
	}
}
