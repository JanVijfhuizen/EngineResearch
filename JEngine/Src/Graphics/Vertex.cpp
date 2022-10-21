#include "pch.h"
#include "Graphics/Vertex.h"

namespace je::vk
{
	Array<VkVertexInputBindingDescription> Vertex::GetBindingDescriptions(Arena& arena)
	{
		Array<VkVertexInputBindingDescription> ret{arena, 1};
		auto& vertexData = ret[0];
		vertexData.binding = 0;
		vertexData.stride = sizeof(Vertex);
		vertexData.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return Move(ret);
	}

	Array<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions(Arena& arena)
	{
		Array<VkVertexInputAttributeDescription> ret{arena, 2};

		auto& position = ret[0];
		position.binding = 0;
		position.location = 0;
		position.format = VK_FORMAT_R32G32_SFLOAT;
		position.offset = offsetof(Vertex, position);

		auto& texCoords = ret[1];
		texCoords.binding = 0;
		texCoords.location = 1;
		texCoords.format = VK_FORMAT_R32G32_SFLOAT;
		texCoords.offset = offsetof(Vertex, texCoords);

		return Move(ret);
	}
}
