#include "pch.h"
#include "Graphics/VulkanInitializer.h"
#include <cstring>
#include <iostream>

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

	void CheckValidationSupport(Arena& tempArena, const Array<StringView>& validationLayers)
	{
#ifdef NDEBUG
		return;
#endif

		const auto _ = tempArena.CreateScope();

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		const Array<VkLayerProperties> availableLayers{tempArena, layerCount};
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetData());

		const auto validationLayersView = validationLayers.GetView();
		const auto availableLayersView = availableLayers.GetView();

		// Iterate over all the layers to see if they are available.
		for (const auto& layer : validationLayersView)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayersView)
				if (strcmp(layer, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			assert(layerFound);
		}
	}

	VkApplicationInfo CreateApplicationInfo()
	{
		constexpr auto version = VK_MAKE_VERSION(1, 0, 0);
		
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Application";
		appInfo.applicationVersion = version;
		appInfo.pEngineName = "Vulkan Application";
		appInfo.engineVersion = version;
		appInfo.apiVersion = VK_API_VERSION_1_0;
		return appInfo;
	}

	VkBool32 DebugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT CreateDebugInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;
		return info;
	}

	VkInstance CreateInstance(const Arena& arena, const Array<StringView>& validationLayers, const Array<StringView>& instanceExtensions)
	{
		const auto appInfo = CreateApplicationInfo();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.GetLength());
		createInfo.ppEnabledExtensionNames = reinterpret_cast<const char**>(instanceExtensions.GetData());

		const auto validationCreateInfo = CreateDebugInfo();

		createInfo.enabledLayerCount = 0;
#ifdef _DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetLength());
		createInfo.ppEnabledLayerNames = reinterpret_cast<const char**>(validationLayers.GetData());
		createInfo.pNext = &validationCreateInfo;
#endif

		VkInstance instance;
		const auto result = vkCreateInstance(&createInfo, nullptr, &instance);
		assert(!result);

		return instance;
	}

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")))
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")))
			func(instance, debugMessenger, pAllocator);
		}

	VkDebugUtilsMessengerEXT CreateDebugger(VkInstance instance)
	{
#ifdef NDEBUG
		return;
#endif

		const auto createInfo = CreateDebugInfo();
		VkDebugUtilsMessengerEXT debugger;
		const auto result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugger);
		assert(!result);
		return debugger;
	}

	VulkanApp CreateApp(const Info& info)
	{
		VulkanApp app{};

		const auto _ = info.tempArena->CreateScope();

		bool debugExtensionPresent = true;
#ifdef _DEBUG
		debugExtensionPresent = false;
		for (auto& instanceExtension : info.instanceExtensions)
			if (instanceExtension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			{
				debugExtensionPresent = true;
				break;
			}
#endif
		const Array<StringView> instanceExtensions
		{
			*info.tempArena,
			 info.instanceExtensions.GetLength() + !debugExtensionPresent
		};
		if (!debugExtensionPresent)
			instanceExtensions[instanceExtensions.GetLength() - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

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
			info.deviceExtensions.GetLength() + !swapChainExtensionPresent
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
			info.validationLayers.GetLength() + !khronosValidationPresent
		};
		if (!khronosValidationPresent)
			validationLayers[validationLayers.GetLength() - 1] = "VK_LAYER_KHRONOS_validation";

		memcpy(validationLayers.GetData(), info.validationLayers.GetData(), sizeof(StringView) * info.validationLayers.GetLength());
		memcpy(instanceExtensions.GetData(), info.instanceExtensions.GetData(), sizeof(StringView) * info.instanceExtensions.GetLength());
		memcpy(deviceExtensions.GetData(), info.deviceExtensions.GetData(), sizeof(StringView) * info.deviceExtensions.GetLength());

		CheckValidationSupport(*info.tempArena, validationLayers);
		app.instance = CreateInstance(*info.tempArena, validationLayers, instanceExtensions);
		app.debugger = CreateDebugger(app.instance);
		app.surface = info.createSurface(app.instance);
		return app;
	}

	void DestroyApp(const VulkanApp& app)
	{
		vkDestroySurfaceKHR(app.instance, app.surface, nullptr);
#ifdef _DEBUG
		DestroyDebugUtilsMessengerEXT(app.instance, app.debugger, nullptr);
#endif
		vkDestroyInstance(app.instance, nullptr);
	}
}
