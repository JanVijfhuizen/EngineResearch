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

	template <typename T>
	[[nodiscard]] constexpr T GetFractals(const size_t number)
	{
		size_t d = 1;
		size_t length = 0;
		while (d <= number)
		{
			d *= 10;
			++length;
		}
		return length;
	}

	template <typename T>
	[[nodiscard]] constexpr T Clamp(const T& t, const T& min, const T& max)
	{
		assert(min <= max);
		return Max(min, Min(t, max));
	}
}
