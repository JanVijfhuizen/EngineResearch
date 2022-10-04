#pragma once
#include "Array.h"

namespace je
{
	class StringView;

	// Array of chars specialized in string behaviour.
	class String final : public Array<char>
	{
	public:
		/*
		String(Arena& arena, size_t number);
		String(Arena& arena, const StringView& view);
		*/
		[[nodiscard]] explicit operator StringView() const;
		[[nodiscard]] StringView GetStringView() const;
		[[nodiscard]] View<char> GetView() const override;
	};
}