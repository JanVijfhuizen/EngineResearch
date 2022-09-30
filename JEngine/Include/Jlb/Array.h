#pragma once
#include "View.h"
#include "Arena.h"

namespace je
{
	template <typename T>
	class Array
	{
	public:
		Array(Arena& arena, size_t length);
		Array(Array&& other) noexcept;
		virtual ~Array();

		virtual View<T> GetView() const;

		[[nodiscard]] operator View<T>() const;
		[[nodiscard]] operator T*() const;
		[[nodiscard]] size_t GetLength() const;

		[[nodiscard]] T* GetData() const;

	private:
		Arena* _arena = nullptr;
		T* _data = nullptr;
		size_t _length = SIZE_MAX;
	};

	template <typename T>
	Array<T>::Array(Arena& arena, const size_t length) : _arena(&arena), _data(arena.New<T>(length)), _length(length)
	{
	}

	template <typename T>
	Array<T>::Array(Array&& other) noexcept : _arena(other._arena), _data(other._data), _length(other._length)
	{
		other._arena = nullptr;
	}

	template <typename T>
	Array<T>::~Array()
	{
		if(_arena)
			_arena->Free(_data);
		_arena = nullptr;
	}

	template <typename T>
	View<T> Array<T>::GetView() const
	{
		return { _data, _length };
	}

	template <typename T>
	Array<T>::operator View<T>() const
	{
		return GetView();
	}

	template <typename T>
	Array<T>::operator T*() const
	{
		return _data;
	}

	template <typename T>
	size_t Array<T>::GetLength() const
	{
		return _length;
	}

	template <typename T>
	T* Array<T>::GetData() const
	{
		return _data;
	}
}
