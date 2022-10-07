﻿#pragma once
#include "Jlb/Array.h"
#include "Jlb/StringView.h"
#include "Jlb/View.h"

namespace je
{
	class Arena;
	struct PhysicalDeviceInfo;
	struct VulkanApp;

	namespace vkinit
	{
		[[nodiscard]] bool IsPhysicalDeviceValid(const PhysicalDeviceInfo& info);
		[[nodiscard]] size_t GetPhysicalDeviceRating(const PhysicalDeviceInfo& info);
		[[nodiscard]] VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures();

		struct Info final
		{
			Arena* tempArena = nullptr;
			View<StringView> validationLayers{};
			View<StringView> instanceExtensions{};
			View<StringView> deviceExtensions{};

			bool(*isPhysicalDeviceValid)(const PhysicalDeviceInfo& info) = IsPhysicalDeviceValid;
			size_t(*getPhysicalDeviceRating)(const PhysicalDeviceInfo& info) = GetPhysicalDeviceRating;
			VkPhysicalDeviceFeatures(*getPhysicalDeviceFeatures)() = GetPhysicalDeviceFeatures;
		};
		
		[[nodiscard]] VulkanApp CreateApp(const Info& info);
		void DestroyApp(const VulkanApp& app);
	}
}
