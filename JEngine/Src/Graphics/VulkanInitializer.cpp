#include "pch.h"
#include "Graphics/VulkanInitializer.h"
#include "Graphics/PhysicalDeviceInfo.h"
#include "Graphics/VulkanApp.h"

namespace je::vkinit
{
	bool IsPhysicalDeviceValid(const PhysicalDeviceInfo& info)
	{
		return true;
	}

	size_t GetPhysicalDeviceRating(const PhysicalDeviceInfo& info)
	{
		size_t score = 0;
		const auto& properties = info.properties;
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += properties.limits.maxImageDimension2D;
		return score;
	}

	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures()
	{
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		return deviceFeatures;
	}

	void CheckValidationSupport(Arena& tempArena)
	{
#ifdef NDEBUG
		return;
#endif

		const auto _ = tempArena.CreateScope();

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		const Array<VkLayerProperties> availableLayers{tempArena, layerCount};
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetData());
	}

	VulkanApp CreateApp(const Info& info)
	{
		const auto _ = info.tempArena->CreateScope();

		// Add swap chain extension if not present.
		bool swapChainExtensionPresent = false;
		for (auto& deviceExtension : info.deviceExtensions)
			if(deviceExtension == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			{
				swapChainExtensionPresent = true;
				break;
			}

		const Array<StringView> deviceExtensions
		{
			*info.tempArena,
			info.deviceExtensions.GetLength() + static_cast<size_t>(!swapChainExtensionPresent)
		};
		if (!swapChainExtensionPresent)
			deviceExtensions[deviceExtensions.GetLength() - 1] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

		// Add khronos validation if not present.
		bool khronosValidationPresent = false;
		for (auto& validationLayer : info.validationLayers)
			if(validationLayer == "VK_LAYER_KHRONOS_validation")
			{
				khronosValidationPresent = true;
				break;
			}

		const Array<StringView> validationLayers
		{
			*info.tempArena,
			info.validationLayers.GetLength() + static_cast<size_t>(!khronosValidationPresent)
		};
		if (!khronosValidationPresent)
			validationLayers[validationLayers.GetLength() - 1] = "VK_LAYER_KHRONOS_validation";

		CheckValidationSupport(*info.tempArena);

		return VulkanApp{};
	}

	void DestroyApp(VulkanApp& app)
	{
		
	}
}
