#pragma once
#include "Arena.h"
#include "View.h"

namespace je
{
	template <typename T>
	struct Array final : View<T>
	{
		
	};

	template <typename T>
	[[nodiscard]] Array<T> CreateArray(Arena& arena, const size_t length)
	{
		Array<T> instance{};
		instance.data = arena.New<T>(length);
		instance.length = length;
		return instance;
	}

	template <typename T>
	void DestroyArray(Array<T>& instance, Arena& arena)
	{
		arena.Free(instance.data);
	}
}
