#include "pch.h"
#include "Graphics/TestRenderNode.h"
#include "Graphics/Vertex.h"
#include "Graphics/VkAllocator.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkImage.h"
#include "Jlb/JMove.h"

namespace je::vk
{
	TestRenderNode::TestRenderNode(Arena& arena, App& app) : _arena(arena), _app(app)
	{
		
	}

	void TestRenderNode::Render(const VkCommandBuffer cmdBuffer)
	{
		
	}

	Array<RenderNode::Output> TestRenderNode::DefineOutputs(Arena& arena) const
	{
		Array<Output> outputs{arena, 1};
		outputs[0].name = "Result";
		auto& resource = outputs[0].resource;
		resource.resolution = glm::ivec3{ 800, 600, 4 };
		return Move(outputs);
	}
}
