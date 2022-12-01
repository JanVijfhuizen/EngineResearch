#include "JEngine/pch.h"
#include "JEngine/Graphics/VkMesh.h"

namespace je::vk
{
	void Mesh::Draw(const VkCommandBuffer cmd, const size_t count) const
	{
		constexpr VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indexCount), static_cast<uint32_t>(count), 0, 0, 0);
	}

	Mesh CreateMesh(const App& app, const Allocator& allocator, const Array<Vertex>& vertices,
		const Array<Vertex::Index>& indices)
	{
		Mesh mesh{};
		mesh.vertexBuffer = CreateVertexBuffer(app, allocator, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		mesh.indexBuffer = CreateVertexBuffer(app, allocator, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		mesh.indexCount = indices.length;
		return mesh;
	}

	void DestroyMesh(const Mesh& mesh, const App& app, const Allocator& allocator)
	{
		auto result = allocator.Free(mesh.indexBuffer.memory);
		assert(result);
		result = allocator.Free(mesh.vertexBuffer.memory);
		assert(result);
		vkDestroyBuffer(app.device, mesh.indexBuffer.buffer, nullptr);
		vkDestroyBuffer(app.device, mesh.vertexBuffer.buffer, nullptr);
	}
}
