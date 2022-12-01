#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace packing
	{
		[[nodiscard]] Array<glm::ivec2> Pack(Arena& arena, Arena& tempArena, const Array<glm::ivec2>& shapes, glm::ivec2& outArea);
	}
}
