#include "pch.h"
#include "Graphics/VulkanSwapChain.h"
#include "Graphics/VulkanApp.h"
#include "Graphics/VulkanInitializer.h"
#include "Jlb/JMath.h"
#include "Jlb/Arena.h"
#include "Modules/WindowModule.h"

namespace je
{
	VkSurfaceFormatKHR ChooseSurfaceFormat(const View<VkSurfaceFormatKHR>& availableFormats)
	{
		// Preferably go for SRGB, if it's not present just go with the first one found.
		// We can basically assume that SRGB is supported on most hardware.
		for (const auto& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats[0];
	}

	VkPresentModeKHR ChoosePresentMode(const View<VkPresentModeKHR>& availablePresentModes)
	{
		// Preferably go for Mailbox, otherwise go for Fifo.
		// Fifo is traditional VSync, where mailbox is all that and better, but unlike Fifo is not required to be supported by the hardware.
		for (const auto& availablePresentMode : availablePresentModes)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, const glm::ivec2 resolution)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y)
		};

		const auto& minExtent = capabilities.minImageExtent;
		const auto& maxExtent = capabilities.maxImageExtent;

		actualExtent.width = math::Clamp(actualExtent.width, minExtent.width, maxExtent.width);
		actualExtent.height = math::Clamp(actualExtent.height, minExtent.height, maxExtent.height);

		return actualExtent;
	}

	VulkanSwapChain::VulkanSwapChain(Arena& arena, Arena& tempArena, const VulkanApp& app, const engine::WindowModule& windowModule) :
		_arena(arena), _tempArena(tempArena), _app(app), _windowModule(windowModule)
	{
		const auto _ = _tempArena.CreateScope();
		
		const auto support = vkinit::QuerySwapChainSupport(tempArena, app.physicalDevice, app.surface);
		const size_t imageCount = support.GetRecommendedImageCount();
		_surfaceFormat = ChooseSurfaceFormat(support.formats);
		_presentMode = ChoosePresentMode(support.presentModes);

		_images = arena.New<Array<Image>>(1, arena, imageCount);
		_frames = arena.New<Array<Frame>>(1, arena, SWAPCHAIN_MAX_FRAMES_IN_FLIGHT);

		Recreate();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		Cleanup();
		_arena.Free(_frames);
		_arena.Free(_images);
	}

	void VulkanSwapChain::Cleanup()
	{
		if (!_swapChain)
			return;

		const auto result = vkDeviceWaitIdle(_app.device);
		assert(!result);
		
		vkDestroyRenderPass(_app.device, _renderPass, nullptr);
		vkDestroySwapchainKHR(_app.device, _swapChain, nullptr);
	}

	void VulkanSwapChain::Recreate()
	{
		const auto _ = _tempArena.CreateScope();
		const auto support = vkinit::QuerySwapChainSupport(_tempArena, _app.physicalDevice, _app.surface);
		_extent = ChooseExtent(support.capabilities, _windowModule.GetResolution());

		const auto families = vkinit::GetQueueFamilies(_tempArena, _app.physicalDevice, _app.surface);

		const uint32_t queueFamilyIndices[] =
		{
			static_cast<uint32_t>(families.graphics),
			static_cast<uint32_t>(families.present)
		};

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _app.surface;
		createInfo.minImageCount = static_cast<uint32_t>(_images->GetLength());
		createInfo.imageFormat = _surfaceFormat.format;
		createInfo.imageColorSpace = _surfaceFormat.colorSpace;
		createInfo.imageExtent = _extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (families.graphics != families.present)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = _presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = _swapChain;

		VkSwapchainKHR newSwapChain;
		const auto result = vkCreateSwapchainKHR(_app.device, &createInfo, nullptr, &newSwapChain);
		assert(!result);

		Cleanup();
		_swapChain = newSwapChain;

		auto length = static_cast<uint32_t>(_images->GetLength());
		const Array<VkImage> vkImages{_tempArena, length};
		vkGetSwapchainImagesKHR(_app.device, _swapChain, &length, vkImages.GetData());

		VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.commandPool = _app.commandPool;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = length;

		const Array<VkCommandBuffer> cmdBuffers{_tempArena, length};
		const auto cmdResult = vkAllocateCommandBuffers(_app.device, &cmdBufferAllocInfo, cmdBuffers.GetData());
		assert(!cmdResult);

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
		attachmentInfo.format = _surfaceFormat.format;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentInfo;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;

		const auto renderPassResult = vkCreateRenderPass(_app.device, &renderPassCreateInfo, nullptr, &_renderPass);
		assert(!renderPassResult);
	}
}
