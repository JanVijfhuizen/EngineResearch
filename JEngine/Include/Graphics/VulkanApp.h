#pragma once

namespace je
{
	struct VulkanApp final
	{
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugger = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};
}