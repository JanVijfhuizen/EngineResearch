#pragma once

namespace je
{
	// Used in data structures like maps, heaps etc.
	template <typename T>
	struct KeyPair final
	{
		T value{};
		size_t key = SIZE_MAX;
	};
}