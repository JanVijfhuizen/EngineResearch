#pragma once
#include "Iterator.h"

namespace je
{
	// Allows for overloading different data structures as a simplified and generalized view.
	// Also works for single objects.
	template <typename T>
	struct View
	{
		// ReSharper disable once CppNonExplicitConvertingConstructor
		View(T& instance);
		View(void* data, size_t length);

		[[nodiscard]] T& operator [](size_t index);
		[[nodiscard]] size_t GetLength() const;

		virtual Iterator<T> begin() const;
		virtual Iterator<T> end() const;

		[[nodiscard]] T* GetData() const;

	private:
		T* _data = nullptr;
		size_t _length = SIZE_MAX;
	};

	template <typename T>
	View<T>::View(T& instance) : _data(&instance), _length(1)
	{

	}

	template <typename T>
	View<T>::View(void* data, const size_t length) : _data(static_cast<T*>(data)), _length(length)
	{

	}

	template <typename T>
	T& View<T>::operator[](size_t index)
	{
		assert(_length > index);
		return _data[index];
	}

	template <typename T>
	size_t View<T>::GetLength() const
	{
		return _length;
	}

	template <typename T>
	Iterator<T> View<T>::begin() const
	{
		Iterator<T> it{};
		it.length = _length;
		it.data = _data;
		return it;
	}

	template <typename T>
	Iterator<T> View<T>::end() const
	{
		Iterator<T> it{};
		it.length = _length;
		it.index = _length;
		it.data = _data;
		return it;
	}

	template <typename T>
	T* View<T>::GetData() const
	{
		return _data;
	}
}
