#pragma once

namespace je
{
	struct PhysicalDeviceInfo final
	{
		VkPhysicalDevice device;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
	};
}