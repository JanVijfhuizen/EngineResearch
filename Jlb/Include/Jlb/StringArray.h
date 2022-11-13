#pragma once
#include "Array.h"
#include <cstdint>

namespace je
{
	[[nodiscard]] Array<char> CreateStringArray(Arena* arena, const char* string, size_t length = SIZE_MAX);
	[[nodiscard]] Array<char> CreateStringArray(Arena* arena, size_t number);
}
