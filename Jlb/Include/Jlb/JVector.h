#pragma once

namespace je
{
	// Resizeable array.
	template <typename T>
	struct Vector final
	{
		T* data = nullptr;
		size_t length = 0;
		size_t count = 0;

		T& Add(const T& instance = {});
		void Remove(size_t index);
		void Clear();

		[[nodiscard]] T& operator [](size_t index) const;
		[[nodiscard]] Iterator<T> begin() const;
		[[nodiscard]] Iterator<T> end() const;
	};

	template <typename T>
	T& Vector<T>::Add(const T& instance)
	{
		assert(count < length);
		return data[count++] = instance;
	}

	template <typename T>
	void Vector<T>::Remove(const size_t index)
	{
		data[index] = data[--count];
	}

	template <typename T>
	void Vector<T>::Clear()
	{
		count = 0;
	}

	template <typename T>
	T& Vector<T>::operator[](const size_t index) const
	{
		assert(index < count);
		return data[index];
	}

	template <typename T>
	Iterator<T> Vector<T>::begin() const
	{
		Iterator<T> it{};
		it.length = count;
		it.index = 0;
		it.data = data;
		return it;
	}

	template <typename T>
	Iterator<T> Vector<T>::end() const
	{
		Iterator<T> it{};
		it.length = count;
		it.index = count;
		it.data = data;
		return it;
	}

	template <typename T>
	[[nodiscard]] Vector<T> CreateVector(Arena& arena, const size_t length)
	{
		Vector<T> instance{};
		instance.data = arena.New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyVector(Vector<T>& instance, Arena& arena)
	{
		arena.Free(instance.data);
	}
}
