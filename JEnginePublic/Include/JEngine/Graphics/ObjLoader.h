#pragma once
#include "Vertex.h"
#include "Jlb/Array.h"

namespace je::obj
{
	// Loads an 3d obj file and converts it to indices and vertices, which can be used to create a mesh.
	void Load(Arena& arena, const char* path, Array<vk::Vertex>& outVertices, Array<vk::Vertex::Index>& outIndices, float scale = 1);
}
