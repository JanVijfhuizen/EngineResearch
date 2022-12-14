#pragma once
#include "Jlb/Array.h"

namespace je::vk
{
	// Defines a vertex point in a mesh.
	struct Vertex final
	{
		typedef uint16_t Index;

		glm::vec3 position{};
		// Forward direction of a vertex.
		glm::vec3 normal{0, 0, 1};
		glm::vec2 textureCoordinates{};
		
		[[nodiscard]] static Array<VkVertexInputBindingDescription> GetBindingDescriptions(Arena& arena);
		[[nodiscard]] static Array<VkVertexInputAttributeDescription> GetAttributeDescriptions(Arena& arena);
	};
}
