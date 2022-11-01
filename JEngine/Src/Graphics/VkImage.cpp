#include "pch.h"
#include "Graphics/VkImage.h"
#include "Graphics/VkAllocator.h"
#include "Graphics/VkApp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <exception>
#include <stb_image_write.h>

namespace je::vk
{
	Image::Image(const CreateInfo& info) :
		_app(info.app), _allocator(info.allocator),
		_format(info.format), _flag(info.flag), _resolution(info.resolution)
	{
		if(info.pixels)
			Load(info.pixels, info.usageFlags, info.layout);
		else if(info.path)
		{
			// Load pixels.
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(info.path.GetData(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;
			assert(pixels);
			_resolution = { texWidth, texHeight, 4 };

			const View<stbi_uc> view{ pixels, imageSize };
			
			Load(view, info.usageFlags, info.layout);
			stbi_image_free(pixels);
		}
		else
		{
			CreateImage(info.usageFlags);

			// Record and execute initial layout transition. 
			VkCommandBuffer cmdBuffer;
			VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.commandPool = _app->commandPool;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandBufferCount = 1;

			auto result = vkAllocateCommandBuffers(_app->device, &cmdBufferAllocInfo, &cmdBuffer);
			assert(!result);

			VkFence fence;
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			result = vkCreateFence(_app->device, &fenceInfo, nullptr, &fence);
			assert(!result);
			result = vkResetFences(_app->device, 1, &fence);
			assert(!result);

			VkCommandBufferBeginInfo cmdBeginInfo{};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
			
			TransitionLayout(cmdBuffer, info.layout, _flag);

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
			result = vkQueueSubmit(_app->queues[App::Queue::renderQueue], 1, &cmdSubmitInfo, fence);
			assert(!result);

			result = vkWaitForFences(_app->device, 1, &fence, VK_TRUE, UINT64_MAX);
			assert(!result);

			vkDestroyFence(_app->device, fence, nullptr);
		}
	}

	Image::Image(Image&& other) noexcept :
		_app(other._app), _allocator(other._allocator), _image(other._image), _layout(other._layout),
		_format(other._format), _flag(other._flag), _resolution(other._resolution), _memory(other._memory)
	{
		other._app = nullptr;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		_app = other._app;
		_allocator = other._allocator;
		_image = other._image;
		_layout = other._layout;
		_format = other._format;
		_flag = other._flag;
		_resolution = other._resolution;
		_memory = other._memory;
		other._app = nullptr;
		return *this;
	}

	Image::~Image()
	{
		if (!_app)
			return;
		vkDestroyImage(_app->device, _image, nullptr);
		const bool freeResult = _allocator->Free(_memory);
		assert(freeResult);
	}

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

	void Image::TransitionLayout(const VkCommandBuffer cmd, const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = _layout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = _image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;

		GetLayoutMasks(_layout, barrier.srcAccessMask, srcStage);
		GetLayoutMasks(newLayout, barrier.dstAccessMask, dstStage);

		vkCmdPipelineBarrier(cmd,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		_layout = newLayout;
	}

	glm::ivec3 Image::GetResolution() const
	{
		return _resolution;
	}

	VkFormat Image::GetFormat() const
	{
		return _format;
	}

	VkImageLayout Image::GetLayout() const
	{
		return _layout;
	}

	Image::operator VkImage() const
	{
		return _image;
	}

	void Image::Load(const View<unsigned char>& pixels, VkImageUsageFlags usageFlags, VkImageLayout layout)
	{
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		const size_t imageSize = static_cast<size_t>(_resolution.x) * static_cast<size_t>(_resolution.y) * 4;

		VkBuffer stagingBuffer;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		auto result = vkCreateBuffer(_app->device, &bufferInfo, nullptr, &stagingBuffer);
		assert(!result);

		VkMemoryRequirements stagingMemRequirements;
		vkGetBufferMemoryRequirements(_app->device, stagingBuffer, &stagingMemRequirements);

		const auto stagingMem = _allocator->Alloc(stagingMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		result = vkBindBufferMemory(_app->device, stagingBuffer, stagingMem.memory, stagingMem.offset);
		assert(!result);

		// Copy pixels to staging buffer.
		void* data;
		vkMapMemory(_app->device, stagingMem.memory, stagingMem.offset, imageSize, 0, &data);
		memcpy(data, pixels.GetData(), imageSize);
		vkUnmapMemory(_app->device, stagingMem.memory);

		CreateImage(usageFlags);

		// Record and execute copy. 
		VkCommandBuffer cmdBuffer;
		VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.commandPool = _app->commandPool;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(_app->device, &cmdBufferAllocInfo, &cmdBuffer);
		assert(!result);

		VkFence fence;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		result = vkCreateFence(_app->device, &fenceInfo, nullptr, &fence);
		assert(!result);
		result = vkResetFences(_app->device, 1, &fence);
		assert(!result);

		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);

		TransitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _flag);

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
			static_cast<uint32_t>(_resolution.x),
			static_cast<uint32_t>(_resolution.y),
			1
		};

		vkCmdCopyBufferToImage(
			cmdBuffer,
			stagingBuffer,
			_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		TransitionLayout(cmdBuffer, layout, _flag);

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
		result = vkQueueSubmit(_app->queues[App::Queue::renderQueue], 1, &cmdSubmitInfo, fence);
		assert(!result);

		result = vkWaitForFences(_app->device, 1, &fence, VK_TRUE, UINT64_MAX);
		assert(!result);

		vkDestroyFence(_app->device, fence, nullptr);
		vkDestroyBuffer(_app->device, stagingBuffer, nullptr);
		const bool freeResult = _allocator->Free(stagingMem);
		assert(freeResult);
	}

	void Image::CreateImage(const VkImageUsageFlags usageFlags)
	{
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = _resolution.x;
		imageCreateInfo.extent.height = _resolution.y;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = _format;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = _layout;
		imageCreateInfo.usage = usageFlags;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto result = vkCreateImage(_app->device, &imageCreateInfo, nullptr, &_image);
		assert(!result);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(_app->device, _image, &memRequirements);

		_memory = _allocator->Alloc(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		result = vkBindImageMemory(_app->device, _image, _memory.memory, _memory.offset);
		assert(!result);
	}
}
