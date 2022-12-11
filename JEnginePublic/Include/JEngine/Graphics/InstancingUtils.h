#pragma once
#include "JEngine/Graphics/VkAllocator.h"
#include "JEngine/Graphics/VkApp.h"
#include "JEngine/Graphics/VkBuffer.h"
#include "Jlb/Array.h"

namespace je
{
	template <typename Type>
	Array<vk::Buffer> CreateStorageBuffers(Arena& arena, const vk::App& app, const vk::Allocator& allocator, const size_t swapChainLength, const size_t length)
	{
		auto& device = app.device;

		VkBufferCreateInfo vertBufferInfo{};
		vertBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertBufferInfo.size = sizeof(Type) * length;
		vertBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		vertBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		const auto buffers = CreateArray<vk::Buffer>(arena, swapChainLength);

		for (size_t i = 0; i < swapChainLength; ++i)
		{
			auto& buffer = buffers[i];
			auto result = vkCreateBuffer(device, &vertBufferInfo, nullptr, &buffer.buffer);
			assert(!result);

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer.buffer, &memRequirements);

			buffer.memory = allocator.Alloc(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			
			result = vkBindBufferMemory(device, buffer.buffer, buffer.memory.memory, buffer.memory.offset);
			assert(!result);
		}

		return buffers;
	}
}
