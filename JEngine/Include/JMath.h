#pragma once

namespace je::math
{
	template <typename T>
	[[nodiscard]] constexpr T Min(const T& a, const T& b)
	{
		return a > b ? b : a;
	}

	template <typename T>
	[[nodiscard]] constexpr T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
}
