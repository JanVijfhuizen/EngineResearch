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

	void TestRenderNode::DefineRenderPass(const App& app, VkRenderPass& outRenderPass)
	{
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass = 0;
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask = 0;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachmentInfo{};
		attachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentInfo.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentInfo;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;

		const auto renderPassResult = vkCreateRenderPass(app.device, &renderPassCreateInfo, nullptr, &outRenderPass);
		assert(!renderPassResult);
	}
}
