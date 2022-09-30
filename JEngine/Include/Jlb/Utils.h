#pragma once

namespace je
{
	template <typename T>
	void Swap(T* arr, const size_t a, const size_t b)
	{
		const T aInstance = arr[a];
		arr[a] = arr[b];
		arr[b] = aInstance;
	}
}
