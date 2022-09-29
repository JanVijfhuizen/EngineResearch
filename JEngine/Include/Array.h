#pragma once
#include "View.h"

namespace je
{
	class Arena;

	template <typename T>
	class Array
	{
	public:
		Array(Arena& arena, size_t length);
		Array(Array&& other) noexcept;
		~Array();

		[[nodiscard]] operator View<T>() const;

	private:
		Arena* _arena = nullptr;
		View<T> _view{};
	};

	template <typename T>
	Array<T>::Array(Arena& arena, const size_t length) : _arena(&arena), _view({arena.New<T>(length), length})
	{
	}

	template <typename T>
	Array<T>::Array(Array&& other) noexcept : _arena(other._arena), _view(other._view)  // NOLINT(performance-move-constructor-init)
	{
		other._arena = nullptr;
	}

	template <typename T>
	Array<T>::~Array()
	{
		if(_arena)
			_arena->Free(_view.GetData());
		_arena = nullptr;
	}

	template <typename T>
	Array<T>::operator View<T>() const
	{
		return _view;
	}
}
