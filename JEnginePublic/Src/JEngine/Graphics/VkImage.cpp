#include "JEngine/pch.h"
#include "JEngine/Graphics/VkImage.h"
#include "JEngine/Graphics/VkAllocator.h"
#include "JEngine/Graphics/VkApp.h"
#include <exception>

namespace je::vk
{
	void GetLayoutMasks(const VkImageLayout layout, VkAccessFlags& outAccessFlags,
		VkPipelineStageFlags& outPipelineStageFlags)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			outAccessFlags = 0;
			outPipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			outAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			outAccessFlags = VK_ACCESS_SHADER_READ_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			outAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			outAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			outPipelineStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		default:
			throw std::exception("Layout transition not supported!");
		}
	}

	void ImageCreateImage(Image& image, const App& app, const Allocator& allocator, const glm::ivec3 resolution, const ImageCreateInfo& info)
	{
		image.resolution = resolution;
		image.format = info.format;
		image.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		image.aspectFlags = info.aspectFlags;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = resolution.x;
		imageCreateInfo.extent.height = resolution.y;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = info.format;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = info.usageFlags;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		auto result = vkCreateImage(app.device, &imageCreateInfo, nullptr, &image.image);
		assert(!result);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(app.device, image.image, &memRequirements);

		image.memory = allocator.Alloc(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		result = vkBindImageMemory(app.device, image.image, image.memory.memory, image.memory.offset);
		assert(!result);
	}

	Image CreateImage(const App& app, const Allocator& allocator, const ImageCreateInfo& info, const glm::ivec3 resolution)
	{
		Image image{};
		ImageCreateImage(image, app, allocator, resolution, info);

		if (info.layout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			// Record and execute initial layout transition. 
			VkCommandBuffer cmdBuffer;
			VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.commandPool = app.commandPool;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandBufferCount = 1;

			auto result = vkAllocateCommandBuffers(app.device, &cmdBufferAllocInfo, &cmdBuffer);
			assert(!result);

			VkFence fence;
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			result = vkCreateFence(app.device, &fenceInfo, nullptr, &fence);
			assert(!result);
			result = vkResetFences(app.device, 1, &fence);
			assert(!result);

			VkCommandBufferBeginInfo cmdBeginInfo{};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);

			ImageTransitionLayout(image, cmdBuffer, info.layout, image.aspectFlags);

			// End recording.
			result = vkEndCommandBuffer(cmdBuffer);
			assert(!result);

			VkSubmitInfo cmdSubmitInfo{};
			cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			cmdSubmitInfo.commandBufferCount = 1;
			cmdSubmitInfo.pCommandBuffers = &cmdBuffer;
			cmdSubmitInfo.waitSemaphoreCount = 0;
			cmdSubmitInfo.pWaitSemaphores = nullptr;
			cmdSubmitInfo.signalSemaphoreCount = 0;
			cmdSubmitInfo.pSignalSemaphores = nullptr;
			cmdSubmitInfo.pWaitDstStageMask = nullptr;
			result = vkQueueSubmit(app.queues[App::Queue::renderQueue], 1, &cmdSubmitInfo, fence);
			assert(!result);

			result = vkWaitForFences(app.device, 1, &fence, VK_TRUE, UINT64_MAX);
			assert(!result);

			vkDestroyFence(app.device, fence, nullptr);
		}

		return image;
	}

	Image CreateImage(const App& app, const Allocator& allocator, const ImageCreateInfo& info, const Array<unsigned char>& pixels,
		const glm::ivec3 resolution)
	{
		Image image{};
		VkImageUsageFlags usageFlags = info.usageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		const size_t imageSize = static_cast<size_t>(resolution.x) * static_cast<size_t>(resolution.y) * 4;

		VkBuffer stagingBuffer;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		auto result = vkCreateBuffer(app.device, &bufferInfo, nullptr, &stagingBuffer);
		assert(!result);

		VkMemoryRequirements stagingMemRequirements;
		vkGetBufferMemoryRequirements(app.device, stagingBuffer, &stagingMemRequirements);

		const auto stagingMem = allocator.Alloc(stagingMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		result = vkBindBufferMemory(app.device, stagingBuffer, stagingMem.memory, stagingMem.offset);
		assert(!result);

		// Copy pixels to staging buffer.
		void* data;
		vkMapMemory(app.device, stagingMem.memory, stagingMem.offset, imageSize, 0, &data);
		memcpy(data, pixels.data, imageSize);
		vkUnmapMemory(app.device, stagingMem.memory);

		ImageCreateImage(image, app, allocator, resolution, info);

		// Record and execute copy. 
		VkCommandBuffer cmd;
		VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.commandPool = app.commandPool;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(app.device, &cmdBufferAllocInfo, &cmd);
		assert(!result);

		VkFence fence;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		result = vkCreateFence(app.device, &fenceInfo, nullptr, &fence);
		assert(!result);
		result = vkResetFences(app.device, 1, &fence);
		assert(!result);

		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd, &cmdBeginInfo);

		ImageTransitionLayout(image, cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.aspectFlags);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent =
		{
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y),
			1
		};

		vkCmdCopyBufferToImage(
			cmd,
			stagingBuffer,
			image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		ImageTransitionLayout(image, cmd, info.layout, image.aspectFlags);

		// End recording.
		result = vkEndCommandBuffer(cmd);
		assert(!result);

		VkSubmitInfo cmdSubmitInfo{};
		cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		cmdSubmitInfo.commandBufferCount = 1;
		cmdSubmitInfo.pCommandBuffers = &cmd;
		cmdSubmitInfo.waitSemaphoreCount = 0;
		cmdSubmitInfo.pWaitSemaphores = nullptr;
		cmdSubmitInfo.signalSemaphoreCount = 0;
		cmdSubmitInfo.pSignalSemaphores = nullptr;
		cmdSubmitInfo.pWaitDstStageMask = nullptr;
		result = vkQueueSubmit(app.queues[App::Queue::renderQueue], 1, &cmdSubmitInfo, fence);
		assert(!result);

		result = vkWaitForFences(app.device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(!result);

		vkDestroyFence(app.device, fence, nullptr);
		vkDestroyBuffer(app.device, stagingBuffer, nullptr);
		const bool freeResult = allocator.Free(stagingMem);
		assert(freeResult);

		return image;
	}

	void ImageTransitionLayout(Image& image, const VkCommandBuffer cmd,	
		const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = image.layout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image.image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;

		GetLayoutMasks(image.layout, barrier.srcAccessMask, srcStage);
		GetLayoutMasks(newLayout, barrier.dstAccessMask, dstStage);

		vkCmdPipelineBarrier(cmd,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		image.layout = newLayout;
	}

	void DestroyImage(const Image& image, const App& app, const Allocator& allocator)
	{
		vkDestroyImage(app.device, image.image, nullptr);
		const bool freeResult = allocator.Free(image.memory);
		assert(freeResult);
	}
}
