#pragma once
#include "Jlb/Array.h"
#include "Jlb/View.h"

namespace je
{
	class Arena;

	namespace packing
	{
		[[nodiscard]] Array<glm::ivec2> Pack(Arena& arena, Arena& tempArena, const View<glm::ivec2>& shapes, glm::ivec2 area);
	}
}
