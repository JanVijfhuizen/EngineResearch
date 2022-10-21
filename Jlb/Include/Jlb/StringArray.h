#pragma once
#include "Array.h"
#include "JMath.h"
#include "StringView.h"

namespace je
{
	struct StringView;

	// Array of chars specialized in string behaviour.
	template <typename T = char>
	class StringArray final : public Array<T>
	{
	public:
		StringArray() = default;
		StringArray(Arena& arena, size_t number);
		StringArray(Arena& arena, const StringView& view);

		[[nodiscard]] explicit operator StringView() const;
		[[nodiscard]] StringView GetStringView() const;
		[[nodiscard]] View<char> GetView() const override;
	};

	template <typename T>
	StringArray<T>::StringArray(Arena& arena, const size_t number) : Array<T>(arena, math::GetFractals<size_t>(number) + 1)
	{
		const auto data = Array<T>::GetData();
		const size_t length = Array<T>::GetLength();
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

	template <typename T>
	StringArray<T>::StringArray(Arena& arena, const StringView& view) : Array<T>(arena, view.GetLength() + 1)
	{
		const auto data = Array<T>::GetData();
		const size_t length = Array<T>::GetLength() - 1;
		for (size_t i = 0; i < length; ++i)
			data[i] = view[i];
		data[length] = '\0';
	}

	template <typename T>
	StringArray<T>::operator StringView() const
	{
		return GetStringView();
	}

	template <typename T>
	StringView StringArray<T>::GetStringView() const
	{
		return StringView{ Array<T>::GetData() };
	}

	template <typename T>
	View<char> StringArray<T>::GetView() const
	{
		return { Array<T>::GetData(), Array<T>::GetLength() - 1 };
	}
}
