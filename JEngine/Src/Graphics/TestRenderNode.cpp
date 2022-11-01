#include "pch.h"
#include "Graphics/TestRenderNode.h"
#include "Graphics/VkAllocator.h"
#include "Graphics/VkImage.h"

namespace je::vk
{
	TestRenderNode::TestRenderNode(Arena& arena, App& app, Allocator& allocator) : _arena(arena)
	{
		glm::ivec3 resolution{ 800, 600, 1 };
		_image = arena.New<Image>(1, app, allocator, resolution);
	}

	TestRenderNode::~TestRenderNode()
	{
		_arena.Delete(_image);
	}

	void TestRenderNode::Render(const UpdateInfo& info)
	{

	}
}
