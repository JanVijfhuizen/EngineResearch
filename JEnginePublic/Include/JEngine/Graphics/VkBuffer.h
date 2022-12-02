#pragma once
#include "VkMemory.h"

namespace je::vk
{
	// Holds a buffer and the memory it points to.
	struct Buffer final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		Memory memory{};
	};
}
