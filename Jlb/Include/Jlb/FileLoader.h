#pragma once
#include "Array.h"

namespace je::file
{
	[[nodiscard]] Array<char> Load(Arena& arena, const char* path);
}
