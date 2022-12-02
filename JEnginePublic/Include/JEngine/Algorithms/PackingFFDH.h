#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace packing
	{
		// Packing algorithm for rectangular shapes. Returns the positions of said shapes, and the size of the area in which they are packed.
		[[nodiscard]] Array<glm::ivec2> Pack(Arena& arena, Arena& tempArena, const Array<glm::ivec2>& shapes, glm::ivec2& outArea);
	}
}
