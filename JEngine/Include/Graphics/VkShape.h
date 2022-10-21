#pragma once
#include "Vertex.h"
#include "Jlb/Array.h"

namespace je::vk
{
	void CreateQuadShape(Arena& arena, Array<Vertex>& outVertices, Array<Vertex::Index>& outIndices);
}
