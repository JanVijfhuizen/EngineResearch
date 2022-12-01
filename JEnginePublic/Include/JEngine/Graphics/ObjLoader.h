#pragma once
#include "Vertex.h"
#include "Jlb/Array.h"

namespace je::obj
{
	void Load(Arena& arena, const char* path, Array<vk::Vertex>& outVertices, Array<vk::Vertex::Index>& outIndices, float scale = 1);
}
