#pragma once

namespace je::vk
{
	struct Memory final
	{
		VkDeviceMemory memory;
		VkDeviceSize size;
		VkDeviceSize offset;
		size_t poolId;
	};
}