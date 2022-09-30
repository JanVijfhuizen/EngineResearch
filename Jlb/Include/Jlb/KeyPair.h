#pragma once

namespace je
{
	template <typename T>
	struct KeyPair final
	{
		T value{};
		size_t key = SIZE_MAX;
	};
}