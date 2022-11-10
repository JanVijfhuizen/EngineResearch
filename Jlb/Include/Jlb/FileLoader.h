#pragma once
#include "Array.h"
#include "Jlb/StringView.h"

namespace je::file
{
	[[nodiscard]] Array<char> Load(Arena& arena, const StringView& path);
	[[nodiscard]] Array<size_t> Subdivide(Arena& arena, const char* data, size_t length, char d);
}
