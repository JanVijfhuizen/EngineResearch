#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		namespace init
		{
			struct QueueFamilies final
			{
				size_t graphics;
				size_t present;
				size_t transfer;

				[[nodiscard]] operator bool() const;
			};

			struct PhysicalDeviceInfo final
			{
				VkPhysicalDevice device;
				VkPhysicalDeviceProperties properties;
				VkPhysicalDeviceFeatures features;
			};

			struct SwapChainSupportDetails final
			{
				VkSurfaceCapabilitiesKHR capabilities{};
				Array<VkSurfaceFormatKHR> formats{};
				Array<VkPresentModeKHR> presentModes{};

				[[nodiscard]] operator bool() const;
				[[nodiscard]] size_t GetRecommendedImageCount() const;
			};

			[[nodiscard]] bool IsPhysicalDeviceValid(const PhysicalDeviceInfo& info);
			[[nodiscard]] size_t GetPhysicalDeviceRating(const PhysicalDeviceInfo& info);
			[[nodiscard]] VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures();

			// Vulkan application create info.
			struct Info final
			{
				Arena* tempArena = nullptr;
				Array<const char*> validationLayers{};
				Array<const char*> instanceExtensions{};
				Array<const char*> deviceExtensions{};

				bool(*isPhysicalDeviceValid)(const PhysicalDeviceInfo& info) = IsPhysicalDeviceValid;
				size_t(*getPhysicalDeviceRating)(const PhysicalDeviceInfo& info) = GetPhysicalDeviceRating;
				VkPhysicalDeviceFeatures(*getPhysicalDeviceFeatures)() = GetPhysicalDeviceFeatures;
				VkSurfaceKHR(*createSurface)(VkInstance instance) = nullptr;
			};

			// Initialize a vulkan app that work for most common use cases like pc games.
			[[nodiscard]] App CreateApp(const Info& info);
			void DestroyApp(const App& app);

			[[nodiscard]] QueueFamilies GetQueueFamilies(Arena& arena, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(Arena& arena, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		}
	}
}
