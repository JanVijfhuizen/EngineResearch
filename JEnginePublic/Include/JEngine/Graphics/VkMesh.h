#pragma once 
#include "VkApp.h" 
#include <cstdint> 
#include "VkAllocator.h" 
#include "VkBuffer.h" 
#include <JEngine/Graphics/Vertex.h>

namespace je::vk
{
	// 2d/3d model used for rendering.
	struct Mesh final
	{
		Buffer vertexBuffer;
		Buffer indexBuffer;
		size_t indexCount;

		void Draw(VkCommandBuffer cmd, size_t count) const;
	};

	[[nodiscard]] Mesh CreateMesh(const App& app, const Allocator& allocator, const Array<Vertex>& vertices, const Array<Vertex::Index>& indices);
	void DestroyMesh(const Mesh& mesh, const App& app, const Allocator& allocator);

	template <typename T>
	Buffer CreateVertexBuffer(const App& app, const Allocator& allocator,
		const Array<T>& data, const VkBufferUsageFlags usageFlags)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(T) * data.length;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer stagingBuffer;
		auto result = vkCreateBuffer(app.device, &bufferInfo, nullptr, &stagingBuffer);
		assert(!result);

		VkMemoryRequirements stagingMemRequirements;
		vkGetBufferMemoryRequirements(app.device, stagingBuffer, &stagingMemRequirements);

		const auto stagingMem = allocator.Alloc(stagingMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		result = vkBindBufferMemory(app.device, stagingBuffer, stagingMem.memory, stagingMem.offset);
		assert(!result);

		// Move vertex/index data to a staging buffer. 
		void* stagingData;
		vkMapMemory(app.device, stagingMem.memory, stagingMem.offset, stagingMem.size, 0, &stagingData);
		memcpy(stagingData, static_cast<const void*>(data.data), bufferInfo.size);
		vkUnmapMemory(app.device, stagingMem.memory);

		bufferInfo.usage = usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VkBuffer buffer;
		result = vkCreateBuffer(app.device, &bufferInfo, nullptr, &buffer);
		assert(!result);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(app.device, buffer, &memRequirements);

		const auto mem = allocator.Alloc(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		result = vkBindBufferMemory(app.device, buffer, mem.memory, mem.offset);
		assert(!result);
		
		// Record and execute copy. 
		VkCommandBuffer cmdBuffer;
		VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.commandPool = app.commandPool;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(app.device, &cmdBufferAllocInfo, &cmdBuffer);
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

		VkBufferCopy region{};
		region.srcOffset = 0;
		region.dstOffset = 0;
		region.size = mem.size;
		vkCmdCopyBuffer(cmdBuffer, stagingBuffer, buffer, 1, &region);

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
		vkDestroyBuffer(app.device, stagingBuffer, nullptr);
		const bool freeResult = allocator.Free(stagingMem);
		assert(freeResult);

		Buffer ret{};
		ret.buffer = buffer;
		ret.memory = mem;
		return ret;
	}
}