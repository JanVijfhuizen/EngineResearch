#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

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
				View<VkSurfaceFormatKHR> formats{};
				View<VkPresentModeKHR> presentModes{};

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
				View<StringView> validationLayers{};
				View<StringView> instanceExtensions{};
				View<StringView> deviceExtensions{};

				bool(*isPhysicalDeviceValid)(const PhysicalDeviceInfo& info) = IsPhysicalDeviceValid;
				size_t(*getPhysicalDeviceRating)(const PhysicalDeviceInfo& info) = GetPhysicalDeviceRating;
				VkPhysicalDeviceFeatures(*getPhysicalDeviceFeatures)() = GetPhysicalDeviceFeatures;
				VkSurfaceKHR(*createSurface)(VkInstance instance) = nullptr;
			};

			[[nodiscard]] App CreateApp(const Info& info);
			void DestroyApp(const App& app);

			[[nodiscard]] QueueFamilies GetQueueFamilies(Arena& arena, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(Arena& arena, VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface);
		}
	}
}
