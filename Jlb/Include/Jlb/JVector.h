#pragma once
#include "Array.h"

namespace je
{
	// Allows for resizing, adding and removing values.
	template <typename T>
	class Vector : public Array<T>
	{
	public:
		Vector(Arena& arena, size_t length);
		Vector(Vector<T>&& other) noexcept;

		T& Add(const T& instance = {});
		void Remove(size_t index);
		void Clear();

		[[nodiscard]] size_t GetCount() const;
		[[nodiscard]] View<T> GetView() const override;

	private:
		size_t _count = 0;
	};

	template <typename T>
	Vector<T>::Vector(Arena& arena, const size_t length): Array<T>(arena, length)
	{
	}

	template <typename T>
	Vector<T>::Vector(Vector<T>&& other) noexcept: Array<T>(other)
	{
		_count = other._count;
	}

	template <typename T>
	T& Vector<T>::Add(const T& instance)
	{
		assert(GetCount() < Array<T>::GetLength());
		return Array<T>::_ptr[_count++] = instance;
	}

	template <typename T>
	void Vector<T>::Remove(const size_t index)
	{
		Array<T>::_ptr[index] = Array<T>::_ptr[--_count];
	}

	template <typename T>
	void Vector<T>::Clear()
	{
		_count = 0;
	}

	template <typename T>
	size_t Vector<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	View<T> Vector<T>::GetView() const
	{
		return { Array<T>::_ptr, _count };
	}
}
