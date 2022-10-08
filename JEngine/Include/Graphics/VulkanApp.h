#pragma once

namespace je
{
	struct VulkanApp final
	{
		enum Queue
		{
			renderQueue,
			presentQueue,
			transferQueue
		};

		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugger = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue queues[3]{};
		VkCommandPool commandPool = VK_NULL_HANDLE;
	};
}