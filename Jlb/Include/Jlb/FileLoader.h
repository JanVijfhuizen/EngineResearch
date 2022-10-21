#pragma once
#include "Array.h"
#include "Jlb/StringView.h"

namespace je::file
{
	[[nodiscard]] Array<char> Load(Arena& arena, StringView path);
}
