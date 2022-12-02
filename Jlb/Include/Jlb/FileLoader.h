#pragma once
#include "Array.h"

namespace je::file
{
	// Loads text file and converts it into an array of chars.
	[[nodiscard]] Array<char> Load(Arena& arena, const char* path);
}
