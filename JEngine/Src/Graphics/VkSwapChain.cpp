#include "pch.h"
#include "Graphics/VkSwapChain.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkInitializer.h"
#include "Jlb/JMath.h"
#include "Jlb/Arena.h"
#include "Modules/WindowModule.h"

namespace je::vk
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

	SwapChain::SwapChain(Arena& arena, Arena& tempArena, const App& app, const engine::WindowModule& windowModule) :
		_arena(arena), _tempArena(tempArena), _app(app), _windowModule(windowModule)
	{
		const auto _ = _tempArena.CreateScope();
		
		const auto support = init::QuerySwapChainSupport(tempArena, app.physicalDevice, app.surface);
		const size_t imageCount = support.GetRecommendedImageCount();
		_surfaceFormat = ChooseSurfaceFormat(support.formats);
		_presentMode = ChoosePresentMode(support.presentModes);

		_images = Array<Image>(arena, imageCount);
		_frames = Array<Frame>(arena, SWAPCHAIN_MAX_FRAMES_IN_FLIGHT);

		Recreate();
	}

	SwapChain::~SwapChain()
	{
		Cleanup();
		_arena.Free(_frames);
		_arena.Free(_images);
	}

	void SwapChain::WaitForImage()
	{
		const auto& frame = _frames[_frameIndex];

		auto result = vkWaitForFences(_app.device, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX);
		assert(!result);
		result = vkAcquireNextImageKHR(_app.device,
			_swapChain, UINT64_MAX, frame.imageAvailableSemaphore, VK_NULL_HANDLE, &_imageIndex);
		assert(!result);

		auto& image = _images[_imageIndex];
		if (image.fence)
			vkWaitForFences(_app.device, 1, &image.fence, VK_TRUE, UINT64_MAX);
		image.fence = frame.inFlightFence;
	}

	VkCommandBuffer SwapChain::BeginFrame(const bool manuallyCallWaitForImage)
	{
		if (!manuallyCallWaitForImage)
			WaitForImage();

		const auto& image = _images[_imageIndex];

		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkResetCommandBuffer(image.cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkBeginCommandBuffer(image.cmdBuffer, &cmdBufferBeginInfo);

		const VkClearValue clearColor = { 1.f, 1.f, 1.f, 1.f };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderPass = _renderPass;
		renderPassBeginInfo.framebuffer = image.frameBuffer;
		renderPassBeginInfo.renderArea.extent = _extent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(image.cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		return image.cmdBuffer;
	}

	void SwapChain::EndFrame(const View<VkSemaphore>& waitSemaphores)
	{
		auto& frame = _frames[_frameIndex];
		auto& image = _images[_imageIndex];

		vkCmdEndRenderPass(image.cmdBuffer);
		auto result = vkEndCommandBuffer(image.cmdBuffer);
		assert(!result);

		const auto _ = _tempArena.CreateScope();
		const Array<VkSemaphore> allWaitSemaphores{_tempArena, waitSemaphores.GetLength() + 1};
		memcpy(allWaitSemaphores.GetData(), waitSemaphores.GetData(), sizeof(VkSemaphore) * waitSemaphores.GetLength());
		allWaitSemaphores[waitSemaphores.GetLength()] = frame.imageAvailableSemaphore;

		constexpr VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &image.cmdBuffer;
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(allWaitSemaphores.GetLength());
		submitInfo.pWaitSemaphores = allWaitSemaphores;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &frame.renderFinishedSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;

		vkResetFences(_app.device, 1, &frame.inFlightFence);
		result = vkQueueSubmit(_app.queues[App::renderQueue], 1, &submitInfo, frame.inFlightFence);
		assert(!result);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &frame.renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapChain;
		presentInfo.pImageIndices = &_imageIndex;

		result = vkQueuePresentKHR(_app.queues[App::presentQueue], &presentInfo);
		_frameIndex = (_frameIndex + 1) % _frames.GetLength();

		if(result)
			Recreate();
	}

	size_t SwapChain::GetLength() const
	{
		return _images.GetLength();
	}

	VkRenderPass SwapChain::GetRenderPass() const
	{
		return _renderPass;
	}

	glm::ivec2 SwapChain::GetResolution() const
	{
		return {_extent.width, _extent.height };
	}

	VkCommandBuffer SwapChain::GetCmdBuffer() const
	{
		return _images[_imageIndex].cmdBuffer;
	}

	size_t SwapChain::GetIndex() const
	{
		return _imageIndex;
	}

	void SwapChain::Cleanup() const
	{
		if (!_swapChain)
			return;

		const auto result = vkDeviceWaitIdle(_app.device);
		assert(!result);
		
		for (auto& image : _images.GetView())
		{
			vkDestroyImageView(_app.device, image.view, nullptr);
			if (image.fence)
				vkWaitForFences(_app.device, 1, &image.fence, VK_TRUE, UINT64_MAX);
			image.fence = VK_NULL_HANDLE;
			vkFreeCommandBuffers(_app.device, _app.commandPool, 1, &image.cmdBuffer);
			vkDestroyFramebuffer(_app.device, image.frameBuffer, nullptr);
		}

		for (const auto& frame : _frames.GetView())
		{
			vkDestroySemaphore(_app.device, frame.imageAvailableSemaphore, nullptr);
			vkDestroySemaphore(_app.device, frame.renderFinishedSemaphore, nullptr);
			vkDestroyFence(_app.device, frame.inFlightFence, nullptr);
		}
		
		vkDestroyRenderPass(_app.device, _renderPass, nullptr);
		vkDestroySwapchainKHR(_app.device, _swapChain, nullptr);
	}

	void SwapChain::Recreate()
	{
		const auto _ = _tempArena.CreateScope();
		const auto support = init::QuerySwapChainSupport(_tempArena, _app.physicalDevice, _app.surface);
		_extent = ChooseExtent(support.capabilities, _windowModule.GetResolution());

		const auto families = init::GetQueueFamilies(_tempArena, _app.physicalDevice, _app.surface);

		const uint32_t queueFamilyIndices[] =
		{
			static_cast<uint32_t>(families.graphics),
			static_cast<uint32_t>(families.present)
		};

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _app.surface;
		createInfo.minImageCount = static_cast<uint32_t>(_images.GetLength());
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

		auto length = static_cast<uint32_t>(_images.GetLength());
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
		viewCreateInfo.format = _surfaceFormat.format;

		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.layers = 1;
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.renderPass = _renderPass;
		frameBufferCreateInfo.width = _extent.width;
		frameBufferCreateInfo.height = _extent.height;

		// Create images.
		for (uint32_t i = 0; i < length; ++i)
		{
			auto& image = _images[i];
			image.image = vkImages[i];

			viewCreateInfo.image = image.image;
			const auto viewResult = vkCreateImageView(_app.device, &viewCreateInfo, nullptr, &image.view);
			assert(!viewResult);

			image.cmdBuffer = cmdBuffers[i];
			frameBufferCreateInfo.pAttachments = &image.view;
			const auto frameBufferResult = vkCreateFramebuffer(_app.device, &frameBufferCreateInfo, nullptr, &image.frameBuffer);
			assert(!frameBufferResult);
		}

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Create frames.
		for (auto& frame : _frames.GetView())
		{
			auto semaphoreResult = vkCreateSemaphore(_app.device, &semaphoreCreateInfo, nullptr, &frame.imageAvailableSemaphore);
			assert(!semaphoreResult);
			semaphoreResult = vkCreateSemaphore(_app.device, &semaphoreCreateInfo, nullptr, &frame.renderFinishedSemaphore);
			assert(!semaphoreResult);
			
			const auto fenceResult = vkCreateFence(_app.device, &fenceCreateInfo, nullptr, &frame.inFlightFence);
			assert(!fenceResult);
		}
	}
}
