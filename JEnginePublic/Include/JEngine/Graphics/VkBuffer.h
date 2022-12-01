#pragma once
#include "VkMemory.h"

namespace je::vk
{
	struct Buffer final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		Memory memory{};
	};
}
