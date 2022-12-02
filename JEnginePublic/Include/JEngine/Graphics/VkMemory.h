#pragma once

namespace je::vk
{
	// Vulkan gpu memory.
	struct Memory final
	{
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
		VkDeviceSize offset = 0;
		size_t poolId = SIZE_MAX;
	};
}