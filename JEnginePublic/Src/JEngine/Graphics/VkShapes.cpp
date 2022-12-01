#include "JEngine/pch.h"
#include "JEngine/Graphics/VkShapes.h"

namespace je::vk
{
	void CreateQuadShape(Arena& arena, Array<Vertex>& outVertices, Array<Vertex::Index>& outIndices, const float scale)
	{
		outVertices = CreateArray<Vertex>(arena, 4);
		outIndices = CreateArray<Vertex::Index>(arena, 6);

		outVertices[0].position = { -1, -1, 0 };
		outVertices[1].position = { -1, 1, 0 };
		outVertices[2].position = { 1, 1, 0 };
		outVertices[3].position = { 1, -1, 0 };

		outVertices[0].textureCoordinates = { 0, 0 };
		outVertices[1].textureCoordinates = { 0, 1 };
		outVertices[2].textureCoordinates = { 1, 1 };
		outVertices[3].textureCoordinates = { 1, 0 };

		outIndices[0] = 0;
		outIndices[1] = 1;
		outIndices[2] = 2;
		outIndices[3] = 0;
		outIndices[4] = 2;
		outIndices[5] = 3;

		for (auto& vertex : outVertices)
			vertex.position *= scale;
	}
}
