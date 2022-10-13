#pragma once

namespace je
{
	struct VulkanMemory final
	{
		VkDeviceMemory memory;
		VkDeviceSize size;
		VkDeviceSize offset;
		size_t poolId;
	};
}