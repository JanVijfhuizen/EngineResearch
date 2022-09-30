#pragma once
#include "Array.h"

namespace je
{
	class StringView;

	class String final : public Array<char>
	{
	public:
		String(Arena& arena, size_t number);
		String(Arena& arena, const StringView& view);
		
		[[nodiscard]] operator StringView() const;
		[[nodiscard]] StringView GetStringView() const;
		[[nodiscard]] View<char> GetView() const override;
	};
}

