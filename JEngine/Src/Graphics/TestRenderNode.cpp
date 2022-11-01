#include "pch.h"
#include "Graphics/TestRenderNode.h"
#include "Graphics/VkAllocator.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkImage.h"

namespace je::vk
{
	TestRenderNode::TestRenderNode(Arena& arena, App& app, Allocator& allocator) : _arena(arena), _app(app)
	{
		Image::CreateInfo imageCreateInfo{};
		imageCreateInfo.app = &app;
		imageCreateInfo.allocator = &allocator;
		imageCreateInfo.resolution = glm::ivec3{ 800, 600, 4 };
		imageCreateInfo.usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageCreateInfo.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		_image = arena.New<Image>(1, imageCreateInfo);

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
		attachmentInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachmentInfo.format = _image->GetFormat();

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentInfo;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;

		const auto renderPassResult = vkCreateRenderPass(app.device, &renderPassCreateInfo, nullptr, &_renderPass);
		assert(!renderPassResult);

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.format = _image->GetFormat();
		viewCreateInfo.image = *_image;

		const auto viewResult = vkCreateImageView(app.device, &viewCreateInfo, nullptr, &_view);
		assert(!viewResult);

		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.layers = 1;
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.renderPass = _renderPass;
		frameBufferCreateInfo.width = _image->GetResolution().x;
		frameBufferCreateInfo.height = _image->GetResolution().y;
		frameBufferCreateInfo.pAttachments = &_view;

		auto result = vkCreateFramebuffer(app.device, &frameBufferCreateInfo, nullptr, &_frameBuffer);
		assert(!result);
	}

	TestRenderNode::~TestRenderNode()
	{
		vkDestroyFramebuffer(_app.device, _frameBuffer, nullptr);
		vkDestroyImageView(_app.device, _view, nullptr);
		vkDestroyRenderPass(_app.device, _renderPass, nullptr);
		_arena.Delete(_image);
	}

	void TestRenderNode::Render(const UpdateInfo& info)
	{
		
	}
}
