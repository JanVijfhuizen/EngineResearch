#include "pch.h"
#include "Graphics/VkMesh.h"

namespace je::vk
{
	Mesh::Mesh(const App& app, Allocator& allocator,
		const View<Vertex>& vertices, const View<Vertex::Index>& indices) : _app(&app), _allocator(&allocator)
	{
		_vertexBuffer = CreateBuffer(app, allocator, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		_indexBuffer = CreateBuffer(app, allocator, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		_indexCount = indices.GetLength();
	}

	Mesh::Mesh(Mesh&& other) noexcept : _vertexBuffer(other._vertexBuffer),
		_indexBuffer(other._indexBuffer), _indexCount(other._indexCount), _app(other._app), _allocator(other._allocator)
	{
		other._app = nullptr;
	}

	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		_vertexBuffer = other._vertexBuffer;
		_indexBuffer = other._indexBuffer;
		_indexCount = other._indexCount;
		_app = other._app;
		_allocator = other._allocator;
		other._app = nullptr;
		return *this;
	}

	Mesh::~Mesh()
	{
		if (!_app)
			return;
		auto result = _allocator->Free(_indexBuffer.memory);
		assert(result);
		result = _allocator->Free(_vertexBuffer.memory);
		assert(result);
		vkDestroyBuffer(_app->device, _indexBuffer.buffer, nullptr);
		vkDestroyBuffer(_app->device, _vertexBuffer.buffer, nullptr);
	}
}