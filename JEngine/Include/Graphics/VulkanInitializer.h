#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je
{
	class Arena;
	struct VulkanApp;

	namespace vkinit
	{
		// Vulkan physical device info.
		struct PhysicalDeviceInfo final
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures features;
		};
		
		[[nodiscard]] bool IsPhysicalDeviceValid(const PhysicalDeviceInfo& info);
		[[nodiscard]] size_t GetPhysicalDeviceRating(const PhysicalDeviceInfo& info);
		[[nodiscard]] VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures();

		// Vulkan application create info.
		struct Info final
		{
			Arena* tempArena = nullptr;
			View<StringView> validationLayers{};
			View<StringView> instanceExtensions{};
			View<StringView> deviceExtensions{};

			bool(*isPhysicalDeviceValid)(const PhysicalDeviceInfo& info) = IsPhysicalDeviceValid;
			size_t(*getPhysicalDeviceRating)(const PhysicalDeviceInfo& info) = GetPhysicalDeviceRating;
			VkPhysicalDeviceFeatures(*getPhysicalDeviceFeatures)() = GetPhysicalDeviceFeatures;
			VkSurfaceKHR(*createSurface)(VkInstance instance) = nullptr;
		};
		
		[[nodiscard]] VulkanApp CreateApp(const Info& info);
		void DestroyApp(const VulkanApp& app);
	}
}
