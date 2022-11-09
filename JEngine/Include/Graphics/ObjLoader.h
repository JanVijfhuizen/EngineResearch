#pragma once
#include "Vertex.h"
#include "Jlb/Array.h"

namespace je
{
	struct StringView;

	namespace obj
	{
		[[nodiscard]] Arena::Scope Load(Arena& tempArena, const StringView& path, Array<vk::Vertex>& outVertices, Array<vk::Vertex::Index>& outIndices);
	}
	
}
