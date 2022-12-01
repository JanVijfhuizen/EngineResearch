#include "pch.h"
#include "JEngine/Graphics/Vertex.h"

namespace je::vk
{
	Array<VkVertexInputBindingDescription> Vertex::GetBindingDescriptions(Arena& arena)
	{
		const auto ret = CreateArray<VkVertexInputBindingDescription>(arena, 1);
		auto& vertexData = ret[0];
		vertexData.binding = 0;
		vertexData.stride = sizeof(Vertex);
		vertexData.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return ret;
	}

	Array<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions(Arena& arena)
	{
		const auto ret = CreateArray<VkVertexInputAttributeDescription>(arena, 3);

		auto& position = ret[0];
		position.binding = 0;
		position.location = 0;
		position.format = VK_FORMAT_R32G32B32_SFLOAT;
		position.offset = offsetof(Vertex, position);

		auto& normal = ret[1];
		normal.binding = 0;
		normal.location = 1;
		normal.format = VK_FORMAT_R32G32B32_SFLOAT;
		normal.offset = offsetof(Vertex, normal);

		auto& texCoords = ret[2];
		texCoords.binding = 0;
		texCoords.location = 2;
		texCoords.format = VK_FORMAT_R32G32_SFLOAT;
		texCoords.offset = offsetof(Vertex, textureCoordinates);

		return ret;
	}
}
