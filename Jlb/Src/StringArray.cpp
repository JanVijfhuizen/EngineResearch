#include "Jlb/pch.h"
#include "Jlb/StringArray.h"
#include <cstring>
#include "Jlb/JMath.h"

namespace je
{
	Array<char> CreateStringArray(Arena& arena, const char* string, size_t length)
	{
		/*
		length = length == SIZE_MAX ? strlen(string) + 1 : length;
		const auto instance = CreateArray<char>(arena, length);

		const auto instance = CreateArray<char>(arena, length);
		for (size_t i = 0; i < length - 1; ++i)
			instance.data[i] = string[i];
		instance.data[length - 1] = '\0';
		return instance;
		*/
		return {};
	}

	Array<char> CreateStringArray(Arena& arena, const size_t number)
	{
		/*
		const auto instance = CreateArray<char>(arena, math::GetFractals<size_t>(number) + 1);
		instance.data[instance.length - 1] = '\0';

		size_t d = 10;
		for (size_t j = 0; j < instance.length; ++j)
		{
			auto& c = instance.data[instance.length - 1 - j];
			const size_t mod = number % d;
			const size_t n = mod * 10 / d;
			c = '0' + static_cast<char>(n);
			d *= 10;
		}
		return instance;
		*/
		return {};
	}
}
