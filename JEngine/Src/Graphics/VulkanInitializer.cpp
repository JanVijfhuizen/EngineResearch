#include "pch.h"
#include "Graphics/VulkanInitializer.h"
#include "Graphics/PhysicalDeviceInfo.h"
#include "Graphics/VulkanApp.h"

namespace je
{
	bool vkinit::IsPhysicalDeviceValid(const PhysicalDeviceInfo& info)
	{
		return true;
	}

	size_t vkinit::GetPhysicalDeviceRating(const PhysicalDeviceInfo& info)
	{
		size_t score = 0;
		const auto& properties = info.properties;
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += properties.limits.maxImageDimension2D;
		return score;
	}

	VkPhysicalDeviceFeatures vkinit::GetPhysicalDeviceFeatures()
	{
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		return deviceFeatures;
	}

	VulkanApp vkinit::CreateApp(const Info& info)
	{
		return VulkanApp{};
	}

	void vkinit::DestroyApp(VulkanApp& app)
	{
	}
}
