#pragma once

namespace je
{
	// Standard linear allocator.
	template <typename T>
	struct Iterator final
	{
		T* data = nullptr;
		size_t length = 0;
		size_t index = 0;

		T& operator*() const;
		T& operator->() const;

		const Iterator& operator++();
		Iterator operator++(int);

		friend bool operator==(const Iterator& a, const Iterator& b)
		{
			return a.index == b.index;
		}

		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return !(a == b);
		}
	};

	template <typename T>
	T& Iterator<T>::operator*() const
	{
		assert(data);
		assert(index <= length);
		return data[index];
	}

	template <typename T>
	T& Iterator<T>::operator->() const
	{
		assert(data);
		assert(index <= length);
		return data[index];
	}

	template <typename T>
	const Iterator<T>& Iterator<T>::operator++()
	{
		++index;
		return *this;
	}

	template <typename T>
	Iterator<T> Iterator<T>::operator++(int)
	{
		Iterator temp(data, length, index);
		++index;
		return temp;
	}
}