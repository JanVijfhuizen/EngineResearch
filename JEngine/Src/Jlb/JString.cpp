#include "pch.h"
#include "Jlb/JString.h"

#include "Jlb/JMath.h"
#include "Jlb/StringView.h"

namespace je
{
	String::String(Arena& arena, const size_t number) : Array<char>(arena, math::GetFractals<size_t>(number) + 1)
	{
		const auto data = GetData();
		const size_t length = GetLength();
		data[length - 1] = '\0';

		size_t d = 10;
		for (size_t j = 0; j < length; ++j)
		{
			auto& c = data[length - 1 - j];
			const size_t mod = number % d;
			const size_t n = mod * 10 / d;
			c = '0' + n;
			d *= 10;
		}
	}

	String::String(Arena& arena, const StringView& view) noexcept : Array<char>(arena, view.GetLength() + 1)
	{
		const auto data = GetData();
		const size_t length = GetLength() - 1;
		for (size_t i = 0; i < length; ++i)
			data[i] = view[i];
		data[length] = '\0';
	}

	String::operator StringView() const
	{
		return GetStringView();
	}

	StringView String::GetStringView() const
	{
		return StringView{GetData() };
	}

	View<char> String::GetView() const
	{
		return { GetData(), GetLength() - 1 };
	}
}
