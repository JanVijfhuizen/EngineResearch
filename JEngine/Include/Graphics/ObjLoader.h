#pragma once
#include "Vertex.h"
#include "Jlb/Array.h"

namespace je
{
	struct StringView;

	namespace obj
	{
		void Load(Arena& arena, const StringView& path, const Arena::Scope& scope, Array<vk::Vertex>& outVertices, Array<vk::Vertex::Index>& outIndices, float scale = 1);
	}
	
}
